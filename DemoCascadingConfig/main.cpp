#include <iostream>

#include <Usagi/Modules/Runtime/Asset/AssetManager.hpp>
#include <Usagi/Modules/Runtime/Asset/Package/AssetPackageFilesystem.hpp>
#include <Usagi/Modules/Runtime/Executive/ServiceAsyncWorker.hpp>
#include <Usagi/Modules/Assets/SahJson/SahInheritableJsonConfig.hpp>

using namespace usagi;

int main(int argc, char *argv[])
{
    AssetManager asset_manager;
    asset_manager.add_package(std::make_unique<AssetPackageFilesystem>("."));
    ServiceAsyncWorker wq;

    SecondaryAssetMeta meta = asset_manager.secondary_asset(
        std::make_unique<SahInheritableJsonConfig>("3.json"),
        wq.get_service()
    );
    while(meta.status != AssetStatus::READY)
    {
        meta = asset_manager.secondary_asset(meta.fingerprint_build);
    }
    std::cout << meta.asset->as<JsonTree>()->tree << std::endl;
}
