#pragma once

#define USAGI_SERVICE(rt, svc_type) \
    static_cast<svc_type::ServiceType &>(rt.svc_type::get_service())
/**/
//
// #define USAGI_DEFINE_SERVICE(name, service) \
// struct Service_graphics
// {
// using ServiceType = ServiceGraphics;
// };
