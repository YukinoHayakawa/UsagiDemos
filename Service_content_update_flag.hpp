#pragma once

namespace usagi
{
struct Service_content_update_flag
{
    using ServiceType = bool;

    ServiceType & get_service()
    {
        return mOptional;
    }

private:
    bool mOptional = false;
};

}
