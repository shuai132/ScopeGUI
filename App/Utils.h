#pragma once

#include <cstdint>

namespace utils {

template <typename T>
inline void setLimit(T& v, T min, T max) {
    if (v < min) {
        v = min;
    } else if (v > max) {
        v = max;
    }
}

uint32_t nextPow2(uint32_t v);

#define FOR(i, n)   for(std::remove_reference<std::remove_const<typeof(n)>::type>::type i = 0; i < n; i++)

}
