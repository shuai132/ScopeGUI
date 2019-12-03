#pragma once

namespace utils {

template <typename T>
static void setLimit(T& v, T min, T max) {
    if (v < min) {
        v = min;
    } else if (v > max) {
        v = max;
    }
}

}
