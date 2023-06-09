
#include "http_verb.h"

namespace pext_split {
    using ::boost::beast::http::verb;

    verb string_to_verb(std::string_view s)
    {
        switch (s.size()) {
            case 0:
            case 1:
            case 2:
                break;

            case 3: {
                static char lookup[4][3] = {
                    { 'P', 'U', 'T' }, { 'A', 'C', 'L' }, {}, { 'G', 'E', 'T' }
                };
                static uint8_t value[4] = { 4, 19, 33, 1 };
                uint32_t as_uint = 0;
                std::memcpy(&as_uint, s.data(), 3);

                uint64_t idx = _pext_u32(as_uint, 3);
                if (std::memcmp(lookup[idx], s.data(), 3) == 0) {
                    return (verb)value[idx];
                }
                break;
            }

            case 4: {
                static char lookup[8][4] = {
                    { 'B', 'I', 'N', 'D' }, { 'L', 'I', 'N', 'K' },
                    { 'C', 'O', 'P', 'Y' }, { 'M', 'O', 'V', 'E' },
                    { 'H', 'E', 'A', 'D' }, {},
                    { 'P', 'O', 'S', 'T' }, { 'L', 'O', 'C', 'K' }
                };
                static uint8_t value[8] = { 16, 31, 8, 11, 2, 33, 3, 9 };
                uint32_t as_uint = 0;
                std::memcpy(&as_uint, s.data(), 4);

                uint64_t idx = _pext_u32(as_uint, 66052);
                if (std::memcmp(lookup[idx], s.data(), 4) == 0) {
                    return (verb)value[idx];
                }
                break;
            }

            case 5: {
                static char lookup[8][5] = { { 'P', 'A', 'T', 'C', 'H' },
                                             {},
                                             { 'T', 'R', 'A', 'C', 'E' },
                                             { 'M', 'K', 'C', 'O', 'L' },
                                             { 'P', 'U', 'R', 'G', 'E' },
                                             {},
                                             {},
                                             { 'M', 'E', 'R', 'G', 'E' } };
                static uint8_t value[8] = { 28, 33, 7, 10, 29, 33, 33, 23 };
                uint32_t as_uint = 0;
                std::memcpy(&as_uint, s.data(), 4);

                uint64_t idx = _pext_u32(as_uint, 1029);
                if (std::memcmp(lookup[idx], s.data(), 5) == 0) {
                    return (verb)value[idx];
                }
                break;
            }

            case 6: {
                static char lookup[16][6] = { { 'D', 'E', 'L', 'E', 'T', 'E' },
                                              { 'U', 'N', 'L', 'I', 'N', 'K' },
                                              { 'N', 'O', 'T', 'I', 'F', 'Y' },
                                              {},
                                              {},
                                              { 'U', 'N', 'B', 'I', 'N', 'D' },
                                              { 'R', 'E', 'B', 'I', 'N', 'D' },
                                              {},
                                              {},
                                              { 'U', 'N', 'L', 'O', 'C', 'K' },
                                              { 'R', 'E', 'P', 'O', 'R', 'T' },
                                              { 'S', 'E', 'A', 'R', 'C', 'H' },
                                              {},
                                              {},
                                              {},
                                              {} };
                static uint8_t value[16] = { 0,  32, 25, 33, 33, 18, 17, 33,
                                             33, 15, 20, 14, 33, 33, 33, 33 };
                uint32_t as_uint = 0;
                std::memcpy(&as_uint, s.data(), 4);

                uint64_t idx = _pext_u32(as_uint, 33685507);
                if (std::memcmp(lookup[idx], s.data(), 6) == 0) {
                    return (verb)value[idx];
                }
                break;
            }

            case 7: {
                static char lookup[2][7] = {
                    { 'C', 'O', 'N', 'N', 'E', 'C', 'T' },
                    { 'O', 'P', 'T', 'I', 'O', 'N', 'S' }
                };
                static uint8_t value[2] = { 5, 6 };
                uint32_t as_uint = 0;
                std::memcpy(&as_uint, s.data(), 4);

                uint64_t idx = _pext_u32(as_uint, 4);
                if (std::memcmp(lookup[idx], s.data(), 7) == 0) {
                    return (verb)value[idx];
                }
                break;
            }

            case 8: {
                static char lookup[4][8] = {
                    { 'P', 'R', 'O', 'P', 'F', 'I', 'N', 'D' },
                    { 'M', '-', 'S', 'E', 'A', 'R', 'C', 'H' },
                    {},
                    { 'C', 'H', 'E', 'C', 'K', 'O', 'U', 'T' }
                };
                static uint8_t value[4] = { 12, 24, 33, 22 };
                uint32_t as_uint = 0;
                std::memcpy(&as_uint, s.data(), 4);

                uint64_t idx = _pext_u32(as_uint, 3);
                if (std::memcmp(lookup[idx], s.data(), 8) == 0) {
                    return (verb)value[idx];
                }
                break;
            }

            case 9: {
                static char lookup[2][9] = {
                    { 'P', 'R', 'O', 'P', 'P', 'A', 'T', 'C', 'H' },
                    { 'S', 'U', 'B', 'S', 'C', 'R', 'I', 'B', 'E' }
                };
                static uint8_t value[2] = { 13, 26 };
                uint32_t as_uint = 0;
                std::memcpy(&as_uint, s.data(), 4);

                uint64_t idx = _pext_u32(as_uint, 1);
                if (std::memcmp(lookup[idx], s.data(), 9) == 0) {
                    return (verb)value[idx];
                }
                break;
            }

            case 10: {
                static char lookup[2][10] = {
                    { 'M', 'K', 'A', 'C', 'T', 'I', 'V', 'I', 'T', 'Y' },
                    { 'M', 'K', 'C', 'A', 'L', 'E', 'N', 'D', 'A', 'R' }
                };
                static uint8_t value[2] = { 21, 30 };
                uint32_t as_uint = 0;
                std::memcpy(&as_uint, s.data(), 4);

                uint64_t idx = _pext_u32(as_uint, 131072);
                if (std::memcmp(lookup[idx], s.data(), 10) == 0) {
                    return (verb)value[idx];
                }
                break;
            }

            case 11: {
                static char lookup[2][11] = {
                    {},
                    { 'U', 'N', 'S', 'U', 'B', 'S', 'C', 'R', 'I', 'B', 'E' }
                };
                static uint8_t value[2] = { 33, 27 };
                uint32_t as_uint = 0;
                std::memcpy(&as_uint, s.data(), 4);

                uint64_t idx = _pext_u32(as_uint, 1);
                if (std::memcmp(lookup[idx], s.data(), 11) == 0) {
                    return (verb)value[idx];
                }
                break;
            }

            default:
                break;
        }

        return verb::unknown;
    }
} // namespace pext_split
