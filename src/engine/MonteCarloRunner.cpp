#include "engine/MonteCarloRunner.h"
#include <thread>
#include <vector>
#include <algorithm>

namespace ss {

MonteCarloRunner::MonteCarloRunner()
    : model_()
    , simulator_(model_)
{}

// static
uint64_t MonteCarloRunner::deriveThreadSeed(uint64_t master, int thread_index) {
    // Knuth multiplicative hash to spread seeds independently.
    // Using 0x9e3779b97f4a7c15 (golden-ratio-based constant) ensures that
    // even adjacent thread indices map to distant seed values.
    return master ^ (static_cast<uint64_t>(thread_index + 1) * 0x9e3779b97f4a7c15ULL);
}

// static
void MonteCarloRunner::workerFn(
    const SeriesSimulator& sim,
    const TeamProfile&     a,
    const TeamProfile&     b,
    const ScenarioConfig&  cfg,
    uint64_t               thread_seed,
    int                    num_trials,
    AggregatedResults&     out)
{
    std::mt19937_64 rng(thread_seed);
    out.total_simulations = num_trials;
    double total_length   = 0.0;

    for (int i = 0; i < num_trials; ++i) {
        auto outcome = sim.simulateOnce(a, b, cfg, rng);
        int  idx     = outcome.games_played - 4;  // [0..3]

        if (outcome.team_a_wins) {
            ++out.team_a_series_wins;
            ++out.team_a_by_length[idx];
        } else {
            ++out.team_b_series_wins;
            ++out.team_b_by_length[idx];
        }
        total_length += outcome.games_played;
    }

    out.avg_series_length = num_trials > 0 ? total_length / num_trials : 0.0;
}

AggregatedResults MonteCarloRunner::run(
    const TeamProfile&    a,
    const TeamProfile&    b,
    const ScenarioConfig& cfg) const
{
    if (cfg.num_simulations <= 0) {
        return {};
    }

    const int hardware_threads = static_cast<int>(
        std::max(1u, std::thread::hardware_concurrency()));
    const int requested_threads = cfg.thread_count_override > 0
        ? cfg.thread_count_override
        : hardware_threads;
    const int num_threads  = std::max(1, std::min(requested_threads, cfg.num_simulations));
    const int base_sims    = cfg.num_simulations / num_threads;
    const int remainder    = cfg.num_simulations % num_threads;

    std::vector<AggregatedResults> thread_results(num_threads);
    std::vector<std::thread>       threads;
    threads.reserve(num_threads);

    for (int i = 0; i < num_threads; ++i) {
        int  this_sims = base_sims + (i == 0 ? remainder : 0);
        auto seed      = deriveThreadSeed(cfg.rng_seed, i);

        threads.emplace_back(
            workerFn,
            std::cref(simulator_),
            a, b, cfg,
            seed, this_sims,
            std::ref(thread_results[i]));
    }

    for (auto& t : threads) t.join();

    AggregatedResults final_result;
    double total_length_sum  = 0.0;
    int    total_sims_merged = 0;

    for (const auto& r : thread_results) {
        total_length_sum  += r.avg_series_length * r.total_simulations;
        total_sims_merged += r.total_simulations;
        final_result.merge(r);
    }

    if (total_sims_merged > 0)
        final_result.avg_series_length = total_length_sum / total_sims_merged;

    return final_result;
}

} // namespace ss
