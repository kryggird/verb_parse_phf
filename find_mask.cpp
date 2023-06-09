#include <algorithm>
#include <array>
#include <bitset>
#include <cstdint>
#include <cstring> // memcpy
#include <string_view>
#include <unordered_map>
#include <vector>

#include <x86intrin.h>

struct HashKey { uint32_t key; uint32_t size; };

namespace std {
    template <> struct hash<HashKey> {
        size_t operator()(const HashKey& data) const {
            return size_t(data.size) << 32ul | size_t(data.key);
        }
    };

    template <> struct equal_to<HashKey> {
        constexpr bool operator()(const HashKey& l, const HashKey& r) const {
            return l.key == r.key && l.size == r.size;
        }
    };
}


using namespace std;
using hash_map_t = unordered_map<string_view, int>;
using int_hash_map_t = unordered_map<HashKey, int>; // Size, enum

static hash_map_t VERBS_MAP {
    {"DELETE", 0}, {"GET", 1}, {"HEAD", 2}, {"POST", 3},
    {"PUT", 4}, {"CONNECT", 5}, {"OPTIONS", 6}, {"TRACE", 7},
    {"COPY", 8}, {"LOCK", 9}, {"MKCOL", 10}, {"MOVE", 11},
    {"PROPFIND", 12}, {"PROPPATCH", 13}, {"SEARCH", 14}, {"UNLOCK", 15},
    {"BIND", 16}, {"REBIND", 17}, {"UNBIND", 18}, {"ACL", 19},
    {"REPORT", 20}, {"MKACTIVITY", 21}, {"CHECKOUT", 22}, {"MERGE", 23},
    {"M-SEARCH", 24}, {"NOTIFY", 25}, {"SUBSCRIBE", 26}, {"UNSUBSCRIBE", 27},
    {"PATCH", 28}, {"PURGE", 29}, {"MKCALENDAR", 30}, {"LINK", 31},
    {"UNLINK", 32},
};

static int_hash_map_t VERBS_MAP_WITH_UNDEFINED = []() {
    int_hash_map_t res {}; 
    for (auto& [k, v]: VERBS_MAP) {
        uint32_t k_as_int = 0u;
        memcpy(&k_as_int, k.data(), min(k.size(), sizeof(k_as_int)));
        res[HashKey {k_as_int, uint32_t(k.size())}] = v;

        if (k.size() < 4ul) {
            uint32_t old_key = k_as_int;
            for (uint64_t i = 0ul; i < uint32_t(-1); i += 0x1'00'00'00ul) {
                uint32_t k_as_int = uint32_t(i);
                memcpy(&k_as_int, k.data(), k.size());
                if (k_as_int != old_key) {
                    res[HashKey {k_as_int, uint32_t(k.size())}] = v;
                }
                old_key = k_as_int;
            }
        }
    }
    return res;
}();

// get next greater value with same number of one bits
uint32_t snoob (uint32_t x) {
   uint32_t smallest = x & -x;
   uint32_t ripple = x + smallest;
   uint32_t ones = x ^ ripple;
   ones = (ones >> 2) / smallest;
   return ripple | ones;
}

template <uint64_t SZ, typename F>
bool is_uniq(const hash_map_t& map, F func) {
    bitset<SZ> bitset {};

    for (auto& [k, v]: map) {
        uint32_t idx = func(k);
        uint32_t old = bitset.test(idx);
        if (old) { return false; }
        bitset.set(idx);
    }
    return true;
}

template <uint64_t SZ, typename F>
bool is_uniq_dedup(const int_hash_map_t& map, F func) {
    int slots[SZ];
    for (int i = 0; i < SZ; ++i) { slots[i] = -1; }

    for (auto& [k, val]: map) {
        uint32_t idx = func(k.key, uint64_t(k.size));
        int slot_val = slots[idx];
        if (slot_val == -1 || slot_val == val) { 
            slots[idx] = val;
        } else { 
            return false; 
        }
    }
    return true;
}

void find_mask_by_length() {
    vector<string_view> sorted;
    for (auto& [k, v]: VERBS_MAP) { sorted.push_back(k); }
    sort(sorted.begin(), sorted.end(), [](auto& l, auto& r) {
        return (l.size() < r.size())
            || ((l.size() == r.size()) && (l < r));
    });

    unordered_map<size_t, hash_map_t> maps_by_length;
    unordered_map<size_t, uint32_t> masks_by_length;
    unordered_map<string_view, uint32_t> idx_by_string;
    for (auto& l: sorted) {
        uint32_t l_as_int = 0u;
        memcpy(&l_as_int, l.data(), min(l.size(), sizeof(l_as_int)));

        maps_by_length[l.size()][l] = VERBS_MAP[l];
    }
    
    for (const auto& [len, map]: maps_by_length) {
        uint32_t best_mask = 0u;
        for (int i = 1; (best_mask == 0) && (i < 8); ++i) {
            uint32_t mask = (1u << i) - 1u;
            for (; snoob(mask) > mask; mask = snoob(mask)) {
                auto lam = [=](string_view sv) { 
                    uint32_t val = 0u;
                    memcpy(&val, sv.data(), min(sv.size(), sizeof(val)));

                    return _pext_u32(val, mask);
                };

                if (is_uniq<128ul>(map, lam)) {
                    best_mask = mask;

                    for (const auto& [l, v]: map) {
                        idx_by_string[l] = lam(l);
                    }
                    break;
                }
            }

        }
        masks_by_length[len] = best_mask;
    }

    for (auto& l: sorted) {
        uint32_t mask = masks_by_length[l.size()];
        uint32_t idx = idx_by_string[l];

        printf("%lu\t" "%#8x\t" "%u\t" "%u\t" "%u\t" "%s\n",
                l.size(), mask, idx, 1 << _popcnt32(mask), VERBS_MAP[l], l.data());
    }
}

void find_mask_global() {
    uint32_t best_mask = 0u;
    for (uint32_t mask = 0b1'111'111u; snoob(mask) > mask; mask = snoob(mask)) {
        // auto lam = [=](string_view sv) { 
        //     uint32_t val = 0u;
        //     memcpy(&val, sv.data(), min(sv.size(), sizeof(val)));
        //     return _pext_u32(val, mask) ^ sv.size();
        // };
        auto lam = [=](uint32_t val, uint64_t sz) { 
            return _pext_u32(val, mask) ^ sz;
        };

        //if (is_uniq<128ul>(VERBS_MAP, lam)) {
        if (is_uniq_dedup<128ul>(VERBS_MAP_WITH_UNDEFINED, lam)) {
            best_mask = mask;
            break;
        }
    }

    if (best_mask) {
        printf("Found mask: %u\n", best_mask);
    } else {
        printf("No mask found!\n");
    }

    printf("%lu\n", VERBS_MAP_WITH_UNDEFINED.size());

    __m128i xmm = _mm_loadu_si128((const __m128i*) "DELETE");
    uint64_t movmask = _mm_movemask_epi8(xmm);
    printf("%lu\n", movmask);
}

int main() {
    find_mask_by_length();
}
