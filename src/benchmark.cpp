#include "softmax.hpp"

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
volatile double sink = 0.0;
using Values = softmax_demo::Values;
using Function = Values (*)(const Values&);

std::vector<std::size_t> parse_sizes(const std::string& csv) {
    std::vector<std::size_t> sizes;
    std::size_t begin = 0;
    while (begin < csv.size()) {
        auto end = csv.find(',', begin);
        const std::string part = csv.substr(begin, end == std::string::npos ? end : end - begin);
        std::size_t count = std::stoull(part);
        if (count == 0 || count > 50000000) throw std::invalid_argument("invalid input size");
        sizes.push_back(count);
        if (end == std::string::npos) break;
        begin = end + 1;
    }
    if (sizes.empty()) throw std::invalid_argument("no sizes");
    return sizes;
}
}  // namespace

int main(int argc, char** argv) {
    try {
        std::string size_arg = "256,4096,65536,1048576";
        std::string output_path;
        int repeats = 7;
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if ((arg == "--sizes" || arg == "--repeats" || arg == "--output") && i+1 < argc) {
                if (arg == "--sizes") size_arg = argv[++i];
                else if (arg == "--repeats") repeats = std::stoi(argv[++i]);
                else output_path = argv[++i];
            } else {
                throw std::invalid_argument("Usage: softmax_bench [--sizes 256,4096] [--repeats 7] [--output FILE]");
            }
        }
        if (repeats < 1 || repeats > 200) throw std::invalid_argument("repeats must be 1..200");
        std::ofstream file;
        if (!output_path.empty()) {
            file.open(output_path);
            if (!file) throw std::runtime_error("cannot open output CSV: " + output_path);
        }
        std::ostream& stream = output_path.empty() ? std::cout : file;
        stream << "variant,size,repeat,iterations,ns_per_call,ns_per_element,checksum\n";
        const std::vector<std::pair<std::string, Function>> funcs = {
            {"naive", softmax_demo::naive},
            {"stable", softmax_demo::stable},
            {"stable_cached", softmax_demo::stable_cached},
            {"online", softmax_demo::online}
        };
        for (std::size_t size : parse_sizes(size_arg)) {
            std::mt19937 generator(static_cast<std::mt19937::result_type>(2026 + size));
            std::uniform_real_distribution<float> distribution(-8.0f, 8.0f);
            Values input(size);
            for (float& v : input) v = distribution(generator);
            const int iterations = size < 1024 ? 3000 : size < 10000 ? 300 : size < 100000 ? 30 : 4;
            for (const auto& [name, fn] : funcs) {
                for (int warmup = 0; warmup < 3; ++warmup) {
                    Values result = fn(input);
                    sink = sink + result[warmup % size];
                }
                for (int repeat = 0; repeat < repeats; ++repeat) {
                    auto begin = std::chrono::steady_clock::now();
                    double checksum = 0.0;
                    for (int iteration = 0; iteration < iterations; ++iteration) {
                        Values result = fn(input);
                        checksum += result[static_cast<std::size_t>(iteration) % size];
                    }
                    auto end = std::chrono::steady_clock::now();
                    sink = sink + checksum;
                    const double elapsed_ns = std::chrono::duration<double, std::nano>(end - begin).count();
                    const double ns_per_call = elapsed_ns / iterations;
                    stream << name << ',' << size << ',' << repeat << ',' << iterations << ','
                           << std::fixed << std::setprecision(3) << ns_per_call << ','
                           << ns_per_call / size << ',' << std::setprecision(9) << checksum << '\n';
                }
            }
        }
        if (!output_path.empty()) {
            std::cerr << "Wrote benchmark CSV: " << output_path << '\n';
        }
        if (!std::isfinite(sink)) throw std::runtime_error("invalid benchmark sink");
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << '\n';
        return 1;
    }
}
