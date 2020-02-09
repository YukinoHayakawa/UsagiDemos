#pragma once

#include <cstddef>

namespace usagi
{
struct stat
{
    std::size_t sprite_count;
    double time_input;
    double time_spawn;
    double time_explode;
    double time_fade;
    double time_physics;
    double time_remove;
    double time_clear;
    double time_render;
    double time_stat;
    double time_present;
};

struct Service_stat
{
    using ServiceType = stat;

    stat value;

    ServiceType & get_service()
    {
        return value;
    }
};
}
