#pragma once

#include <string_view>

#include <x86intrin.h>

#define LIKELY(x)       __builtin_expect((x),1)

namespace avx_load {
    static __m128i MASKS[4] = {
        _mm_setr_epi32(0,  0,  0,  0),
        _mm_setr_epi32(0,  0,  0, -1),
        _mm_setr_epi32(0,  0, -1, -1),
        _mm_setr_epi32(0, -1, -1, -1),
    };

    inline bool cmp_tail(std::string_view lhs, std::string_view rhs) {
        int idx = lhs.size() >> 2ul;
        __m128i mask = MASKS[idx];
        const char* lhs_ptr = (lhs.data() + lhs.size()) - sizeof(__m128i);
        const char* rhs_ptr = (rhs.data() + lhs.size()) - sizeof(__m128i);

        return (lhs.size() == rhs.size()) && _mm_testc_si128(
            _mm_maskload_epi32((int*) lhs_ptr, mask),
            _mm_maskload_epi32((int*) rhs_ptr, mask)
        );
    }

    constexpr uint64_t VEC_SZ = 16;
    constexpr uint64_t PAGE_SZ = 4096;
    constexpr uint64_t MASK = (PAGE_SZ - 1ul);

    int avx2_cmp_v2(std::string_view lhs, std::string_view rhs) {
        uint64_t sz_mask = -1ul << lhs.size();
        uint64_t as_u64 = ((uint64_t) lhs.data() | (uint64_t) rhs.data()) & MASK;
        int is_page_aligned = as_u64 < (PAGE_SZ - VEC_SZ);
        int is_same_size = lhs.size() == rhs.size();
        
        uint64_t sz_mask_suffix = -1ul >> lhs.size();
        uint64_t as_u64_suffix = (uint64_t) lhs.data() & (uint64_t) rhs.data() & MASK;
        int is_page_aligned_suffix = as_u64_suffix > PAGE_SZ;

        if (LIKELY(is_page_aligned)) {
            auto lhs_xmm = _mm_lddqu_si128((__m128i*) lhs.data());
            auto rhs_xmm = _mm_lddqu_si128((__m128i*) rhs.data());
            auto eq_xmm = _mm_cmpeq_epi8(lhs_xmm, rhs_xmm);
            uint64_t eq_mask = _mm_movemask_epi8(eq_xmm);

            return (eq_mask | sz_mask) + is_same_size;
        } else if (is_page_aligned_suffix) {
            auto lhs_xmm = _mm_lddqu_si128((__m128i*) lhs.data() + lhs.size() - VEC_SZ);
            auto rhs_xmm = _mm_lddqu_si128((__m128i*) rhs.data() + rhs.size() - VEC_SZ);
            auto eq_xmm = _mm_cmpeq_epi8(lhs_xmm, rhs_xmm);
            uint64_t eq_mask = _mm_movemask_epi8(eq_xmm);

            return (eq_mask | sz_mask_suffix) + is_same_size;
        } else {
            return lhs != rhs;
        }
    }
}

/*
#include <cstdio>

int main() {
    using namespace std;
    using namespace std::string_view_literals;
    using namespace avx_load;

    auto lhs = "+A.B.C.D"sv;
    auto rhs = "*A.B.C.D"sv;

    printf("%s\n", cmp_tail(lhs, rhs) ? "Yes!" : "No!");
}
*/
