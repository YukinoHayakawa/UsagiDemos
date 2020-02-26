#include <cstdio>
#include <iostream>
#include <chrono>

#include <Usagi/Module/Service/Asset/AssetManager.hpp>
#include <Usagi/Module/Service/Asset/AssetSourceFilesystemDirectory.hpp>

using namespace usagi;
using timer = std::chrono::high_resolution_clock;

void mem_map_copy(
    const std::u8string_view src, 
    const std::u8string_view dst, 
    bool blocking)
{
    AssetManager am;
    am.add_source(std::make_unique<AssetSourceFilesystemDirectory>("."));

    std::shared_ptr<MemoryRegion> src_region;

    const auto start = timer::now();

    // Fetch the file
    while(true)
    {
        auto result = am.request_raw_asset(
            src,
            AssetPriority::NORMAL,
            AssetRequestOption(
                ASSET_LOAD_IF_MISSING |
                (blocking ? ASSET_BLOCKING_LOAD : 0)
            )
        );
        if(result)
        {
            printf("Loaded.\n");
            src_region = result.value();
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
            std::u8string(dst),
            FileOpenMode(OPEN_READ | OPEN_WRITE),
            FileOpenOptions(OPTION_CREATE_IF_MISSING)
        },
        MAPPING_WRITE,
        src_region->length
    };

    printf("Copying...\n");
    memcpy(output.base_view(), src_region->base_address, src_region->length);
    output.flush(0, src_region->length);
    printf("Copied.\n");
}

int main(int argc, char *argv[])
{
    mem_map_copy(u8"input.png", u8"output_nonblocking.png", false);
    mem_map_copy(u8"input.png", u8"output_blocking.png", true);
}
