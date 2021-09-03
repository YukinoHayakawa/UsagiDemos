#pragma once

namespace usagi
{
struct ServiceColorChoice
{
    using ServiceT = ServiceColorChoice;

    ServiceT & get_service() { return *this; }

    int color_choice = 0;
};
}
