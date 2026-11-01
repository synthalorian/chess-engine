#pragma once

#include <chrono>
#include <cstdint>

inline double nodes_per_second(std::uint64_t nodes, std::chrono::duration<double> elapsed) {
    const double seconds = elapsed.count();
    return seconds > 0.0 ? static_cast<double>(nodes) / seconds : 0.0;
}
