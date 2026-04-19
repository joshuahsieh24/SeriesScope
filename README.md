SeriesScope
A native C++ desktop simulator for NBA playoff best-of-7 series, powered by real NBA efficiency data.

SeriesScope runs tens of thousands of Monte Carlo simulations to produce statistically grounded series win probabilities — not expert opinions, not vibes. Every prediction is traceable to real numbers: opponent-adjusted efficiency ratings, momentum, home court schedule, roster health, and team volatility. The model's assumptions are visible, adjustable, and documented.

What it does
Select any two NBA playoff teams, configure scenario variables, and run a simulation. SeriesScope produces:

Series win probability for each team, derived from 10,000 simulated best-of-7 series
Series length distribution — how often the series ends in 4, 5, 6, or 7 games
Ranked factor breakdown — which variables are actually driving the prediction and by how much
Confidence classification — Dominant Favorite / Clear Favorite / Volatile Upset Risk / Coin Flip, based on win probability and combined team volatility
Scenario comparison — store a baseline result, change a variable, re-run, and see the exact probability delta
Why the predictions are more than common sense
The model uses opponent-adjusted net rating as its primary signal — not win-loss record. Net rating measures how many points per 100 possessions a team outscores opponents by, after accounting for strength of schedule. A 45-win team with a weak schedule can have a worse net rating than a 42-win team that played elite competition. The model sees through the record to the underlying efficiency.

The key formula:

P(A wins game) = logistic(total / 7.0)
total = (net_rating_delta × 0.50)    ← opponent-adjusted efficiency edge
      + (win_pct_delta   × 1.00)     ← execution and clutch signal
      + (form_delta      × 1.50)     ← last-10-game momentum
      ± 3.20                         ← home court (symmetric, ±3.2 pts)
      + star_penalty                 ← (1 − availability) × star_impact × 4.0
Calibration: The logistic scale of 7.0 means a 7-point net rating edge maps to ~73% per-game win probability — consistent with how NBA point spreads translate to win probabilities. The home court value of 3.2 produces logistic(3.2/7.0) ≈ 61.8%, matching the NBA's historical home win rate.

Compounding across 7 games matters more than people think. A team with a 65% per-game edge only wins the series ~77% of the time. A 55% per-game favorite wins the series just ~61% of the time. Most casual predictions dramatically underestimate how much variance exists in a short series. SeriesScope quantifies the full distribution.

Volatility is modeled, not ignored. Streaky, star-dependent teams produce more chaotic series outcomes than consistent system teams — even at identical win probabilities. Noise is injected in logit space (not directly on the probability) to preserve mathematically valid bounds. Two teams at 60/40 with high volatility look completely different from two teams at 60/40 who are consistent.

Scenario analysis
The most analytically useful feature is counterfactual simulation.

Run a baseline — e.g. OKC vs MIN, OKC has home court
Click Store as Baseline
Reduce OKC's star availability slider to 40% (simulating injury)
Re-run
The center panel shows: Baseline: OKC 71% — Adjustment favors MIN by +14pp (71% → 57%)

This makes the model's assumptions explicit and testable. You can isolate exactly how much any single variable is worth in series win probability.

Data pipeline
Team stats are fetched from NBA.com via a Python ingestion script and written to a local SQLite database. The C++ app reads only from local storage — no network calls at runtime, no API dependency at launch.

# One-time setup
python3 -m venv .venv && source .venv/bin/activate
pip install -r tools/requirements.txt

# Fetch current season stats
python3 tools/ingest_nba_data.py

# Launch the app
./build/SeriesScope
The status bar shows when data was last ingested and from what source. On first launch with no database, the app falls back to a bundled fixture covering 16 playoff teams.

What gets fetched:

Source	Fields	Provenance
LeagueStandingsV3	Conference, seed, wins, losses, home/road record	Exact
LeagueDashTeamStats (Advanced)	Off rating, def rating, net rating, pace	Exact
TeamGameLogs	Last 10/20 game results and plus/minus	Derived
Derived fields (recent form, home/away strength, volatility) use documented formulas. Two fields — star impact and bench depth — are approximations built from correlated signals, and are labeled as such in the code and README.

Architecture
┌─────────────────────────────────────────────────────┐
│  UI Layer  (Qt Widgets, dark theme)                 │
│  MainWindow → TeamSelectionPanel                    │
│            → ScenarioControlsPanel                  │
│            → ResultsPanel                           │
└──────────────────────┬──────────────────────────────┘
                       │ Qt signals/slots
┌──────────────────────▼──────────────────────────────┐
│  ApplicationCoordinator                             │
│  Owns data layer, dispatches simulation via         │
│  QtConcurrent, emits simulationFinished to UI       │
└──────────┬──────────────────────┬───────────────────┘
           │                      │
┌──────────▼──────────┐  ┌───────▼───────────────────┐
│  Engine  (Qt-free)  │  │  Data Layer               │
│  GameProbabilityModel│  │  DatabaseManager (RAII)   │
│  SeriesSimulator    │  │  TeamRepository            │
│  MonteCarloRunner   │  │  FixtureLoader             │
│  ExplanationEngine  │  └───────────────────────────┘
└─────────────────────┘
The engine layer has zero Qt dependency and is fully unit-tested headlessly. The UI layer contains no business logic. The coordinator owns the lifetime of all subsystems via RAII and never lets the UI thread block during simulation.

Threading: MonteCarloRunner partitions simulations across hardware_concurrency() threads. Each thread gets a unique seed derived via Knuth's multiplicative hash (master ^ (i+1) × 0x9e3779b97f4a7c15), runs its own mt19937_64, and accumulates into a local result buffer. The main thread merges after joining — no locking in the hot path.

Determinism: The same rng_seed always produces byte-identical results regardless of platform or run. Verified by the test suite.

Building
macOS
brew install cmake qt@6 sqlite3
cmake -B build -DCMAKE_PREFIX_PATH=$(brew --prefix qt@6)
cmake --build build -j$(nproc)
./build/SeriesScope
Ubuntu
sudo apt install cmake qt6-base-dev libqt6concurrent6-dev libsqlite3-dev
cmake -B build && cmake --build build -j$(nproc)
Tests
cmake -B build -DBUILD_TESTS=ON
cmake --build build -j$(nproc)
./build/tests/ss_tests
Known limitations
Decision	Tradeoff
Team-level net rating as primary signal	Ignores matchup-specific defense, individual player matchups
Fixed home court value (3.2 pts)	Historical average — teams vary significantly
Star impact and depth are approximations	Accurate player-level modeling requires per-game individual stats
No mid-series updating	Model doesn't incorporate actual game results as the series progresses
SQLite over in-memory store	Enables result history and reproducibility; adds I/O on every run
Potential extensions
Bayesian updating — adjust team strength priors from actual playoff results as the series progresses
Player-level modeling — disaggregate team ratings into per-player contribution curves
Series replay — store and step through per-game outcomes from any simulation
Live ingestion — poll for updated stats during the playoffs with an IngestionWorker Qt thread
ELO-style strength — replace static net rating with dynamically updated ratings
