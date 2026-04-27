#include "engine/SeriesSimulator.h"

#include <array>

namespace ss {

SeriesSimulator::SeriesSimulator(const GameProbabilityModel& model)
    : model_(model) {}

SimulationResult SeriesSimulator::simulateOnce(
    const TeamProfile&    a,
    const TeamProfile&    b,
    const ScenarioConfig& cfg,
    std::mt19937_64&      rng) const
{
    static constexpr std::array<bool, 7> a_home_games {true, true, false, false, true, false, true};

    std::uniform_real_distribution<double> dist(0.0, 1.0);
    SimulationResult result;
    int wins_a = 0;
    int wins_b = 0;
    const int wins_needed = (cfg.series_length / 2) + 1;

    for (int game_index = 0; game_index < cfg.series_length; ++game_index) {
        const bool   a_is_home = cfg.team_a_has_home_court ? a_home_games[game_index] : !a_home_games[game_index];
        const double win_prob  = model_.computeWinProbability(a, b, cfg, a_is_home);

        if (dist(rng) < win_prob) {
            ++wins_a;
        } else {
            ++wins_b;
        }

        result.games_played = game_index + 1;

        if (wins_a >= wins_needed) {
            result.team_a_wins = true;
            return result;
        }

        if (wins_b >= wins_needed) {
            result.team_a_wins = false;
            return result;
        }
    }

    result.team_a_wins = wins_a >= wins_b;
    return result;
}

} // namespace ss
