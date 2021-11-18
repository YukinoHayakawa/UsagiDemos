#include <iostream>

#include <Usagi/Modules/Runtime/Asset/AssetManager.hpp>
#include <Usagi/Modules/Runtime/Asset/SecondaryAsset.hpp>
#include <Usagi/Modules/Runtime/Asset/SecondaryAssetHandler.hpp>
#include <Usagi/Modules/Runtime/Asset/Package/AssetPackageFilesystem.hpp>
#include <Usagi/Modules/Runtime/Executive/ServiceAsyncWorker.hpp>

using namespace usagi;

class Text : public SecondaryAsset
{
public:
    std::string string;
};

class Sah1 : public SecondaryAssetHandler<Text>
{
protected:
    std::unique_ptr<SecondaryAsset> construct() override
    {
        auto dep1 = primary_asset_async("1.txt");
        auto dep2 = primary_asset_async("2.txt");
        auto val1 = dep1.get();
        auto val2 = dep2.get();

        auto txt = std::make_unique<Text>();

        txt->string = val1.region.to_string_view();
        txt->string += val2.region.to_string_view();

        return std::move(txt);
    }

    void append_features(Hasher &hasher) override
    {
        hasher.append("test");
    }
};

class Sah2 : public SecondaryAssetHandler<Text>
{
protected:
    std::unique_ptr<SecondaryAsset> construct() override
    {
        auto dep1 = secondary_asset_async(std::make_unique<Sah1>());
        auto dep2 = primary_asset_async("3.txt");

        auto val1 = dep1.get();
        auto val2 = dep2.get();

        auto txt = std::make_unique<Text>();

        txt->string = val1.asset->as<Text>()->string;
        txt->string += val2.region.to_string_view();

        return std::move(txt);
    }
};

int main(int argc, char *argv[])
{
    AssetManager asset_manager;
    asset_manager.add_package(std::make_unique<AssetPackageFilesystem>("."));
    ServiceAsyncWorker wq;

    SecondaryAssetMeta meta = asset_manager.secondary_asset(
        std::make_unique<Sah2>(),
        wq.get_service()
    );
    while(meta.status != AssetStatus::READY)
    {
        meta = asset_manager.secondary_asset(meta.fingerprint_build);
    }
    std::cout << meta.asset->as<Text>()->string << std::endl;
}
