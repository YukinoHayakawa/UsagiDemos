#pragma once

#include <Usagi/Core/Clock.hpp>

namespace usagi
{
struct Service_master_clock
{
    using ServiceType = Clock;
    virtual ~Service_master_clock() = default;

    ServiceType & get_service()
    {
        return get_service_impl();
    }

private:
    virtual ServiceType &get_service_impl() = 0;
};

struct Service_master_clock_default : Service_master_clock
{
    Clock master_clock;

    Clock & get_service_impl() override
    {
        return master_clock;
    }
};
}
