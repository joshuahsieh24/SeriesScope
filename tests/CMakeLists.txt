enable_testing()
FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/03597a01eeebcda3a35d79383637651a0076a445.zip)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
add_executable(series_scope_tests
    test_engine.cpp
    test_award_engine.cpp
    test_player_repository.cpp
)
target_link_libraries(series_scope_tests GTest::gtest_main SeriesScopeLib)
include(GoogleTest)
gtest_discover_tests(series_scope_tests)
