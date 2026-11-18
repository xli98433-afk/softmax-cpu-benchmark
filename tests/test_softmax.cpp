#include "softmax.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

using softmax_demo::Values;
using Function = Values (*)(const Values&);

void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

void verify(const std::string& name, Function function,
            const Values& input, const Values& reference, float tolerance = 2e-6f) {
    Values output = function(input);
    require(output.size() == input.size(), name + ": incorrect result size");
    double sum = 0.0;
    for (std::size_t i = 0; i < output.size(); ++i) {
        require(std::isfinite(output[i]), name + ": non-finite output");
        require(output[i] >= 0.0f && output[i] <= 1.0f, name + ": invalid probability");
        require(std::fabs(output[i] - reference[i]) <= tolerance,
                name + ": reference mismatch at index " + std::to_string(i));
        sum += output[i];
    }
    require(std::fabs(sum - 1.0) < 2e-5, name + ": probabilities do not sum to one");
}

void test_errors(const std::string& name, Function function) {
    const Values bad_inputs[] = {{}, {1.0f, std::numeric_limits<float>::infinity()},
                                  {std::numeric_limits<float>::quiet_NaN()}};
    for (const Values& input : bad_inputs) {
        bool rejected = false;
        try { (void)function(input); }
        catch (const std::invalid_argument&) { rejected = true; }
        require(rejected, name + ": expected invalid_argument");
    }
}

int main() {
    try {
        const std::vector<std::pair<std::string, Function>> funcs = {
            {"naive", softmax_demo::naive},
            {"stable", softmax_demo::stable},
            {"stable_cached", softmax_demo::stable_cached},
            {"online", softmax_demo::online}
        };
        std::mt19937 generator(2026);
        std::uniform_real_distribution<float> distribution(-10.0f, 10.0f);
        Values random_input(8192);
        for (float& v : random_input) v = distribution(generator);
        const Values random_ref = softmax_demo::stable(random_input);

        int cases = 0;
        for (const auto& [name, fn] : funcs) {
            verify(name, fn, {0.0f, 0.0f}, {0.5f, 0.5f}); ++cases;
            verify(name, fn, {1.0f, 2.0f, 3.0f},
                   {0.09003057f, 0.24472847f, 0.66524096f}); ++cases;
            verify(name, fn, {42.0f}, {1.0f}); ++cases;
            verify(name, fn, random_input, random_ref); ++cases;
            test_errors(name, fn); ++cases;
        }
        // The textbook formula can overflow. Stable variants must remain finite.
        const Values huge = {1000.0f, 1001.0f, 1002.0f};
        const Values expected = {0.09003057f, 0.24472847f, 0.66524096f};
        for (const auto& [name, fn] : funcs) {
            if (name != "naive") {
                verify(name, fn, huge, expected); ++cases;
                verify(name, fn, {-1000.0f, -1001.0f},
                       {0.7310586f, 0.2689414f}); ++cases;
            }
        }
        const Values overflowed = softmax_demo::naive(huge);
        require(!std::isfinite(overflowed[0]), "naive should illustrate overflow");
        ++cases;
        std::cout << "PASS: " << cases << " checks (including edge cases and numerical stability)\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FAIL: " << e.what() << '\n';
        return 1;
    }
}
