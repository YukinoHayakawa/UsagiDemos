#pragma once

#include <Usagi/Runtime/Service/Service.hpp>

#include "Service_graphics_gdi.hpp"

namespace usagi
{
struct System_gdi_present
{
    using WriteAccess = ComponentFilter<>;
    using ReadAccess = ComponentFilter<>;

    template <typename RuntimeServices, typename EntityDatabaseAccess>
    void update(RuntimeServices &&rt, EntityDatabaseAccess &&db)
    {
        USAGI_SERVICE(rt, Service_graphics_gdi).present();
    }
};
}
