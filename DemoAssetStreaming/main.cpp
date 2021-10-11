// #include <cstdio>
// #include <chrono>
//
//
// using timer = std::chrono::high_resolution_clock;
//
// int main(int argc, char *argv[])
// {
//
//
//     const auto start = timer::now();
//
//
//     // Fetch the file
//     while(true)
//     {
//         auto result = am.request_cached_asset<MemoryRegion>(
//             src,
//             AssetPriority::NORMAL,
//             ASSET_LOAD_IF_MISSING
//         );
//
//         if(result)
//         {
//             printf("Loaded.\n");
//             src_region = result.value();
//             break;
//         }
//
//         auto end = timer::now();
//         const auto elapsed = std::chrono::duration_cast<
//             std::chrono::microseconds
//         >(end - start).count();
//
//         printf("Loading... %lld us\n", elapsed);
//     }
//
//     using namespace platform::file;
//
//     MemoryMappedFile output {
//         RegularFile {
//         std::u8string(dst),
//         FileOpenMode(OPEN_READ | OPEN_WRITE),
//         FileOpenOptions(OPTION_CREATE_IF_MISSING)
//     },
//         MAPPING_WRITE,
//         src_region->length
//     };
//
//     printf("Copying...\n");
//     memcpy(output.base_view(), src_region->base_address, src_region->length);
//     output.flush(0, src_region->length);
//     printf("Copied.\n");
// }

#include <cstdio>
#include <cmath>

constexpr float PI = 3.1415926535f;
constexpr int n = 200;
constexpr float h = PI / n;

float u[n + 1];


// float cos_xj[n + 1];
// float p1[n + 1];
// // float p2[n + 1];
// float p3[n + 1];

bool present[n + 1] = { };

float calc(int j) {
    if(present[j]) return u[j];

    float xj = j * h;
    float cos_xj = cos(xj);
    float p2 = calc(j + 1) / (h * h);
    float p1 = calc(j - 1) / (h * h);
    float p3 = 20.f + 10.f * sin(xj) - 2.f / (h * h);

    float v = (cos_xj - p1 - p2) / p3;
    present[j] = true;
    u[j] = v;
    return v;

};

int main(int argc, char const *argv[])
{
    u[0] = 0;
    u[n] = 0;
    present[0] = present[n] = true;



    for(int j = 1; j < n; ++j)
    {
        printf("%f\n", calc(j));
    }
    // uj = (cos_xj - u_j-1 / h^2 - u_j+1 / h^2) / (20 + 10 sin_xj - 2 / h^2)
    // // u     cos_xj   p1            p2             p3
    //
    // // first pass: iterate forward and compute cos_xj, p1, p3
    // // second pass: iterate backward and compute p2 and uj
    //
    // for(int j = 1; j < n; ++j)
    // {
    //     float xj = j * h;
    //     cos_xj[j] = cos(xj);
    //     p1[j] = u[j-1] / (h * h);
    //     p3[j] = 20.f + 10.f * sin(xj) - 2.f / (h * h);
    // }
    //
    // for(int j = n - 1; j > 0; --j)
    // {
    //     float p2 = u[j+1] / (h * h);
    //     u[j] = cos_xj[j] - p1[j] - p2 / p3[j];
    // }
    //
    // for(int j = 1; j < n; ++j)
    // {
    //     float xj = j * h;
    //     float lhs = (u[j-1] - 2.f * u[j] + u[j+1]) / (h * h) + (20.f + 10.f * sin(xj)) * u[j];
    //     float rhs = cos(xj);
    //     printf("lhs = %f, rhs = %f\n", lhs, rhs);
    // }

    return 0;
}
