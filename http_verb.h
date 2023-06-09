// Extracted from https://github.com/boostorg/beast/blob/855fc23885307aeb6ad1318001a4bbdd4345d47c/include/boost/beast/http/impl/verb.ipp

#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace boost::beast::http {

    enum class verb {
        delete_,
        get,
        head,
        post,
        put,
        connect,
        options,
        trace,
        copy,
        lock,
        mkcol,
        move,
        propfind,
        proppatch,
        search,
        unlock,
        bind,
        rebind,
        unbind,
        acl,
        report,
        mkactivity,
        checkout,
        merge,
        msearch,
        notify,
        subscribe,
        unsubscribe,
        patch,
        purge,
        mkcalendar,
        link,
        unlink,
        unknown
    };

    constexpr unsigned int verb_count = 33;

    const std::string_view as_string(verb v);
    uint32_t prefix_as_uint(verb v);
}

namespace verb_constants {
    constexpr auto COUNT = boost::beast::http::verb_count;

    constexpr std::string_view unknown_sv {"UNKNOWN"};
    constexpr std::array<std::string_view, COUNT> ALL_VERBS = {
        "DELETE",
        "GET",
        "HEAD",
        "POST",
        "PUT",
        "CONNECT",
        "OPTIONS",
        "TRACE",
        "COPY",
        "LOCK",
        "MKCOL",
        "MOVE",
        "PROPFIND",
        "PROPPATCH",
        "SEARCH",
        "UNLOCK",
        "BIND",
        "REBIND",
        "UNBIND",
        "ACL",
        "REPORT",
        "MKACTIVITY",
        "CHECKOUT",
        "MERGE",
        "M-SEARCH",
        "NOTIFY",
        "SUBSCRIBE",
        "UNSUBSCRIBE",
        "PATCH",
        "PURGE",
        "MKCALENDAR",
        "LINK",
        "UNLINK"
    };
    constexpr std::array<std::string_view, 3> COMMON_VERBS = {
        "GET", "POST", "PUT"
    };

}
