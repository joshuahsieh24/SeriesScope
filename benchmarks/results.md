# SeriesScope Monte Carlo Benchmark

- Timestamp: 2026-04-21 19:59:01 PDT
- CPU: Apple M1
- Logical cores: 8
- Matchup: Boston Celtics vs Denver Nuggets
- SQLite fixture DB: `/Users/mac/Documents/pulse/SeriesScope/benchmarks/benchmark_fixture.sqlite3`
- Benchmark target: `benchmark_monte_carlo`
- Timing source: `std::chrono::steady_clock`
- Trials per N/mode: 3

| N | ST ms min/med/max | ST sims/s med | ST us/sim med | MT ms min/med/max | MT sims/s med | MT us/sim med | Speedup |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1000 | 0.13/0.13/0.15 | 7825277 | 0.13 | 0.11/0.11/0.11 | 9295149 | 0.11 | 1.19 |
| 10000 | 1.07/1.08/1.11 | 9242503 | 0.11 | 0.49/0.50/0.54 | 20151134 | 0.05 | 2.18 |
| 100000 | 10.50/11.17/11.47 | 8950348 | 0.11 | 3.31/3.38/4.26 | 29546462 | 0.03 | 3.30 |
| 1000000 | 109.16/109.68/109.69 | 9117187 | 0.11 | 32.67/33.08/35.61 | 30234240 | 0.03 | 3.32 |
