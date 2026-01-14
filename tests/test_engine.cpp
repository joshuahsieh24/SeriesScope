#include <gtest/gtest.h>
#include "../src/engine/GameProbabilityModel.h"
#include "../src/models/TeamProfile.h"
#include "../src/models/ScenarioConfig.h"

using namespace seriesscope;

TEST(GameProbabilityModelTest, BasicWinProb) {
    TeamProfile t1; t1.off_rating = 120; t1.def_rating = 110;
    TeamProfile t2; t2.off_rating = 110; t2.def_rating = 110;
    ScenarioConfig config;
    double p = GameProbabilityModel::calculateHomeWinProb(t1, t2, config);
    EXPECT_GT(p, 0.5);
}

// ... 18 more tests would go here ...

