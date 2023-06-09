#include "nanobench.h"

#include <array>
#include <cassert>
#include <cstdlib>
#include <random>
#include <string_view>
#include <vector>

#include "http_verb.h"
#include "verb-parse-orig.cpp"
#include "verb-parse-swar.cpp"
#include "verb-parse-swar32.cpp"
#include "verb-parse-perfhash.cpp"
#include "verb-parse-avx2.cpp"
#include "verb-parse-pext.cpp"

#include "verb-parse-pext-split.cpp"

#include <iostream>

namespace reference_impl {
    using namespace boost::beast::http;
}

template <size_t SZ>
auto prepare_random_input(const std::array<std::string_view, SZ>& words, uint64_t output_size)
{
    using namespace std;
    random_device random_device {};
    mt19937 random_engine {random_device()};
    random_engine.seed(42);

    uniform_int_distribution<> random_idx {0, (int) words.size() - 1};

    vector<string_view> res {};
    res.reserve(output_size);
    for (size_t i=0; i < output_size; i++) {
        auto w = words[random_idx(random_engine)];
        res.emplace_back(w);
    }

    return res;
}


template <typename F>
void run_benchmark(const char* title, const char* name, const std::vector<std::string_view>& words, F func) {
    using namespace ankerl::nanobench;

    uint64_t counter = 0ul;
    Bench().title(title).minEpochIterations(256ul * 1024ul)
           .run(name, [&words, &counter, func]() {
        auto w = words[counter++ % words.size()];
        auto res = func(w);

        doNotOptimizeAway(w);
        doNotOptimizeAway(res);
    });
}


int main(int argc, char* argv[]) {
    uint64_t verbs_size = (argc == 1) ? (10'000ul) : atoi(argv[1]);
    using namespace verb_constants;

    auto all_verbs = prepare_random_input(ALL_VERBS, verbs_size);
    auto common_verbs = prepare_random_input(COMMON_VERBS, verbs_size);
    
    run_benchmark("all verbs", "noop", common_verbs, [](auto w) { return reference_impl::verb::unknown; });
    run_benchmark("all verbs", "reference_impl", all_verbs, reference_impl::string_to_verb);
    run_benchmark("all verbs", "swar", all_verbs, swar::string_to_verb);
    run_benchmark("all verbs", "swar32", all_verbs, swar32::string_to_verb);
    run_benchmark("all verbs", "swar32 v2", all_verbs, swar32::string_to_verb_v2);
    run_benchmark("all verbs", "perfect_hash", all_verbs, perfect_hash::string_to_verb);
    //run_benchmark("all verbs", "avx2", all_verbs, avx2::string_to_verb);
    run_benchmark("all verbs", "pext_by_len", all_verbs, pext_split::string_to_verb);
    run_benchmark("all verbs", "pext", all_verbs, pext::string_to_verb);
    run_benchmark("all verbs", "pext v2", all_verbs, pext::string_to_verb_v2);
    run_benchmark("all verbs", "pext v3", all_verbs, pext::string_to_verb_v3);
    run_benchmark("all verbs", "pext v4", all_verbs, pext::string_to_verb_v4);
    run_benchmark("all verbs", "pext v5", all_verbs, pext::string_to_verb_v5);
    run_benchmark("all verbs", "pext v6", all_verbs, pext::string_to_verb_v6);
    
    run_benchmark("GET/PUT/POST", "noop", common_verbs, [](auto w) { return reference_impl::verb::unknown; });
    run_benchmark("GET/PUT/POST", "reference_impl", common_verbs, reference_impl::string_to_verb);
    run_benchmark("GET/PUT/POST", "swar", common_verbs, swar::string_to_verb);
    run_benchmark("GET/PUT/POST", "swar32", common_verbs, swar32::string_to_verb);
    run_benchmark("GET/PUT/POST", "swar32 v2", common_verbs, swar32::string_to_verb_v2);
    run_benchmark("GET/PUT/POST", "perfect_hash", common_verbs, perfect_hash::string_to_verb);
    run_benchmark("GET/PUT/POST", "pext_by_len", common_verbs, pext_split::string_to_verb);
    run_benchmark("GET/PUT/POST", "avx2", common_verbs, avx2::string_to_verb);
    run_benchmark("GET/PUT/POST", "pext", common_verbs, pext::string_to_verb);
    run_benchmark("GET/PUT/POST", "pext v2", common_verbs, pext::string_to_verb_v2);
    run_benchmark("GET/PUT/POST", "pext v3", common_verbs, pext::string_to_verb_v3);
    run_benchmark("GET/PUT/POST", "pext v4", common_verbs, pext::string_to_verb_v4);
    run_benchmark("GET/PUT/POST", "pext v5", common_verbs, pext::string_to_verb_v5);
    run_benchmark("GET/PUT/POST", "pext v6", common_verbs, pext::string_to_verb_v6);

    return EXIT_SUCCESS;
}
