#pragma once

#include <Usagi/Modules/Resources/ResWindowManager/HeapWindowManager.hpp>
#include <Usagi/Modules/Runtime/HeapManager/HeapManagerStatic.hpp>
#include <Usagi/Runtime/Service/LazyInitService.hpp>
#include <Usagi/Runtime/Service/ServiceAccess.hpp>
#include <Usagi/Runtime/Service/Service.hpp>

#include <Usagi/Modules/Platforms/Vulkan/VulkanGpuDevice.hpp>
#include <Usagi/Modules/Resources/ResGraphicsVulkan/HeapVulkanObjectManager.hpp>
#include <Usagi/Modules/Resources/ResVulkanWSI/RbVulkanSwapchain.hpp>

namespace usagi
{
USAGI_DECL_ALIAS_SERVICE(
    ServiceHardwareGraphics,
    LazyInitService<VulkanGpuDevice>
);

USAGI_DECL_SERVICE_ALIAS(ServiceHardwareGraphics, graphics);

using AppServiceHeapManager = SimpleService<HeapManagerStatic<
    HeapWindowManager,
    HeapVulkanObjectManager // todo this should be platform independent name
>>;

USAGI_DECL_SERVICE_ALIAS(AppServiceHeapManager, heap_manager);
}
