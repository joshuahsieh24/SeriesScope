#include <gtest/gtest.h>
#include "../src/engine/AwardEngine.h"
#include "../src/models/PlayerProfile.h"
#include "../src/models/AwardResult.h"

using namespace seriesscope;

// Builds a minimal valid PlayerProfile.
static ss::PlayerProfile makePlayer(
    const std::string& name, const std::string& team,
    double minutes, double usg, double ts, double off_rtg, double def_rtg,
    double stl_pct, double blk_pct, double dreb_pct,
    double clutch_min, double clutch_pts, double clutch_pm)
{
    ss::PlayerProfile p;
    p.player_id   = name;
    p.player_name = name;
    p.team_id     = team;
    p.minutes     = minutes;
    p.usg_pct     = usg;
    p.ts_pct      = ts;
    p.off_rating  = off_rtg;
    p.def_rating  = def_rtg;
    p.stl_pct     = stl_pct;
    p.blk_pct     = blk_pct;
    p.dreb_pct    = dreb_pct;
    p.clutch_min  = clutch_min;
    p.clutch_pts  = clutch_pts;
    p.clutch_plus_minus = clutch_pm;
    return p;
}

// A realistic 10-player pool (5 per team)
static std::vector<ss::PlayerProfile> makePool() {
    return {
        makePlayer("Tatum",    "BOS", 36, 0.32, 0.61, 118, 112, 0.010, 0.008, 0.30, 8.0, 3.2,  2.5),
        makePlayer("Brown",    "BOS", 34, 0.25, 0.58, 115, 113, 0.012, 0.005, 0.20, 6.0, 2.0,  1.0),
        makePlayer("White",    "BOS", 30, 0.18, 0.56, 112, 108, 0.020, 0.007, 0.28, 4.0, 1.5,  0.5),
        makePlayer("Horford",  "BOS", 28, 0.14, 0.55, 110, 106, 0.008, 0.030, 0.35, 3.0, 0.8, -0.5),
        makePlayer("Holiday",  "BOS", 32, 0.16, 0.54, 111, 105, 0.025, 0.006, 0.22, 5.0, 1.8,  1.2),
        makePlayer("Adebayo",  "MIA", 34, 0.22, 0.57, 114, 103, 0.018, 0.025, 0.38, 7.0, 2.5,  3.0),
        makePlayer("Butler",   "MIA", 33, 0.28, 0.59, 116, 110, 0.016, 0.006, 0.24, 9.0, 4.5,  4.2),
        makePlayer("Herro",    "MIA", 31, 0.26, 0.57, 113, 114, 0.009, 0.003, 0.18, 5.0, 2.8,  0.8),
        makePlayer("Robinson", "MIA", 26, 0.10, 0.63, 109, 115, 0.007, 0.002, 0.16, 2.0, 1.0, -0.2),
        makePlayer("Vincent",  "MIA", 25, 0.15, 0.53, 108, 116, 0.014, 0.004, 0.20, 3.0, 1.2,  0.3),
    };
}

TEST(AwardEngineTest, MVPGoesToHighestUsageEfficiencyPlayer) {
    ss::AwardEngine engine;
    auto pool   = makePool();
    auto awards = engine.computeAwards(pool, "BOS", "MIA");
    // Tatum: 0.32×0.35 + 0.61×0.35 + (118/130)×0.30 ≈ 0.586 (highest)
    EXPECT_EQ(awards.mvp().player_name, "Tatum");
}

TEST(AwardEngineTest, DefenderGoesToBestDefensivePlayer) {
    ss::AwardEngine engine;
    auto pool   = makePool();
    auto awards = engine.computeAwards(pool, "BOS", "MIA");
    // Adebayo: def_rtg=103 (lowest), high block %, high dreb % → highest defender score
    EXPECT_EQ(awards.defender().player_name, "Adebayo");
}

TEST(AwardEngineTest, ClutchAwardHandlesAllZeroClutchMinutes) {
    ss::AwardEngine engine;
    auto pool = makePool();
    for (auto& p : pool) { p.clutch_min = 0.0; p.clutch_pts = 0.0; p.clutch_plus_minus = 0.0; }

    // Should not crash; rationale should note missing data
    auto awards = engine.computeAwards(pool, "BOS", "MIA");
    EXPECT_FALSE(awards.clutch().player_name.empty());
    EXPECT_EQ(awards.clutch().rationale, "Clutch data unavailable");
}

TEST(AwardEngineTest, XFactorPrefersHighEfficiencyLowUsagePlayer) {
    ss::AwardEngine engine;
    // Robinson: usg=0.10, ts=0.63 → ratio=6.3 — highest X-Factor by far
    auto pool   = makePool();
    auto awards = engine.computeAwards(pool, "BOS", "MIA");
    EXPECT_EQ(awards.xfactor().player_name, "Robinson");
}

TEST(AwardEngineTest, AllScoresInUnitRange) {
    ss::AwardEngine engine;
    auto awards = engine.computeAwards(makePool(), "BOS", "MIA");
    for (const auto& a : awards.awards) {
        EXPECT_GE(a.score, 0.0);
        EXPECT_LE(a.score, 1.0);
    }
}

TEST(AwardEngineTest, AllFourAwardsHaveNonEmptyPlayerName) {
    ss::AwardEngine engine;
    auto awards = engine.computeAwards(makePool(), "BOS", "MIA");
    for (const auto& a : awards.awards) {
        EXPECT_FALSE(a.player_name.empty());
    }
}

TEST(AwardEngineTest, AllFourAwardsHaveNonEmptyRationale) {
    ss::AwardEngine engine;
    auto awards = engine.computeAwards(makePool(), "BOS", "MIA");
    for (const auto& a : awards.awards) {
        EXPECT_FALSE(a.rationale.empty());
    }
}
