#include "perft_stats.h"
#include <algorithm>
#include <cmath>
#include <iostream>

int main() {
    using namespace std::chrono;
    struct TestCase {
        std::uint64_t nodes;
        duration<double> elapsed;
        double expected;
    };
    const TestCase cases[] = {
        {1000, seconds(2), 500},
        {1000, milliseconds(250), 4000},
        {20, microseconds(500), 40000},
        {1, nanoseconds(100), 10000000},
        {0, milliseconds(1), 0},
        {1000, seconds(0), 0},
        {1000, seconds(-1), 0},
    };
    for (const auto& test : cases) {
        const double actual = nodes_per_second(test.nodes, test.elapsed);
        if (!std::isfinite(actual) ||
            std::abs(actual - test.expected) > 1e-9 * std::max(1.0, test.expected)) {
            std::cerr << "Expected " << test.expected << " nps, got " << actual << '\n';
            return 1;
        }
    }
    std::cout << "Perft nodes per second PASS\n";
}
