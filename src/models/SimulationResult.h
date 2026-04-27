#pragma once

#include <array>
#include <algorithm>

namespace ss {

struct SimulationResult {
    bool team_a_wins  = false;
    int  games_played = 0;
};

struct AggregatedResults {
    int total_simulations  = 0;
    int team_a_series_wins = 0;
    int team_b_series_wins = 0;
    std::array<int, 4> team_a_by_length {0, 0, 0, 0};
    std::array<int, 4> team_b_by_length {0, 0, 0, 0};
    double avg_series_length = 0.0;

    void merge(const AggregatedResults& other) {
        total_simulations  += other.total_simulations;
        team_a_series_wins += other.team_a_series_wins;
        team_b_series_wins += other.team_b_series_wins;

        for (std::size_t i = 0; i < team_a_by_length.size(); ++i) {
            team_a_by_length[i] += other.team_a_by_length[i];
            team_b_by_length[i] += other.team_b_by_length[i];
        }
    }

    double team_a_win_pct() const {
        return total_simulations > 0
            ? static_cast<double>(team_a_series_wins) / static_cast<double>(total_simulations)
            : 0.0;
    }

    double team_b_win_pct() const {
        return total_simulations > 0
            ? static_cast<double>(team_b_series_wins) / static_cast<double>(total_simulations)
            : 0.0;
    }

    bool most_likely_winner_is_a() const {
        return team_a_series_wins >= team_b_series_wins;
    }

    int most_likely_length() const {
        const auto& bucket = most_likely_winner_is_a() ? team_a_by_length : team_b_by_length;
        auto        it     = std::max_element(bucket.begin(), bucket.end());
        return static_cast<int>(std::distance(bucket.begin(), it)) + 4;
    }
};

} // namespace ss

namespace seriesscope {
using SimulationResult  = ss::SimulationResult;
using AggregatedResults = ss::AggregatedResults;
}
