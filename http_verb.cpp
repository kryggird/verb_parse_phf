#include "http_verb.h"

#include <array>
#include <cstdint>
#include <cstring> // memcpy


/*constexpr std::array<uint32_t, COUNT> verb_prefix = []() {
    std::array<uint32_t, COUNT> res = {0u};
    for (uint64_t idx = 0; idx < COUNT; ++idx) {
        auto verb = verb2string[idx];
        uint32_t max_count = (verb.size() > 4ul) 
                           ? 4u 
                           : ((uint32_t) verb.size());
        
        for (uint32_t inner = 0; inner < max_count; ++inner) {
            res[idx] |= ((uint8_t) verb[inner]) << (inner * 8u);
        }
    }
    return res;
}();
*/

namespace boost::beast::http {
    const std::string_view as_string(verb v) {
        if (v == verb::unknown) {
            return verb_constants::unknown_sv;
        } else {
            return verb_constants::ALL_VERBS.at(std::underlying_type<verb>::type(v));
        }
    }

    /*uint32_t prefix_as_uint(verb v) {
        if (v == verb::unknown) {
            return 0u;
        } else {
            return verb_prefix.at(std::underlying_type<verb>::type(v));
        }

    }*/
}
