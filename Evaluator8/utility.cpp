#include "utility.h"
#include "constants.h"
#include <cmath>
#include <iostream>
#include <limits>

bool is_close(const float& x, const float& y)
{
    if (std::fabs(x - y) < std::numeric_limits<float>::epsilon()) {
        return true;
    }
    return false;
}

bool is_close(const double& x, const double& y)
{
    if (std::abs(x - y) < std::numeric_limits<double>::epsilon()) {
        return true;
    }
    return false;
}

bool is_close(const float& x, const float& y, const float& eps)
{
    if (std::fabs(x - y) < eps) {
        return true;
    }
    return false;
}