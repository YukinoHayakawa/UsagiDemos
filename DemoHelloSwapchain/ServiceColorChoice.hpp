#pragma once

#include <Usagi/Runtime/Service/ServiceAccess.hpp>

namespace usagi
{
struct ServiceColorChoice
{
    using ServiceT = ServiceColorChoice;

    ServiceT & get_service() { return *this; }

    int color_choice = 0;
};

USAGI_DECL_SERVICE_ALIAS(ServiceColorChoice, color_choice);
}
