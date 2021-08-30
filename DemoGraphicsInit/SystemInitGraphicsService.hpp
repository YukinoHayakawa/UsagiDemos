#pragma once

#include <Usagi/Entity/EntityDatabase.hpp>
#include <Usagi/Module/Common/Color/Color.hpp>
#include <Usagi/Runtime/Service.hpp>

#include <Usagi/Module/Platform/Vulkan/VulkanGpuDevice.hpp>
#include <Usagi/Module/Service/Graphics/Enum.hpp>
#include <Usagi/Module/Service/Windowing/ServiceNativeWindowManager.hpp>

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
