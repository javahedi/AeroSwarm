#include <catch2/catch_test_macros.hpp>
#include "aeroswarm/parallel/simulation.hpp"

#include <atomic>
#include <vector>
#include <thread>
#include <mutex>

TEST_CASE("ParallelTerrain allows a cell to be claimed only once") {
    ParallelTerrain terrain{3,3};

    REQUIRE(terrain.try_claim_cell({1,1}));
    REQUIRE_FALSE(terrain.try_claim_cell({1,1}));
}

TEST_CASE("ParallelTerrain excludes claimed neighbors") {
    ParallelTerrain terrain{3, 3};

    const auto before = terrain.available_neighbors({1, 1});
    REQUIRE(before.size() == 8);

    REQUIRE(terrain.try_claim_cell({0, 1}));

    const auto after = terrain.available_neighbors({1, 1});
    REQUIRE(after.size() == 7);
}


TEST_CASE("ParallelTerrain does not allow obstacles to be claimed") {
    ParallelTerrain terrain{3,3};

    terrain.set_obstacle({1,2});
    REQUIRE_FALSE(terrain.try_claim_cell({1,2}));
}


TEST_CASE("ParallelTerrain excludes obstacles from available neighbors") {
    ParallelTerrain terrain{3, 3};

    terrain.set_obstacle({0, 1});

    const auto neighbors = terrain.available_neighbors({1, 1});

    REQUIRE(neighbors.size() == 7);
}


TEST_CASE("ParallelTerrain identifies target cells") {
    ParallelTerrain terrain{3, 3};

    REQUIRE_FALSE(terrain.is_target({1, 2}));

    terrain.set_target({1, 2});

    REQUIRE(terrain.is_target({1, 2}));
    REQUIRE_FALSE(terrain.is_target({1, 1}));
}


TEST_CASE("ParallelTerrain allows target cell to be claimed") {
    ParallelTerrain terrain{3, 3};

    terrain.set_target({1, 2});

    REQUIRE(terrain.try_claim_cell({1, 2}));
    REQUIRE_FALSE(terrain.try_claim_cell({1, 2}));
    REQUIRE(terrain.is_target({1, 2}));
}


TEST_CASE("ParallelTerrain allows exactly one thread to claim a cell version 0") {
    ParallelTerrain terrain{3, 3};

    std::atomic<int> successful_claims{0};
    std::vector<std::thread> threads;

    auto worker =  [&terrain, & successful_claims]() {
        if (terrain.try_claim_cell({1, 1})) {
                successful_claims.fetch_add(1);
            }
    };

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back(worker);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    REQUIRE(successful_claims.load() == 1);
}




TEST_CASE("ParallelTerrain allows exactly one thread to claim a cell version 1") {
    ParallelTerrain terrain{3, 3};

    std::atomic<int> successful_claims{0};

    std::vector<std::thread> threads;

    /*
    lambas function

    [capture](parameters) {
        // body
    }


    int number = 10;
    auto f = [&number]() {
        number = 20;
    };
    f();

    std::cout << number; // 20
    auto f = [number]() {
        // has its own captured value of number
    };

    [number]   → give lambda a value/copy of number
    [&number]  → give lambda access to original number
    [&]        → capture needed outside variables by reference
    [=]        → capture needed outside variables by value
        
    */
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&]() {
            if (terrain.try_claim_cell({1, 1})) {
                successful_claims.fetch_add(1);
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    REQUIRE(successful_claims.load() == 1);
}

/*
for i in {1..20}; do
    ctest --test-dir build --output-on-failure || break
done
*/



TEST_CASE("ParallelTerrain allows exactly one thread to claim a cell version 2") {
    ParallelTerrain terrain{3, 3};

    int successful_claims{0};
    std::mutex counter_mutex;

    std::vector<std::thread> threads;

    /*
    &terrain, &successful_claims, &counter_mutex

    Thread 0's worker ─────┐
    Thread 1's worker ─────┤
    Thread 2's worker ─────┤
    Thread 3's worker ─────┼──→ SAME terrain
    ...                    │
    Thread 9's worker ─────┘

    all workers ──→ SAME successful_claims
    all workers ──→ SAME counter_mutex



    */

   auto worker =  [&terrain, &successful_claims, &counter_mutex]() {
        if (terrain.try_claim_cell({1, 1})) {
                std::lock_guard<std::mutex> lock(counter_mutex);
                ++successful_claims;
            }
    };

    /*
    // the loop does NOT mean ordered execution
    
    created:   0 1 2 3 4 5 6 7 8 9

    execution:
                7
            2
                        9
    0
                6
    ...
    
    
    */
    for (int i = 0; i < 10; ++i) {
        /*
        std::thread t(worker);
        threads.push_back(std::move(t));
        emplace_back avoids needing that temporary t.
        */
        threads.emplace_back(worker);


        /*
        assume if Thread 4 → true
        other 9  → false

        successful_claims == 1
        */
    }

    for (auto& thread : threads) {
        thread.join();
    }

    REQUIRE(successful_claims == 1);
}




TEST_CASE("ParallelTerrain allows exactly one thread to claim a cell version 3") {
    ParallelTerrain terrain{3, 3};

    std::vector<int> results(10,0);
    
    std::vector<std::thread> threads;

    // for (int i = 0; i < 10; ++i) {
    //     threads.emplace_back([&, i]() {
    //             results[i] = terrain.try_claim_cell({1,1}) ? 1 : 0;
            
    //     });
    // }

    for (int i = 0; i < 10; ++i) {

        auto worker = [&terrain, &results, i]() {
            const bool claimed =
                terrain.try_claim_cell({1, 1});

            results[i] = claimed ? 1 : 0;
        };

        threads.emplace_back(worker);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    int successful_claims = 0;

    for (int result : results) {
        successful_claims += result;
    }

    REQUIRE(successful_claims == 1);
}



TEST_CASE("ParallelTerrain allows different cells to be claimed concurrently") {
    ParallelTerrain terrain{5, 5};

    const std::vector<Position> positions{
        {0, 0},
        {1, 0},
        {2, 0},
        {3, 0},
        {4, 0},
        {0, 1},
        {1, 1},
        {2, 1},
        {3, 1},
        {4, 1}
    };

    std::vector<int> results(positions.size(), 0);
    std::vector<std::thread> threads;

    for (std::size_t i = 0; i < positions.size(); ++i) {
        threads.emplace_back([&terrain, &positions, &results, i]() {
            results[i] = 
                        terrain.try_claim_cell(positions[i]) ? 1 : 0;
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    int successful_claims = 0;

    for (int result : results) {
        successful_claims += result;
    }

    REQUIRE(successful_claims == static_cast<int>(positions.size()));
}




TEST_CASE("ParallelTerrain initializes a valid drone start position") {
    ParallelTerrain terrain{3, 3};

    REQUIRE(terrain.initialize_start_position({1, 1}));

    // Same cell is already considered visited,
    // so it should no longer be claimable as a movement target.
    REQUIRE_FALSE(terrain.try_claim_cell({1, 1}));
}


TEST_CASE("ParallelTerrain allows multiple drones to share the same start position") {
    ParallelTerrain terrain{3, 3};

    REQUIRE(terrain.initialize_start_position({1, 1}));
    REQUIRE(terrain.initialize_start_position({1, 1}));
}

TEST_CASE("ParallelTerrain rejects invalid drone start positions") {
    ParallelTerrain terrain{3, 3};

    terrain.set_obstacle({1, 1});

    REQUIRE_FALSE(terrain.initialize_start_position({1, 1}));
    REQUIRE_FALSE(terrain.initialize_start_position({5, 5}));
}



TEST_CASE("ParallelTerrain reports information gain") {
    ParallelTerrain terrain{3, 3};

    REQUIRE(terrain.information_gain({1, 1}) == 8);
}

TEST_CASE("ParallelTerrain information gain excludes obstacles") {
    ParallelTerrain terrain{3, 3};

    terrain.set_obstacle({0, 1});
    terrain.set_obstacle({2, 1});

    REQUIRE(terrain.information_gain({1, 1}) == 6);
}


TEST_CASE("ParallelTerrain corner has three available neighbors") {
    ParallelTerrain terrain{3, 3};

    const auto neighbors =
        terrain.available_neighbors({0, 0});

    REQUIRE(neighbors.size() == 3);
}