#include <cstdio>

#include <Usagi/Module/Service/ServiceAsset/AssetManager.hpp>
#include <Usagi/Module/Service/ServiceAsset/AssetSourceFilesystemDirectory.hpp>

using namespace usagi;

int main(int argc, char *argv[])
{
    AssetManager am;
    am.add_source(std::make_unique<AssetSourceFilesystemDirectory>("."));

    std::shared_ptr<MemoryRegion> src;
    // Fetch the file
    while(true)
    {
        auto result = am.request_raw_asset(
            u8"./input.png",
            AssetPriority::NORMAL,
            ASSET_LOAD_IF_MISSING
        );
        if(result)
        {
            printf("Loaded.\n");
            src = result.value();
            break;;
        }
        else
        {
            printf("Loading...\n");
        }
    }

    MemoryMappedFile output {
        RegularFile {
            u8"./output.png",
            platform::file::FileOpenMode::READ_WRITE,
            platform::file::FileOpenOptions::CREATE_IF_MISSING
        },
        platform::memory::MemoryMappingMode::READ_WRITE,
        src->length
    };

    printf("Copying...\n");
    memcpy(output.base_view(), src->base_address, src->length);
    output.flush(0, src->length);
    printf("Copied.\n");
}
