#include "http_verb.h"
#include "avx_load.hpp"

#include <array>
#include <algorithm>
#include <cstring>

#include <x86intrin.h>

// #define LIKELY(x)       __builtin_expect(!!(x), 1)
#define UNLIKELY(x)       __builtin_expect(!!(x), 0)
#define UNPREDICTABLE(x)       __builtin_expect_with_probability(!!(x), 0, 0.5)

namespace pext {
    using ::boost::beast::http::verb;

    std::string_view
    operator"" _sv(char const* p, std::size_t n)
    {
        return std::string_view{p, n};
    }

    static const uint32_t MASK = 33687137u;
    uint32_t lut_idx (std::string_view sv) { 
        uint32_t val = 0u;
        memcpy(&val, sv.data(), std::min(sv.size(), sizeof(val)));
        return _pext_u32(val, MASK) ^ sv.size();
    };


    using lut_elem_t = uint8_t; //verb;
    static const std::array<lut_elem_t, 128> LOOKUP_TABLE = []() {
        using namespace boost::beast::http;
        std::array<lut_elem_t, 128> lookup_table {(lut_elem_t) verb::unknown};

        for (unsigned int as_int = 0; as_int < verb_count; ++as_int) {
            std::string_view sv = as_string((verb) as_int);
            lookup_table[lut_idx(sv)] = (lut_elem_t) as_int;
        }

        return lookup_table;
    }();

    verb
    string_to_verb(std::string_view v)
    {
        if (v.size() < 3 or v.size() > 13) {
            return verb::unknown;
        }

        verb res = (verb) LOOKUP_TABLE[lut_idx(v)];

        if (v == as_string(res)) {
            return res;
        }

        return verb::unknown;
    }
    
    uint32_t load_sv(std::string_view sv) {
        union {
            char as_char[4];
            uint32_t res;
        };
        as_char[0] = sv[0];
        as_char[1] = sv[1];
        as_char[2] = sv[2];
        as_char[3] = (sv.size() > 3ul) ? sv[3] : 0;
        return res;
    }
    
    uint32_t lut_idx_v2(std::string_view sv) {
        return _pext_u32(load_sv(sv), MASK) ^ ((uint32_t) sv.size());
    }

    verb
    string_to_verb_v2(std::string_view v)
    {
        if (v.size() < 3 or v.size() > 13) {
            return verb::unknown;
        }

        verb res = (verb) LOOKUP_TABLE[lut_idx_v2(v)];

        if (v != as_string(res)) {
            return verb::unknown;
        } else {
            return res;
        }
    }
    
    static const std::array<lut_elem_t, 128> LOOKUP_TABLE_UNSAFE = []() {
        using namespace boost::beast::http;
        std::array<lut_elem_t, 128> lookup_table {(lut_elem_t) verb::unknown};

        for (unsigned int as_int = 0; as_int < verb_count; ++as_int) {
            std::string_view sv = as_string((verb) as_int);
            lookup_table[lut_idx(sv)] = (lut_elem_t) as_int;
            if (sv.size() == 3) {
                lookup_table[lut_idx(sv) | (1 << 6)] = (lut_elem_t) as_int;
            }
        }

        return lookup_table;
    }();

    uint32_t idx_from_data_and_size(uint32_t as_int, uint32_t size) {
        return _pext_u32(as_int, MASK) ^ ((uint32_t) size);
    }
    
    uint32_t lut_idx_v3(std::string_view sv) {
        if (UNLIKELY((uint64_t(sv.data()) & (4096ul - 1ul)) == (4096ul - 3ul))) {
            return lut_idx_v2(sv);
        }
        uint32_t as_int;
        std::memcpy(&as_int, sv.data(), 4ul);
        //return _pext_u32(as_int, MASK) ^ ((uint32_t) sv.size());
        return idx_from_data_and_size(as_int, (uint32_t) sv.size());
    }

    verb
    string_to_verb_v3(std::string_view v)
    {
        if (v.size() < 3 or v.size() > 13) {
            return verb::unknown;
        }

        verb res = (verb) LOOKUP_TABLE_UNSAFE[lut_idx_v3(v)];

        if (v != as_string(res)) {
            return verb::unknown;
        } else {
            return res;
        }
    }

    union sv_slot_t {
        uint8_t as_chars[16];
        __m128i as_xmm;
        uint8_t size;
    };

    static const std::array<sv_slot_t, 128> LOOKUP_TABLE_STR = []() {
        std::array<sv_slot_t, 128> res {}; // Default initialize to zero
        for (int i = 0; i < 128; ++i) {
            verb v = (verb) LOOKUP_TABLE[i];
           
            if (v != verb::unknown) {
                auto sv = as_string(v);

                uint8_t* as_arr = res[i].as_chars;
                as_arr[0] = (uint8_t) sv.size();
                for (size_t j = 0; j < sv.size(); ++j) {
                    as_arr[j+1] = sv[j];
                }
            }
        }
        return res;
    }();

    alignas(32) static const uint8_t MASK_BUFFER[32] {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
         0,  0,  0,  0,  0,  0,  0,  0,  
         0,  0,  0,  0,  0,  0,  0,  0
    };

    size_t load_size_from_table(int idx) { return LOOKUP_TABLE_STR[idx].size; }
    __m128i load_str_from_table(int idx) {
        return _mm_srli_si128(LOOKUP_TABLE_STR[idx].as_xmm, 1);
    }
    __m128i load_str_from_buffer(std::string_view sv) {
        if (UNLIKELY((uint64_t(sv.data()) & (4096ul - 1ul)) >= (4096ul - 16ul))) {
            union {
                uint8_t as_buf[16];
                __m128i as_xmm;
            };

            as_xmm = _mm_set1_epi8(0);
            memcpy(as_buf, sv.data(), sv.size());
            return as_xmm;
        }
        
        __m128i mask = _mm_lddqu_si128((__m128i*) (MASK_BUFFER - sv.size()));
        __m128i data = _mm_lddqu_si128((__m128i*) sv.data());
        return _mm_and_si128(mask, data);
    }
    
    verb
    string_to_verb_v4(std::string_view v)
    {
        if (v.size() < 3 or v.size() > 13) {
            return verb::unknown;
        }

        int idx = lut_idx_v3(v);
        verb res = (verb) LOOKUP_TABLE[idx];
        size_t size = load_size_from_table(idx);
        __m128i lhs = load_str_from_table(idx);
        __m128i rhs = load_str_from_buffer(v);

        if (LIKELY(size == v.size()) && LIKELY(_mm_testz_si128(lhs, rhs))) {
            return res;
        } else {
            return verb::unknown;
        }
    }

    struct sv_slot_scalar_t {
        uint32_t head;
        uint64_t tail;
        uint8_t size;
        uint8_t padding[3];
    };

    static const std::array<sv_slot_scalar_t, 128> LOOKUP_TABLE_STR_SCALAR = []() {
        std::array<sv_slot_scalar_t, 128> res {}; // Default initialize to zero
        for (int i = 0; i < 128; ++i) {
            verb v = (verb) LOOKUP_TABLE[i];
           
            if (v != verb::unknown) {
                auto sv = as_string(v);

                sv_slot_scalar_t slot;

                slot.size = (uint8_t) sv.size();
                slot.head = 0u;
                slot.tail = 0ul;

                std::memcpy(&slot.head, sv.data(), std::min(sv.size(), 4ul));
                if (sv.size() > 4ul) {
                    size_t to_copy = sv.size() - 4ul;
                    std::memcpy(&slot.tail, sv.data() + 4ul, std::min(to_copy, 8ul));
                }

                slot.padding[0] = 0;
                slot.padding[1] = 0;
                slot.padding[2] = 0;

                res[i] = slot;
            }
        }
        return res;
    }();

    uint32_t load_head_from_buffer(std::string_view sv) {
        uint32_t as_int;
        if (UNLIKELY((uint64_t(sv.data()) & (4096ul - 1ul)) == (4096ul - 3ul))) {
            as_int = 0;
            std::memcpy(&as_int, sv.data(), sv.size());
        } else {
            std::memcpy(&as_int, sv.data(), 4ul);
            as_int = _bzhi_u32(as_int, sv.size() * 8ul);
        }
        return as_int;
    }
    
    uint64_t load_tail_from_buffer(std::string_view sv) {
        union {
            uint64_t as_long;
            uint8_t as_buf[8];
        };
        size_t truncated_size = sv.size() < 4ul ? 0ul : sv.size() - 4ul;

        if (UNLIKELY((uint64_t(sv.data()) & (4096ul - 1ul)) > (4096ul - 12ul))) {
            as_long = 0ul;
            // Ensure nothing is done on truncated size == 0
            // See https://www.imperialviolet.org/2016/06/26/nonnull.html
            // for why we cannot use memcpy
            for (size_t i = 0; i < truncated_size; ++i) {
                as_buf[i] = sv[i + 4ul];
            }
        } else {
            as_long = 0;
            std::memcpy(&as_long, sv.data() + 4ul, 8ul);
            as_long = _bzhi_u64(as_long, truncated_size * 8ul);

        }
        return as_long;
    }


    verb
    string_to_verb_v5(std::string_view v)
    {
        if (v.size() < 3 or v.size() > 13) {
            return verb::unknown;
        }

        uint32_t head = load_head_from_buffer(v);
        uint64_t tail = load_tail_from_buffer(v);
        int idx = idx_from_data_and_size(head, (uint32_t) v.size());
        verb res = (verb) LOOKUP_TABLE[idx];
        auto slot = LOOKUP_TABLE_STR_SCALAR[idx];

        if (LIKELY(slot.size == v.size()) && LIKELY(slot.head == head) && LIKELY(slot.tail == tail)) {
            return res;
        } else {
            return verb::unknown;
        }
    }
    
    verb
    string_to_verb_v6(std::string_view v)
    {
        if (v.size() < 3 or v.size() > 13) {
            return verb::unknown;
        }

        uint32_t head = load_head_from_buffer(v);
        uint64_t tail = load_tail_from_buffer(v);
        int idx = idx_from_data_and_size(head, (uint32_t) v.size());
        verb res = (verb) LOOKUP_TABLE[idx];
        auto slot = LOOKUP_TABLE_STR_SCALAR[idx];

        // if (LIKELY(slot.size == v.size()) && LIKELY(slot.head == head) && LIKELY(slot.tail == tail)) {
        uint8_t input_size = v.size();
        asm goto (
            "cmpb %0, %1;                \n\t"
            "jne %l[false_positive];     \n\t"
            "cmpl %2, %3;                \n\t"
            "jne %l[false_positive];     \n\t"
            "cmpq %4, %5;                \n\t"
            "jne %l[false_positive];     \n\t"
            : /* no outputs */
            : "r" (slot.size), "r" (input_size),
              "r" (slot.head), "r" (head),
              "r" (slot.tail), "r" (tail) /* inputs */
            : "cc" /* clobbers flags */
            : false_positive
        );
        return res;

false_positive:
            //__attribute__((cold));
            return verb::unknown;
    }
}
