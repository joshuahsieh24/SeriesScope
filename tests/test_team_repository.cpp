#include <gtest/gtest.h>
#include "../src/data/DatabaseManager.h"
#include "../src/data/TeamRepository.h"

using namespace seriesscope;

static TeamProfile makeTeam(const std::string& id, const std::string& name,
                             double net_rating = 5.0) {
    TeamProfile t;
    t.id           = id;
    t.name         = name;
    t.abbreviation = id.substr(0, 3);
    t.net_rating   = net_rating;
    t.off_rating   = 115.0;
    t.def_rating   = 110.0;
    t.wins         = 50;
    t.losses       = 32;
    t.eFG_pct      = 0.55;
    t.TOV_pct      = 0.13;
    t.ORB_pct      = 0.25;
    t.FT_rate      = 0.20;
    t.volatility   = 0.20;
    t.star_impact  = 1.0;
    t.depth_rating = 0.50;
    return t;
}

class TeamRepositoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        db_   = std::make_unique<DatabaseManager>(":memory:");
        db_->createSchema();
        repo_ = std::make_unique<TeamRepository>(*db_);
    }
    std::unique_ptr<DatabaseManager> db_;
    std::unique_ptr<TeamRepository>  repo_;
};

TEST_F(TeamRepositoryTest, GetByIdReturnsNulloptForMissingTeam) {
    auto result = repo_->getTeamById("NONEXISTENT");
    EXPECT_FALSE(result.has_value());
}

TEST_F(TeamRepositoryTest, GetByIdReturnsTeamWhenPresent) {
    repo_->upsertTeam(makeTeam("BOS", "Boston Celtics", 11.6));
    auto result = repo_->getTeamById("BOS");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->id,   "BOS");
    EXPECT_EQ(result->name, "Boston Celtics");
    EXPECT_DOUBLE_EQ(result->net_rating, 11.6);
}

TEST_F(TeamRepositoryTest, UpsertAndRetrieveRoundTrip) {
    auto original = makeTeam("DEN", "Denver Nuggets", 5.5);
    original.eFG_pct  = 0.564;
    original.TOV_pct  = 0.126;
    original.ORB_pct  = 0.292;
    original.FT_rate  = 0.174;
    repo_->upsertTeam(original);

    auto fetched = repo_->getTeamById("DEN");
    ASSERT_TRUE(fetched.has_value());
    EXPECT_DOUBLE_EQ(fetched->eFG_pct, 0.564);
    EXPECT_DOUBLE_EQ(fetched->TOV_pct, 0.126);
    EXPECT_DOUBLE_EQ(fetched->ORB_pct, 0.292);
    EXPECT_DOUBLE_EQ(fetched->FT_rate, 0.174);
}

TEST_F(TeamRepositoryTest, UpsertReplacesExistingTeam) {
    repo_->upsertTeam(makeTeam("OKC", "Oklahoma City Thunder", 8.0));
    auto updated = makeTeam("OKC", "Oklahoma City Thunder", 12.5);
    repo_->upsertTeam(updated);

    auto fetched = repo_->getTeamById("OKC");
    ASSERT_TRUE(fetched.has_value());
    EXPECT_DOUBLE_EQ(fetched->net_rating, 12.5);
}

TEST_F(TeamRepositoryTest, GetAllTeamsReturnsAllUpserted) {
    repo_->upsertTeam(makeTeam("BOS", "Boston Celtics"));
    repo_->upsertTeam(makeTeam("MIA", "Miami Heat"));
    repo_->upsertTeam(makeTeam("MIL", "Milwaukee Bucks"));
    auto all = repo_->getAllTeams();
    EXPECT_EQ(all.size(), 3u);
}
