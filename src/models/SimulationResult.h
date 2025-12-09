#pragma once
#include <vector>
#include <string>

namespace seriesscope {
struct SimulationResult {
    bool home_won_series;
    int home_games_won;
    int away_games_won;
    std::vector<int> game_winners; // 0 for home, 1 for away
};

struct AggregatedResults {
    double home_win_prob;
    double avg_games;
    std::vector<double> outcome_probs; // [H4-0, H4-1, ..., A4-0]
    int simulation_count;
};
}
