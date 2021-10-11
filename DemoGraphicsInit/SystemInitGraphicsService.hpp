#pragma once

#include <Usagi/Runtime/Service/Service.hpp>
#include <Usagi/Runtime/Service/LazyInitService.hpp>

#include <Usagi/Modules/Platforms/Vulkan/VulkanGpuDevice.hpp>

namespace usagi
{
USAGI_DECL_ALIAS_SERVICE(
    ServiceHardwareGraphics,
    LazyInitService<VulkanGpuDevice>
);

struct SystemInitGraphicsService
{
    using WriteAccess = C<>;
    using ReadAccess = C<>;

    void update(auto &&rt, auto &&db)
    {
        // touch the service so it will be initialized.
        (void)USAGI_SERVICE(rt, ServiceHardwareGraphics);
    }
};
}
