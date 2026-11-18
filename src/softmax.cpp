#include "softmax.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace softmax_demo {
namespace {
void validate(const Values& input) {
    if (input.empty()) {
        throw std::invalid_argument("softmax input cannot be empty");
    }
    for (float value : input) {
        if (!std::isfinite(value)) {
            throw std::invalid_argument("softmax requires finite inputs");
        }
    }
}
}  // namespace

Values naive(const Values& input) {
    validate(input);
    double denom = 0.0;
    for (float value : input) denom += std::exp(static_cast<double>(value));
    Values result(input.size());
    for (std::size_t i = 0; i < input.size(); ++i) {
        result[i] = static_cast<float>(std::exp(static_cast<double>(input[i])) / denom);
    }
    return result;
}

Values stable(const Values& input) {
    validate(input);
    const float max_value = *std::max_element(input.begin(), input.end());
    double denom = 0.0;
    for (float value : input) {
        denom += std::exp(static_cast<double>(value) - max_value);
    }
    Values result(input.size());
    for (std::size_t i = 0; i < input.size(); ++i) {
        result[i] = static_cast<float>(
            std::exp(static_cast<double>(input[i]) - max_value) / denom);
    }
    return result;
}

Values stable_cached(const Values& input) {
    validate(input);
    const float max_value = *std::max_element(input.begin(), input.end());
    Values result(input.size());
    double denom = 0.0;
    for (std::size_t i = 0; i < input.size(); ++i) {
        result[i] = static_cast<float>(
            std::exp(static_cast<double>(input[i]) - max_value));
        denom += result[i];
    }
    for (float& value : result) {
        value = static_cast<float>(static_cast<double>(value) / denom);
    }
    return result;
}

Values online(const Values& input) {
    validate(input);
    double max_value = -std::numeric_limits<double>::infinity();
    double denom = 0.0;
    for (float value : input) {
        const double x = static_cast<double>(value);
        if (x > max_value) {
            // Rescale the previous normalizer when the maximum changes.
            denom = denom * std::exp(max_value - x) + 1.0;
            max_value = x;
        } else {
            denom += std::exp(x - max_value);
        }
    }
    Values result(input.size());
    for (std::size_t i = 0; i < input.size(); ++i) {
        result[i] = static_cast<float>(
            std::exp(static_cast<double>(input[i]) - max_value) / denom);
    }
    return result;
}
}  // namespace softmax_demo
