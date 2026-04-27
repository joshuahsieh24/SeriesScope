add_library(SeriesScopeLib STATIC
    data/DatabaseManager.cpp
    data/TeamRepository.cpp
    data/PlayerRepository.cpp
    engine/GameProbabilityModel.cpp
    engine/SeriesSimulator.cpp
    engine/MonteCarloRunner.cpp
    engine/AwardEngine.cpp
)

target_include_directories(SeriesScopeLib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(SeriesScopeLib
    PUBLIC
        SQLite3::SQLite3
        Threads::Threads
)
