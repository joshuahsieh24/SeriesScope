#include <gtest/gtest.h>
#include "../src/engine/GameProbabilityModel.h"
#include "../src/engine/MonteCarloRunner.h"
#include "../src/models/TeamProfile.h"
#include "../src/models/ScenarioConfig.h"

using namespace ss;

// ---------------------------------------------------------------------------
// Test fixtures
// ---------------------------------------------------------------------------

static TeamProfile makeTeam(double net_rating, double win_pct_frac = 0.5,
                             double eFG = 0.55, double TOV = 0.13,
                             double ORB = 0.25, double FT = 0.20) {
    TeamProfile t;
    t.net_rating  = net_rating;
    t.off_rating  = 115.0 + net_rating / 2.0;
    t.def_rating  = 115.0 - net_rating / 2.0;
    t.wins        = static_cast<int>(82 * win_pct_frac);
    t.losses      = 82 - t.wins;
    t.recent_form = 0.0;
    t.star_impact = 1.0;
    t.volatility  = 0.20;
    t.depth_rating = 0.50;
    t.eFG_pct     = eFG;
    t.TOV_pct     = TOV;
    t.ORB_pct     = ORB;
    t.FT_rate     = FT;
    return t;
}

static ScenarioConfig defaultCfg(int n = 10000, uint64_t seed = 0xC0FFEE1234ULL) {
    ScenarioConfig cfg;
    cfg.num_simulations      = n;
    cfg.rng_seed             = seed;
    cfg.team_a_has_home_court = true;
    cfg.team_a_star_avail    = 1.0;
    cfg.team_b_star_avail    = 1.0;
    return cfg;
}

// ---------------------------------------------------------------------------
// GameProbabilityModel — baseline
// ---------------------------------------------------------------------------

TEST(GameProbabilityModelTest, BasicWinProb) {
    TeamProfile a = makeTeam(10.0);
    TeamProfile b = makeTeam(0.0);
    ScenarioConfig cfg = defaultCfg();
    GameProbabilityModel model;
    double p = model.computeWinProbability(a, b, cfg, /*a_is_home=*/true);
    EXPECT_GT(p, 0.5);
}

TEST(GameProbabilityModelTest, HomeCourtAdvantageIsSymmetric) {
    TeamProfile a = makeTeam(0.0);
    TeamProfile b = makeTeam(0.0);
    ScenarioConfig cfg = defaultCfg();
    GameProbabilityModel model;
    double home_p = model.computeWinProbability(a, b, cfg, true);
    double away_p = model.computeWinProbability(a, b, cfg, false);
    EXPECT_NEAR(home_p + away_p, 1.0, 1e-9);
    EXPECT_GT(home_p, 0.5);
}

TEST(GameProbabilityModelTest, ProbabilityClampedAbove) {
    // Extreme net rating advantage should not reach 1.0
    TeamProfile a = makeTeam(50.0);
    TeamProfile b = makeTeam(-50.0);
    GameProbabilityModel model;
    double p = model.computeWinProbability(a, b, defaultCfg(), true);
    EXPECT_LT(p, 1.0);
    EXPECT_GE(p, 0.0);
}

TEST(GameProbabilityModelTest, ProbabilityClampedBelow) {
    TeamProfile a = makeTeam(-50.0);
    TeamProfile b = makeTeam(50.0);
    GameProbabilityModel model;
    double p = model.computeWinProbability(a, b, defaultCfg(), false);
    EXPECT_GT(p, 0.0);
    EXPECT_LE(p, 1.0);
}

TEST(GameProbabilityModelTest, StarUnavailabilityReducesWinProb) {
    TeamProfile a = makeTeam(5.0);
    TeamProfile b = makeTeam(0.0);
    GameProbabilityModel model;
    ScenarioConfig full = defaultCfg();
    full.team_a_star_avail = 1.0;
    ScenarioConfig injured = defaultCfg();
    injured.team_a_star_avail = 0.0;
    double p_full    = model.computeWinProbability(a, b, full,    true);
    double p_injured = model.computeWinProbability(a, b, injured, true);
    EXPECT_GT(p_full, p_injured);
}

// ---------------------------------------------------------------------------
// GameProbabilityModel — Four Factors (Feature 3)
// ---------------------------------------------------------------------------

TEST(GameProbabilityModelTest, HigherEFGWins) {
    // Team A has better eFG%, everything else equal
    TeamProfile a = makeTeam(0.0, 0.5, /*eFG=*/0.58, 0.13, 0.25, 0.20);
    TeamProfile b = makeTeam(0.0, 0.5, /*eFG=*/0.52, 0.13, 0.25, 0.20);
    GameProbabilityModel model;
    double p = model.computeWinProbability(a, b, defaultCfg(), false);
    EXPECT_GT(p, 0.5);
}

TEST(GameProbabilityModelTest, LowerTurnoverRateWins) {
    // Team A has lower TOV% (better ball security), everything else equal
    TeamProfile a = makeTeam(0.0, 0.5, 0.55, /*TOV=*/0.10, 0.25, 0.20);
    TeamProfile b = makeTeam(0.0, 0.5, 0.55, /*TOV=*/0.16, 0.25, 0.20);
    GameProbabilityModel model;
    double p = model.computeWinProbability(a, b, defaultCfg(), false);
    EXPECT_GT(p, 0.5);
}

TEST(GameProbabilityModelTest, FourFactorsCombinedShiftsProbability) {
    // Team A dominates all four factors — should produce a meaningfully higher win prob
    // than equal teams even without a net rating edge.
    TeamProfile a = makeTeam(0.0, 0.5, 0.58, 0.10, 0.30, 0.22);
    TeamProfile b = makeTeam(0.0, 0.5, 0.52, 0.16, 0.20, 0.16);
    GameProbabilityModel model;
    double p = model.computeWinProbability(a, b, defaultCfg(), false);
    EXPECT_GT(p, 0.55);  // should be a real advantage, not noise
}

TEST(GameProbabilityModelTest, EqualFourFactorsNoEdge) {
    // Same four factors, same net rating, neutral court → should be near 0.5
    TeamProfile a = makeTeam(0.0, 0.5, 0.55, 0.13, 0.25, 0.20);
    TeamProfile b = makeTeam(0.0, 0.5, 0.55, 0.13, 0.25, 0.20);
    GameProbabilityModel model;
    double p = model.computeWinProbability(a, b, defaultCfg(), false);
    EXPECT_NEAR(p, 0.5, 0.02);
}
