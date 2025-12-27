#include "MonteCarloRunner.h"
#include "SeriesSimulator.h"
#include <thread>
#include <future>
#include <vector>

namespace seriesscope {
AggregatedResults MonteCarloRunner::run(const TeamProfile& home, const TeamProfile& away, const ScenarioConfig& config, int iterations, std::function<void(int)> progressCallback) {
    int threads = std::thread::hardware_concurrency();
    if (threads == 0) threads = 1;
    int per_thread = iterations / threads;
    
    std::vector<std::future<int>> futures;
    for (int t = 0; t < threads; ++t) {
        futures.push_back(std::async(std::launch::async, [=, &home, &away, &config]() {
            // Knuth hash for seed
            uint32_t seed = t * 2654435761U;
            std::mt19937 gen(seed);
            SeriesSimulator sim(home, away, config);
            int wins = 0;
            for (int i = 0; i < per_thread; ++i) {
                if (sim.simulateOneSeries(gen).home_won_series) wins++;
            }
            return wins;
        }));
    }
    
    int total_wins = 0;
    for (auto& f : futures) total_wins += f.get();
    
    AggregatedResults res;
    res.home_win_prob = (double)total_wins / iterations;
    res.simulation_count = iterations;
    return res;
}
}
