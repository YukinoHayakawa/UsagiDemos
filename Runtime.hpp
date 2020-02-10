#pragma once

#include <type_traits>

#define USAGI_SERVICE(rt, svc_type) \
    static_cast<svc_type::ServiceType &>(rt.svc_type::get_service())

#define USAGI_OPT_SERVICE(rt, svc_type, svc_var, task) \
    do { \
        if constexpr(std::is_base_of_v< \
            svc_type, std::remove_cvref_t<decltype(rt)>>) \
        { \
            auto &svc_var = USAGI_SERVICE(rt, svc_type); \
            task \
        } \
    } while(false) \
/**/

//
// #define USAGI_DEFINE_SERVICE(name, service) \
// struct Service_graphics
// {
// using ServiceType = ServiceGraphics;
// };
