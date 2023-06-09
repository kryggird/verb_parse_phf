#include <cstdlib>
#include <vector>
#include <string_view>
#include <random>

#include "time_utils.h"

#include "http_verb.h"
#include "verb-parse-orig.cpp"
#include "verb-parse-swar.cpp"
#include "verb-parse-swar32.cpp"
#include "verb-parse-perfhash.cpp"
#include "verb-parse-avx2.cpp"
#include "verb-parse-pext.cpp"

namespace reference_impl {
    using namespace boost::beast::http;
}

class Benchmark {
 
    const size_t size;
    const size_t iterations;
    std::vector<std::string> input;

    std::random_device random_device;
    std::mt19937 random_engine;

    std::uniform_int_distribution<> random_verb_idx;
    std::uniform_int_distribution<> random_common_verb_idx;


public:
    Benchmark(size_t size, size_t iterations)
        : size{size}
        , iterations{iterations}
        , random_device{}
        , random_engine{random_device()}
        , random_verb_idx{0, boost::beast::http::verb_count - 1}
        , random_common_verb_idx(0, 2)
    {
    }

    void run_all() {
        test_all_verbs();
        puts("");
        test_common_verbs();
    }

    void test_all_verbs() {
        printf("Random order (size = %lu, iterations = %lu)\n", size, iterations);
        prepare_random_input([this](){return random_verb();});
        measure_all();
    }

    void test_common_verbs() {
        printf("Random GET/POST/PUT (size = %lu, iterations = %lu)\n", size, iterations);
        prepare_random_input([this](){return random_common_verb();});
        measure_all();
    }

private:
    void measure_all()
    {
        measure_total_time("boost::beast   : ", [this](){return test_boost_beast();}, iterations);
        measure_total_time("SWAR  (64 bit) : ", [this](){return test_swar64();}, iterations);
        measure_total_time("SWAR  (32 bit) : ", [this](){return test_swar32();}, iterations);
        measure_total_time("SWAR2 (32 bit) : ", [this](){return test_swar32_v2();}, iterations);
        measure_total_time("perfect hash   : ", [this](){return test_perfect_hash();}, iterations);
        //measure_total_time("avx2           : ", [this](){return test_avx2();}, iterations);
        measure_total_time("pext           : ", [this](){return test_pext();}, iterations);
    }

private:
    uint32_t test_boost_beast() {
        uint32_t res = 0;
        for (const std::string& s: input)
            res += static_cast<uint32_t>(boost::beast::http::string_to_verb(s));

        return res;
    }

    uint32_t test_swar64() {
        uint32_t res = 0;
        for (const std::string& s: input) {
            res += static_cast<uint32_t>(swar::string_to_verb(s));
        }

        return res;
    }

    uint32_t test_swar32() {
        uint32_t res = 0;
        for (const std::string& s: input) {
            res += static_cast<uint32_t>(swar32::string_to_verb(s));

            // if (swar32::string_to_verb(s) != reference_impl::string_to_verb(s)) {
            //     auto swar32_s = as_string(swar32::string_to_verb(s));
            //     printf("swar32: %s vs orig: %s\n", (char*) swar32_s.data(), s.c_str());
            //     exit(1);
            // }
        }

        return res;
    }

    uint32_t test_swar32_v2() {
        uint32_t res = 0;
        for (const std::string& s: input)
            res += static_cast<uint32_t>(swar32::string_to_verb_v2(s));

        return res;
    }

    uint32_t test_perfect_hash() {
        uint32_t res = 0;
        for (const std::string& s: input) {
            res += static_cast<uint32_t>(perfect_hash::string_to_verb(s));

            // if (perfect_hash::string_to_verb(s) != reference_impl::string_to_verb(s)) {
            //     auto phf_s = as_string(perfect_hash::string_to_verb(s));
            //     printf("phf: %s vs orig: %s\n", (char*) phf_s.data(), s.c_str());
            //     exit(1);
            // }
        }

        return res;
    }
    
    uint32_t test_avx2() {
        uint32_t res = 0;
        for (const std::string& s: input)
            res += static_cast<uint32_t>(avx2::string_to_verb(s));

        return res;
    }
    
    uint32_t test_pext() {
        uint32_t res = 0;
        for (const std::string& s: input) {
            res += static_cast<uint32_t>(pext::string_to_verb(s));

            // if (pext::string_to_verb(s) != reference_impl::string_to_verb(s)) {
            //     auto pext_s = as_string(pext::string_to_verb(s));
            //     printf("pext: %d vs orig: %d\n", (int) pext::string_to_verb(s), swar32::string_to_verb(s));
            //     printf("pext: %s vs orig: %s\n", (char*) pext_s.data(), s.c_str());
            //     exit(1);
            // }
        }

        return res;
    }

private:
    template <typename RANDOM_VERB>
    void prepare_random_input(RANDOM_VERB random)
    {
        random_engine.seed(0);

        input.clear();
        input.reserve(size);
        for (size_t i=0; i < size; i++)
            input.emplace_back(as_string(random()));
    }

    boost::beast::http::verb random_verb() {
        return static_cast<boost::beast::http::verb>(random_verb_idx(random_engine));
    }

    boost::beast::http::verb random_common_verb() {
        switch (random_common_verb_idx(random_engine)) {
            case 0:
                return boost::beast::http::verb::get;
            case 1:
                return boost::beast::http::verb::post;
            case 2:
                return boost::beast::http::verb::put;

            default:
                abort();
        }
    }

};


int main(int argc, char* argv[]) {
    int tmp;

    tmp = 0;
    if (argc > 1) {
        tmp = atoi(argv[1]);
    }

    const size_t size = (tmp > 0) ? tmp : 1024;

    tmp = 0;
    if (argc > 2) {
        tmp = atoi(argv[2]);
    }

    const int iterations = (tmp > 0) ? tmp : 10;

    Benchmark b(size, iterations);
    b.run_all();

    return EXIT_SUCCESS;
}
