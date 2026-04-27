#include <gtest/gtest.h>
#include "../src/data/DatabaseManager.h"
#include "../src/data/PlayerRepository.h"

using namespace seriesscope;

static PlayerProfile makePlayer(const std::string& id, const std::string& team, double minutes) {
    PlayerProfile p;
    p.player_id   = id;
    p.player_name = "Player " + id;
    p.team_id     = team;
    p.minutes     = minutes;
    p.gp          = 30;
    return p;
}

class PlayerRepositoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        db_ = std::make_unique<DatabaseManager>(":memory:");
        db_->createSchema();
        repo_ = std::make_unique<PlayerRepository>(*db_);
    }

    std::unique_ptr<DatabaseManager>   db_;
    std::unique_ptr<PlayerRepository>  repo_;
};

TEST_F(PlayerRepositoryTest, UpsertAndRetrieveByTeam) {
    repo_->upsertPlayer(makePlayer("p1", "BOS", 35.0));
    repo_->upsertPlayer(makePlayer("p2", "BOS", 30.0));

    auto players = repo_->getPlayersByTeam("BOS");
    ASSERT_EQ(players.size(), 2u);
    EXPECT_EQ(players[0].player_id, "p1");  // ordered by minutes DESC
    EXPECT_EQ(players[1].player_id, "p2");
}

TEST_F(PlayerRepositoryTest, TopFiveCutoffPerTeam) {
    // Insert 6 players for BOS, 6 for MIA
    for (int i = 0; i < 6; ++i)
        repo_->upsertPlayer(makePlayer("b" + std::to_string(i), "BOS", 40.0 - i));
    for (int i = 0; i < 6; ++i)
        repo_->upsertPlayer(makePlayer("m" + std::to_string(i), "MIA", 38.0 - i));

    auto matchup = repo_->getPlayersForMatchup("BOS", "MIA");
    EXPECT_EQ(matchup.size(), 10u);  // 5 per team
}

TEST_F(PlayerRepositoryTest, GetPlayersForMatchupOrdering) {
    for (int i = 0; i < 5; ++i)
        repo_->upsertPlayer(makePlayer("b" + std::to_string(i), "BOS", 40.0 - i * 2));
    for (int i = 0; i < 5; ++i)
        repo_->upsertPlayer(makePlayer("m" + std::to_string(i), "MIA", 36.0 - i * 2));

    auto matchup = repo_->getPlayersForMatchup("BOS", "MIA");
    ASSERT_EQ(matchup.size(), 10u);
    // First 5 are BOS (team_a), next 5 are MIA (team_b)
    for (int i = 0; i < 5; ++i) EXPECT_EQ(matchup[i].team_id, "BOS");
    for (int i = 5; i < 10; ++i) EXPECT_EQ(matchup[i].team_id, "MIA");
}

TEST_F(PlayerRepositoryTest, UpsertReplaces) {
    auto p = makePlayer("p1", "BOS", 35.0);
    repo_->upsertPlayer(p);
    p.minutes = 99.0;
    repo_->upsertPlayer(p);  // replace

    auto players = repo_->getPlayersByTeam("BOS");
    ASSERT_EQ(players.size(), 1u);
    EXPECT_DOUBLE_EQ(players[0].minutes, 99.0);
}

TEST_F(PlayerRepositoryTest, EmptyTeamReturnsEmpty) {
    auto players = repo_->getPlayersByTeam("NONEXISTENT");
    EXPECT_TRUE(players.empty());
}
