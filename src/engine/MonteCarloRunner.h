#pragma once
#include "engine/GameProbabilityModel.h"
#include "engine/SeriesSimulator.h"
#include "models/TeamProfile.h"
#include "models/ScenarioConfig.h"
#include "models/SimulationResult.h"

namespace ss {

// Runs N Monte Carlo simulations across T worker threads.
//
// Threading model:
//   - hardware_concurrency() threads are spawned.
//   - Simulations are partitioned evenly; any remainder goes to thread 0.
//   - Each thread receives a unique seed derived deterministically from the
//     master seed via Knuth's multiplicative hash, ensuring identical global
//     seeds always produce identical global results regardless of thread count.
//   - Each thread accumulates a local AggregatedResults; the main thread
//     merges all local results after joining — no locking during the hot path.
//
// The runner is stateless between calls and safe to reuse.
class MonteCarloRunner {
public:
    MonteCarloRunner();

    AggregatedResults run(
        const TeamProfile&    a,
        const TeamProfile&    b,
        const ScenarioConfig& cfg) const;

private:
    GameProbabilityModel model_;
    SeriesSimulator      simulator_;

    static void workerFn(
        const SeriesSimulator& sim,
        const TeamProfile&     a,
        const TeamProfile&     b,
        const ScenarioConfig&  cfg,
        uint64_t               thread_seed,
        int                    num_trials,
        AggregatedResults&     out_result);

    static uint64_t deriveThreadSeed(uint64_t master, int thread_index);
};

} // namespace ss
