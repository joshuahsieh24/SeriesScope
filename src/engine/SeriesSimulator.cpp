#include "SeriesSimulator.h"
#include "GameProbabilityModel.h"

namespace seriesscope {
SeriesSimulator::SeriesSimulator(const TeamProfile& h, const TeamProfile& a, const ScenarioConfig& c) : home(h), away(a), config(c) {}

SimulationResult SeriesSimulator::simulateOneSeries(std::mt19937& gen) {
    SimulationResult result{false, 0, 0, {}};
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    
    // 2-2-1-1-1 format
    bool home_at_home[] = {true, true, false, false, true, false, true};
    
    for (int i = 0; i < config.series_length; ++i) {
        double p;
        if (home_at_home[i]) {
            p = GameProbabilityModel::calculateHomeWinProb(home, away, config);
        } else {
            p = 1.0 - GameProbabilityModel::calculateHomeWinProb(away, home, config);
        }
        
        if (dist(gen) < p) {
            result.home_games_won++;
            result.game_winners.push_back(0);
        } else {
            result.away_games_won++;
            result.game_winners.push_back(1);
        }
        
        if (result.home_games_won > config.series_length / 2) {
            result.home_won_series = true;
            break;
        }
        if (result.away_games_won > config.series_length / 2) {
            result.home_won_series = false;
            break;
        }
    }
    return result;
}
}
