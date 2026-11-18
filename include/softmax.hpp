#pragma once
#include <vector>

namespace softmax_demo {
// All functions require a non-empty input with finite float values.
// They throw std::invalid_argument otherwise.
using Values = std::vector<float>;

// Simple textbook implementation. Not numerically safe for large inputs.
Values naive(const Values& input);

// Three logical passes: find max, compute normalizer, compute outputs.
Values stable(const Values& input);

// Cache exp(x-max) in the output array and normalize in-place.
Values stable_cached(const Values& input);

// Online normalizer (Milakov & Gimelshein, 2018), then emit outputs.
Values online(const Values& input);
}  // namespace softmax_demo
