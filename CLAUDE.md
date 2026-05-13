# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this project is

SeriesScope is a native C++ desktop app that simulates NBA playoff best-of-7 series using Monte Carlo methods. It reads from a local SQLite database populated by a Python ingestion pipeline (`tools/ingest_nba_data.py`) and exposes results through a Qt Widgets UI. There are no network calls at runtime.

## Build commands

### Engine + tests only (no Qt required)
```bash
cmake -B build -DBUILD_TESTS=ON -DBUILD_BENCHMARKS=ON
cmake --build build -j$(nproc)
```

### Full desktop app (requires Qt 6)
```bash
# macOS
cmake -B build -DBUILD_APP=ON -DCMAKE_PREFIX_PATH=$(brew --prefix qt@6)
# Ubuntu
cmake -B build -DBUILD_APP=ON
cmake --build build -j$(nproc)
./build/SeriesScope
```

### Run all tests
```bash
./build/tests/series_scope_tests
```

### Run a single test (GoogleTest filter)
```bash
./build/tests/series_scope_tests --gtest_filter="GameProbabilityModelTest.BasicWinProb"
```

### Run benchmarks
```bash
./build/benchmark_monte_carlo
# Results written to benchmarks/results.md
```

### Python data pipeline
```bash
python3 -m venv .venv && source .venv/bin/activate
pip install -r tools/requirements.txt
python3 tools/ingest_nba_data.py           # current season
python3 tools/ingest_nba_data.py --season 2024-25
pytest tools/test_derive.py               # 23 Python unit tests
```

## Architecture

The codebase has three strictly layered groups. The engine layer has **zero Qt dependency** and is the primary unit-test target.

```
UI (Qt Widgets)
  └─ ApplicationCoordinator (QObject — owns subsystem lifetimes, dispatches to QtConcurrent)
       ├─ Data layer: DatabaseManager (RAII SQLite), TeamRepository, PlayerRepository, FixtureLoader
       └─ Engine layer (Qt-free): GameProbabilityModel → SeriesSimulator → MonteCarloRunner
                                  AwardEngine, ExplanationEngine
```

**Data flow for a simulation run:**
1. UI panels emit Qt signals to `ApplicationCoordinator::runSimulation`.
2. `ApplicationCoordinator` dispatches `MonteCarloRunner::run` via `QtConcurrent` so the UI thread never blocks.
3. `MonteCarloRunner` spawns `hardware_concurrency()` threads, each seeded via Knuth's multiplicative hash (`master ^ (i+1) × 0x9e3779b97f4a7c15`). No locking in the hot path — each thread writes to its own `AggregatedResults` and the main thread merges after joining.
4. Results are emitted back to the UI thread via `resultsReady` signal.

**Offline fallback:** On first launch with no SQLite database, `FixtureLoader` seeds from `fixtures/sample_teams.json` (16 playoff teams with realistic stats).

## Key types

- `TeamProfile` (`src/models/`) — pure value struct; all simulation inputs live here.
- `ScenarioConfig` (`src/models/`) — controls per-run parameters: `num_simulations`, `rng_seed`, `thread_count_override`, per-team availability/adjustment sliders.
- `AggregatedResults` (`src/models/SimulationResult.h`) — output of a full Monte Carlo run; includes per-length win-count arrays (`[4,5,6,7]` indexed as `[0..3]`).
- `GameProbabilityModel` — stateless, free-threaded. The win-probability formula uses a logistic function over a weighted sum of net rating delta, win-pct delta, form, home court (±3.2 pts), and star availability penalty.

## Namespaces

All engine/model code lives in `namespace ss`. The `seriesscope` namespace re-exports the same types via `using` aliases for compatibility with the data and UI layers that were written before `ss` was introduced.

## Testing notes

Tests use GoogleTest fetched at configure time. The test binary is `series_scope_tests`. The engine tests (`test_engine.cpp`, `test_award_engine.cpp`, `test_player_repository.cpp`) cover determinism guarantees — same `rng_seed` must produce byte-identical results regardless of thread count. Do not break this invariant when touching `MonteCarloRunner` or `deriveThreadSeed`.

## CMake options

| Option | Default | Effect |
|--------|---------|--------|
| `BUILD_APP` | OFF | Enables Qt6 find + UI compilation |
| `BUILD_TESTS` | OFF | Fetches GoogleTest and builds `series_scope_tests` |
| `BUILD_BENCHMARKS` | ON | Builds `benchmark_monte_carlo` |

The benchmark binary embeds `SERIESSCOPE_SOURCE_DIR` as a compile definition to locate `benchmarks/benchmark_fixture.sqlite3` and write `benchmarks/results.md` at runtime.
