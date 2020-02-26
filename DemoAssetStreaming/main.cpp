#include <cstdio>
#include <iostream>
#include <chrono>

#include <Usagi/Module/Service/Asset/AssetManager.hpp>
#include <Usagi/Module/Service/Asset/AssetSourceFilesystemDirectory.hpp>

using namespace usagi;
using timer = std::chrono::high_resolution_clock;

int main(int argc, char *argv[])
{
    AssetManager am;
    am.add_source(std::make_unique<AssetSourceFilesystemDirectory>("."));

    std::shared_ptr<MemoryRegion> src;

    const auto start = timer::now();

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
            break;
        }

        auto end = timer::now();
        const auto elapsed = std::chrono::duration_cast<
            std::chrono::microseconds
        >(end - start).count();

        printf("Loading... %lld us\n", elapsed);
    }

    using namespace platform::file;

    MemoryMappedFile output {
        RegularFile {
            u8"./output.png",
            FileOpenMode(OPEN_READ | OPEN_WRITE),
            FileOpenOptions(OPTION_CREATE_IF_MISSING)
        },
        MAPPING_WRITE,
        src->length
    };

    printf("Copying...\n");
    memcpy(output.base_view(), src->base_address, src->length);
    output.flush(0, src->length);
    printf("Copied.\n");
}
