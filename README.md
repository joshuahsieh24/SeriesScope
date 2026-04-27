# SeriesScope

**A native C++ desktop simulator for NBA playoff best-of-7 series, powered by real NBA efficiency data.**

SeriesScope runs tens of thousands of Monte Carlo simulations to produce statistically grounded series win probabilities — not expert opinions, not vibes. Every prediction is traceable to real numbers: opponent-adjusted efficiency ratings, momentum, home court schedule, roster health, and team volatility. The model's assumptions are visible, adjustable, and documented.

---

## What it does

Select any two NBA playoff teams, configure scenario variables, and run a simulation. SeriesScope produces:

- **Series win probability** for each team, derived from 10,000 simulated best-of-7 series
- **Series length distribution** — how often the series ends in 4, 5, 6, or 7 games
- **Ranked factor breakdown** — which variables are actually driving the prediction and by how much
- **Confidence classification** — Dominant Favorite / Clear Favorite / Volatile Upset Risk / Coin Flip, based on win probability and combined team volatility
- **Scenario comparison** — store a baseline result, change a variable, re-run, and see the exact probability delta

---

## Why the predictions are more than common sense

The model uses **opponent-adjusted net rating** as its primary signal — not win-loss record. Net rating measures how many points per 100 possessions a team outscores opponents by, after accounting for strength of schedule. A 45-win team with a weak schedule can have a worse net rating than a 42-win team that played elite competition. The model sees through the record to the underlying efficiency.

The net rating values are fetched directly from NBA.com via the `nba_api` ingestion pipeline (`E_NET_RATING` from `LeagueDashTeamStats` Advanced) — these are the league's own official pace-adjusted, opponent-adjusted efficiency numbers.

### The win probability formula

```
P(A wins game) = logistic(total / 7.0)

total = (net_rating_delta × 0.50)    ← opponent-adjusted efficiency edge
      + (win_pct_delta   × 1.00)     ← execution and clutch signal
      + (form_delta      × 1.50)     ← last-10-game momentum
      ± 3.20                         ← home court (symmetric, ±3.2 pts)
      + star_penalty                 ← (1 − availability) × star_impact × 4.0
```

**Calibration anchors:**
- Scale of `7.0` → a 7-point net rating edge maps to ~73% per-game win probability, consistent with NBA point-spread research
- Home court value of `3.2` → `logistic(3.2 / 7.0) ≈ 61.8%`, matching the NBA's historical home win rate
- Form scale of `1.5` → max momentum swing is half a home-court advantage, not dominant
- Star penalty of `4.0` pts max → calibrated from Win Share production of franchise-level stars

### Why compounding across 7 games matters

Most casual predictions dramatically underestimate how much variance exists in a short series. A team with a 65% per-game edge only wins the series ~77% of the time. A 55% per-game favorite wins the series just ~61% of the time. SeriesScope simulates the full 7-game distribution using the real NBA 2-2-1-1-1 home court schedule, tracking exactly which games are home and away for each team.

### Volatility is modeled, not ignored

Streaky, star-dependent teams produce more chaotic series outcomes than consistent system teams — even at identical win probabilities. Noise is injected in logit space (not directly on the probability) to preserve mathematically valid bounds while making high-volatility matchups genuinely more unpredictable.

### Star availability as a nonlinear penalty

The penalty scales by how star-dependent the team is (`star_impact`) multiplied by how unavailable they are. A roster with `star_impact = 0.9` at 30% availability loses nearly 2.5 points off their effective net rating. A deep, system team with `star_impact = 0.3` loses almost nothing from the same slider. The model treats roster construction as a variable.

---

## Scenario analysis

The most analytically useful feature is counterfactual simulation.

1. Run a baseline — e.g. OKC vs MIN, OKC has home court
2. Click **Store as Baseline**
3. Reduce OKC's star availability slider to 40% (simulating injury)
4. Re-run

The center panel shows: `Baseline: OKC 71% — Adjustment favors MIN by +14pp (71% → 57%)`

This makes the model's assumptions explicit and testable. You can isolate exactly how much any single variable is worth in series win probability.

---

## Data pipeline

Team stats are fetched from NBA.com via a Python ingestion script and written to a local SQLite database. The C++ app reads only from local storage — no network calls at runtime, no API dependency at launch.

```bash
# One-time setup
python3 -m venv .venv && source .venv/bin/activate
pip install -r tools/requirements.txt

# Fetch current season stats
python3 tools/ingest_nba_data.py

# Specific season
python3 tools/ingest_nba_data.py --season 2024-25

# Launch the app (from project root)
./build/SeriesScope
```

The status bar shows when data was last ingested and from what source. On first launch with no database, the app falls back to a bundled fixture covering 16 playoff teams with realistic stats.

### What gets fetched

| Source | Fields | Provenance |
|--------|--------|-----------|
| `LeagueStandingsV3` | Conference, seed, wins, losses, home/road record | Exact |
| `LeagueDashTeamStats` (Advanced) | `E_OFF_RATING`, `E_DEF_RATING`, `E_NET_RATING`, `E_PACE` | Exact |
| `TeamGameLogs` | Last 10/20 game results and plus/minus | Derived |

### Derived fields and formulas

| Field | Formula | Accuracy |
|-------|---------|----------|
| `recent_form` | `(wins_last10 − 5) / 5` → [−1, 1] | Derived |
| `home_strength` | `0.30 + (home_win% − 0.40) / 0.50 × 0.70` | Derived |
| `away_strength` | `0.15 + (road_win% − 0.20) / 0.50 × 0.85` | Derived |
| `volatility` | `(std_dev(PLUS_MINUS last 20) − 6) / 18` | Derived |
| `star_impact` | Approximated from net rating and volatility | Approx |
| `depth_rating` | Approximated from consistency and win% | Approx |

---

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│  UI Layer  (Qt Widgets, dark stylesheet)                        │
│  MainWindow → TeamSelectionPanel                                │
│            → ScenarioControlsPanel                              │
│            → ResultsPanel                                       │
└────────────────────────┬────────────────────────────────────────┘
                         │ Qt signals/slots
┌────────────────────────▼────────────────────────────────────────┐
│  ApplicationCoordinator  (QObject, orchestration + persistence) │
│  Owns DatabaseManager, TeamRepository, FixtureLoader            │
│  Dispatches simulation via QtConcurrent, emits to UI thread     │
└───────────┬───────────────────────────────┬─────────────────────┘
            │                               │
┌───────────▼──────────────┐   ┌────────────▼───────────────────┐
│  Engine Layer (Qt-free,  │   │  Data Layer                    │
│  fully unit-testable)    │   │  DatabaseManager  (RAII SQLite) │
│                          │   │  TeamRepository   (CRUD)        │
│  GameProbabilityModel    │   │  FixtureLoader    (JSON seed)   │
│  SeriesSimulator         │   └────────────────────────────────┘
│  MonteCarloRunner        │
│  ExplanationEngine       │
└──────────────────────────┘
```

**Key architectural decisions:**

- The engine layer has zero Qt dependency and is fully unit-tested headlessly — simulation logic is completely decoupled from the UI
- `ApplicationCoordinator` owns all subsystem lifetimes via RAII; the UI thread never blocks during simulation
- `MonteCarloRunner` partitions work across `hardware_concurrency()` threads, each with a unique seed derived via Knuth's multiplicative hash (`master ^ (i+1) × 0x9e3779b97f4a7c15`), and accumulates into local result buffers — no locking in the hot path
- Same `rng_seed` always produces byte-identical results regardless of platform or run, verified by the test suite
- Python owns data acquisition; C++ owns simulation, analytics, and UI — clean boundary with no runtime Python dependency

---

## Building

### macOS
```bash
brew install cmake qt@6 sqlite3
cmake -B build -DCMAKE_PREFIX_PATH=$(brew --prefix qt@6)
cmake --build build -j$(nproc)
./build/SeriesScope
```

### Ubuntu
```bash
sudo apt install cmake qt6-base-dev libqt6concurrent6-dev libsqlite3-dev
cmake -B build && cmake --build build -j$(nproc)
./build/SeriesScope
```

### Windows (MSVC + vcpkg)
```powershell
vcpkg install qt6 sqlite3
cmake -B build -DCMAKE_TOOLCHAIN_FILE=<vcpkg_root>/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```

### Running tests
```bash
cmake -B build -DBUILD_TESTS=ON
cmake --build build -j$(nproc)
./build/tests/ss_tests
```

---

## Project structure

```
SeriesScope/
├── CMakeLists.txt
├── fixtures/
│   └── sample_teams.json          # 16-team playoff fixture (offline fallback)
├── src/
│   ├── models/                    # Pure value types — no Qt, no logic
│   │   ├── TeamProfile.h
│   │   ├── ScenarioConfig.h
│   │   ├── SimulationResult.h
│   │   └── FactorExplanation.h
│   ├── utils/
│   │   └── StringUtils.h
│   ├── data/                      # RAII SQLite + repositories
│   │   ├── DatabaseManager.h/.cpp
│   │   ├── TeamRepository.h/.cpp
│   │   └── FixtureLoader.h/.cpp
│   ├── engine/                    # Qt-free, fully testable
│   │   ├── GameProbabilityModel.h/.cpp
│   │   ├── SeriesSimulator.h/.cpp
│   │   ├── MonteCarloRunner.h/.cpp
│   │   └── ExplanationEngine.h/.cpp
│   ├── ui/                        # Qt Widgets
│   │   ├── StyleSheet.h
│   │   ├── MainWindow.h/.cpp
│   │   ├── TeamSelectionPanel.h/.cpp
│   │   ├── ScenarioControlsPanel.h/.cpp
│   │   └── ResultsPanel.h/.cpp
│   ├── app/
│   │   └── ApplicationCoordinator.h/.cpp
│   └── main.cpp
├── tests/
│   └── test_engine.cpp            # 19 unit tests
└── tools/
    ├── ingest_nba_data.py         # nba_api → SQLite + JSON
    ├── derive_team_profiles.py    # normalization functions
    ├── test_derive.py             # 23 pytest tests
    └── requirements.txt
```

---

## Known limitations

| Decision | Tradeoff |
|----------|----------|
| Team-level net rating as primary signal | Ignores matchup-specific defense and individual player matchups |
| Fixed home court value (3.2 pts) | Historical average — teams vary significantly |
| Star impact and depth are approximations | Accurate player-level modeling requires per-game individual stats |
| No mid-series updating | Model uses pre-series ratings; does not incorporate actual game results |
| SQLite over in-memory store | Enables result history and reproducibility; adds I/O on every run |

---

## Potential extensions

- **Bayesian updating** — adjust team strength priors from actual playoff results as the series progresses
- **Player-level modeling** — disaggregate team ratings into per-player contribution curves
- **Series replay** — store and step through per-game outcomes from any simulation
- **In-series conditioning** — after Game 1, simulator knows the series is now a best-of-6 and re-weights accordingly
- **ELO-style dynamic ratings** — replace static net rating with ratings that update after each playoff game
