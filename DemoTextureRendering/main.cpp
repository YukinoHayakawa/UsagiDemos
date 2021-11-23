#include <Usagi/Modules/IO/Input/ServiceInputSource.hpp>
#include <Usagi/Modules/IO/Windowing/ServiceNativeWindowManager.hpp>
#include <Usagi/Modules/Runtime/Asset/AssetManager.hpp>
#include <Usagi/Modules/Runtime/Asset/ServiceAssetManager.hpp>
#include <Usagi/Modules/Runtime/Asset/Package/AssetPackageFilesystem.hpp>
#include <Usagi/Modules/Runtime/Executive/ServiceAsyncWorker.hpp>
#include <Usagi/Modules/Runtime/Executive/ServiceStateTransitionGraph.hpp>
#include <Usagi/Runtime/Service/LazyInitService.hpp>

#include <Usagi/Modules/Platforms/WinCommon/Input/InputEventSourceWin32RawInput.hpp>
#include <Usagi/Modules/Platforms/WinCommon/Windowing/NativeWindowManagerWin32.hpp>

#include <Usagi/Modules/Platforms/Vulkan/VulkanGpuDevice.hpp>
#include <Usagi/Modules/Assets/SahGraphicsVulkan/SahVulkanGraphicsPipeline.hpp>

using namespace usagi;

USAGI_DECL_ALIAS_SERVICE(
    ServiceHardwareGraphics,
    LazyInitService<VulkanGpuDevice>
);

struct Services
    : ServiceInputSource
    , ServiceNativeWindowManager
    , ServiceStateTransitionGraph
    , ServiceHardwareGraphics
    , ServiceAsyncWorker
    , ServiceAssetManager
{
    Services()
        : ServiceInputSource(Tag<InputEventSourceWin32RawInput>())
        , ServiceNativeWindowManager(Tag<NativeWindowManagerWin32>())
    {
    }
};

int main(int argc, char *argv[])
{
    Services services;
    auto &asset_manager = USAGI_SERVICE(services, ServiceAssetManager);
    asset_manager.add_package(std::make_unique<AssetPackageFilesystem>("."));
    auto &gpu = USAGI_SERVICE(services, ServiceHardwareGraphics);

    auto asset = asset_manager.secondary_asset(
        std::make_unique<SahVulkanGraphicsPipeline>(
            &gpu, "pipeline_1.json"),
        USAGI_SERVICE(services, ServiceAsyncWorker)
    );
    while(asset.status != AssetStatus::READY)
    {
        asset = asset_manager.secondary_asset(asset.fingerprint_build);
    }
}
