#include "data/DatabaseManager.h"
#include "data/TeamRepository.h"
#include "engine/MonteCarloRunner.h"
#include "models/ScenarioConfig.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#if defined(__APPLE__)
#include <sys/sysctl.h>
#endif

namespace {

using Clock = std::chrono::steady_clock;

struct TrialMetrics {
    double total_ms = 0.0;
    double sims_per_sec = 0.0;
    double micros_per_sim = 0.0;
};

struct SummaryStats {
    TrialMetrics min;
    TrialMetrics median;
    TrialMetrics max;
};

struct BenchRow {
    int n = 0;
    SummaryStats single;
    SummaryStats multi;
    int single_threads = 1;
    int multi_threads = 1;
};

std::string cpuBrand() {
#if defined(__APPLE__)
    size_t size = 0;
    if (sysctlbyname("machdep.cpu.brand_string", nullptr, &size, nullptr, 0) == 0 && size > 1) {
        std::string value(size, '\0');
        if (sysctlbyname("machdep.cpu.brand_string", value.data(), &size, nullptr, 0) == 0) {
            if (!value.empty() && value.back() == '\0') {
                value.pop_back();
            }
            if (!value.empty()) {
                return value;
            }
        }
    }

    if (sysctlbyname("hw.model", nullptr, &size, nullptr, 0) == 0 && size > 1) {
        std::string value(size, '\0');
        if (sysctlbyname("hw.model", value.data(), &size, nullptr, 0) == 0) {
            if (!value.empty() && value.back() == '\0') {
                value.pop_back();
            }
            return value;
        }
    }
#endif
    return "Unknown CPU";
}

template <typename T>
const T& medianOfThree(std::vector<T>& values) {
    std::sort(values.begin(), values.end(), [](const T& lhs, const T& rhs) {
        return lhs.total_ms < rhs.total_ms;
    });
    return values[1];
}

SummaryStats summarize(std::vector<TrialMetrics> trials) {
    std::sort(trials.begin(), trials.end(), [](const TrialMetrics& lhs, const TrialMetrics& rhs) {
        return lhs.total_ms < rhs.total_ms;
    });

    return SummaryStats {
        .min = trials.front(),
        .median = trials[1],
        .max = trials.back(),
    };
}

TrialMetrics runOnce(
    const ss::MonteCarloRunner& runner,
    const ss::TeamProfile&      teamA,
    const ss::TeamProfile&      teamB,
    const ss::ScenarioConfig&   cfg)
{
    const auto start = Clock::now();
    [[maybe_unused]] auto results = runner.run(teamA, teamB, cfg);
    const auto end = Clock::now();

    const double total_ms = std::chrono::duration<double, std::milli>(end - start).count();
    const double sims_per_sec = total_ms > 0.0
        ? (static_cast<double>(cfg.num_simulations) * 1000.0) / total_ms
        : 0.0;
    const double micros_per_sim = cfg.num_simulations > 0
        ? (total_ms * 1000.0) / static_cast<double>(cfg.num_simulations)
        : 0.0;

    return TrialMetrics {
        .total_ms = total_ms,
        .sims_per_sec = sims_per_sec,
        .micros_per_sim = micros_per_sim,
    };
}

SummaryStats benchmarkMode(
    const ss::MonteCarloRunner& runner,
    const ss::TeamProfile&      teamA,
    const ss::TeamProfile&      teamB,
    ss::ScenarioConfig          cfg)
{
    std::vector<TrialMetrics> trials;
    trials.reserve(3);

    for (int i = 0; i < 3; ++i) {
        cfg.rng_seed = 0xC0FFEE1234ULL + static_cast<uint64_t>(i);
        trials.push_back(runOnce(runner, teamA, teamB, cfg));
    }

    return summarize(std::move(trials));
}

ss::TeamProfile makeBoston() {
    ss::TeamProfile team;
    team.id = "1610612738";
    team.name = "Boston Celtics";
    team.abbreviation = "BOS";
    team.off_rating = 122.2;
    team.def_rating = 110.6;
    team.net_rating = 11.6;
    team.eFG_pct = 0.578;
    team.TOV_pct = 0.119;
    team.ORB_pct = 0.282;
    team.FT_rate = 0.187;
    team.primary_color = "#007A33";
    team.wins = 64;
    team.losses = 18;
    team.recent_form = 0.40;
    team.star_impact = 1.00;
    team.volatility = 0.18;
    team.depth_rating = 0.82;
    return team;
}

ss::TeamProfile makeDenver() {
    ss::TeamProfile team;
    team.id = "1610612743";
    team.name = "Denver Nuggets";
    team.abbreviation = "DEN";
    team.off_rating = 117.8;
    team.def_rating = 112.3;
    team.net_rating = 5.5;
    team.eFG_pct = 0.564;
    team.TOV_pct = 0.126;
    team.ORB_pct = 0.292;
    team.FT_rate = 0.174;
    team.primary_color = "#0E2240";
    team.wins = 57;
    team.losses = 25;
    team.recent_form = 0.20;
    team.star_impact = 1.00;
    team.volatility = 0.16;
    team.depth_rating = 0.73;
    return team;
}

std::filesystem::path sourceDir() {
    return std::filesystem::path(SERIESSCOPE_SOURCE_DIR);
}

std::filesystem::path benchmarkDbPath() {
    return sourceDir() / "benchmarks" / "benchmark_fixture.sqlite3";
}

std::filesystem::path resultsPath() {
    return sourceDir() / "benchmarks" / "results.md";
}

void seedBenchmarkDb(const std::filesystem::path& path) {
    std::filesystem::create_directories(path.parent_path());
    std::filesystem::remove(path);

    seriesscope::DatabaseManager db(path.string());
    db.createSchema();

    seriesscope::TeamRepository repo(db);
    repo.upsertTeam(makeBoston());
    repo.upsertTeam(makeDenver());
}

std::pair<ss::TeamProfile, ss::TeamProfile> loadMatchup(const std::filesystem::path& path) {
    seriesscope::DatabaseManager db(path.string());
    seriesscope::TeamRepository repo(db);
    return {
        repo.getTeamById("1610612738"),
        repo.getTeamById("1610612743"),
    };
}

std::string nowTimestamp() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm {};
#if defined(_WIN32)
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif
    std::ostringstream out;
    out << std::put_time(&tm, "%Y-%m-%d %H:%M:%S %Z");
    return out.str();
}

void printTable(const std::vector<BenchRow>& rows) {
    std::cout << "\nSeriesScope Monte Carlo Benchmark\n";
    std::cout << "Matchup: Boston Celtics vs Denver Nuggets\n";
    std::cout << "Threads: single=1, multi=" << std::max(1u, std::thread::hardware_concurrency()) << "\n\n";

    std::cout
        << std::left
        << std::setw(10) << "N"
        << std::setw(22) << "ST ms min/med/max"
        << std::setw(16) << "ST sims/s"
        << std::setw(16) << "ST us/sim"
        << std::setw(22) << "MT ms min/med/max"
        << std::setw(16) << "MT sims/s"
        << std::setw(16) << "MT us/sim"
        << std::setw(10) << "Speedup"
        << "\n";

    std::cout << std::string(128, '-') << "\n";

    for (const BenchRow& row : rows) {
        std::ostringstream st_ms;
        st_ms << std::fixed << std::setprecision(2)
              << row.single.min.total_ms << "/"
              << row.single.median.total_ms << "/"
              << row.single.max.total_ms;

        std::ostringstream mt_ms;
        mt_ms << std::fixed << std::setprecision(2)
              << row.multi.min.total_ms << "/"
              << row.multi.median.total_ms << "/"
              << row.multi.max.total_ms;

        const double speedup = row.multi.median.total_ms > 0.0
            ? row.single.median.total_ms / row.multi.median.total_ms
            : 0.0;

        std::cout
            << std::left
            << std::setw(10) << row.n
            << std::setw(22) << st_ms.str()
            << std::setw(16) << static_cast<long long>(row.single.median.sims_per_sec)
            << std::setw(16) << std::fixed << std::setprecision(2) << row.single.median.micros_per_sim
            << std::setw(22) << mt_ms.str()
            << std::setw(16) << static_cast<long long>(row.multi.median.sims_per_sec)
            << std::setw(16) << std::fixed << std::setprecision(2) << row.multi.median.micros_per_sim
            << std::setw(10) << std::fixed << std::setprecision(2) << speedup
            << "\n";
    }

    std::cout << "\n";
}

void writeResults(
    const std::vector<BenchRow>& rows,
    const ss::TeamProfile&       teamA,
    const ss::TeamProfile&       teamB)
{
    std::filesystem::create_directories(resultsPath().parent_path());
    std::ofstream out(resultsPath());

    out << "# SeriesScope Monte Carlo Benchmark\n\n";
    out << "- Timestamp: " << nowTimestamp() << "\n";
    out << "- CPU: " << cpuBrand() << "\n";
    out << "- Logical cores: " << std::max(1u, std::thread::hardware_concurrency()) << "\n";
    out << "- Matchup: " << teamA.name << " vs " << teamB.name << "\n";
    out << "- SQLite fixture DB: `" << benchmarkDbPath().string() << "`\n";
    out << "- Benchmark target: `benchmark_monte_carlo`\n";
    out << "- Timing source: `std::chrono::steady_clock`\n";
    out << "- Trials per N/mode: 3\n\n";

    out << "| N | ST ms min/med/max | ST sims/s med | ST us/sim med | MT ms min/med/max | MT sims/s med | MT us/sim med | Speedup |\n";
    out << "| ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |\n";

    for (const BenchRow& row : rows) {
        const double speedup = row.multi.median.total_ms > 0.0
            ? row.single.median.total_ms / row.multi.median.total_ms
            : 0.0;

        out << "| " << row.n
            << " | " << std::fixed << std::setprecision(2)
            << row.single.min.total_ms << "/" << row.single.median.total_ms << "/" << row.single.max.total_ms
            << " | " << std::setprecision(0) << row.single.median.sims_per_sec
            << " | " << std::setprecision(2) << row.single.median.micros_per_sim
            << " | " << row.multi.min.total_ms << "/" << row.multi.median.total_ms << "/" << row.multi.max.total_ms
            << " | " << std::setprecision(0) << row.multi.median.sims_per_sec
            << " | " << std::setprecision(2) << row.multi.median.micros_per_sim
            << " | " << std::setprecision(2) << speedup
            << " |\n";
    }
}

} // namespace

int main() {
    const auto db_path = benchmarkDbPath();
    seedBenchmarkDb(db_path);
    auto [teamA, teamB] = loadMatchup(db_path);

    ss::ScenarioConfig base_cfg;
    base_cfg.series_length = 7;
    base_cfg.team_a_has_home_court = true;
    base_cfg.team_a_star_avail = 1.0;
    base_cfg.team_b_star_avail = 1.0;
    base_cfg.volatility_multiplier = 1.0;

    const int multi_threads = std::max(1u, std::thread::hardware_concurrency());
    const std::vector<int> workloads {1000, 10000, 100000, 1000000};

    ss::MonteCarloRunner runner;
    std::vector<BenchRow> rows;
    rows.reserve(workloads.size());

    for (int n : workloads) {
        ss::ScenarioConfig single_cfg = base_cfg;
        single_cfg.num_simulations = n;
        single_cfg.thread_count_override = 1;

        ss::ScenarioConfig multi_cfg = base_cfg;
        multi_cfg.num_simulations = n;
        multi_cfg.thread_count_override = multi_threads;

        rows.push_back(BenchRow {
            .n = n,
            .single = benchmarkMode(runner, teamA, teamB, single_cfg),
            .multi = benchmarkMode(runner, teamA, teamB, multi_cfg),
            .single_threads = 1,
            .multi_threads = multi_threads,
        });
    }

    printTable(rows);
    writeResults(rows, teamA, teamB);

    std::cout << "Saved results to " << resultsPath() << "\n";
    return 0;
}
