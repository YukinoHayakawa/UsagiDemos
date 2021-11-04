
#include <cassert>
#include <future>
#include <iostream>

#include <Usagi/Library/Memory/LockGuard.hpp>
#include <Usagi/Modules/Assets/SahProgramModule/SahProgramModule.hpp>
#include <Usagi/Modules/Common/Time/Clock.hpp>
#include <Usagi/Modules/Runtime/Asset/AssetManager.hpp>
#include <Usagi/Modules/Runtime/Asset/SecondaryAsset.hpp>
#include <Usagi/Modules/Runtime/Asset/SecondaryAssetHandler.hpp>
#include <Usagi/Modules/Runtime/Asset/Package/AssetPackageFilesystem.hpp>
#include <Usagi/Modules/Runtime/ProgramModule/ClangJIT.hpp>
#include <Usagi/Modules/Runtime/ProgramModule/RuntimeModule.hpp>
#include <Usagi/Runtime/Task.hpp>

using namespace usagi;

class StdTaskExecutor : public TaskExecutor
{
    std::map<std::uint64_t, std::future<void>> mTask;
    using TaskRef = decltype(mTask)::iterator;
    std::mutex mMutex;
    std::uint64_t mTaskId = 0;

public:
    std::uint64_t submit(
        std::unique_ptr<Task> task,
        std::optional<std::vector<std::uint64_t>> wait_on) override
    {
        std::lock_guard lk(mMutex);

        auto task_id = ++mTaskId;

        auto future = std::async(
            std::launch::async,
            [t = std::move(task), wait = std::move(wait_on), this]() {
                Clock clk;
                // using namespace std::chrono_literals;
                // std::this_thread::sleep_for(5ms);

                if(wait.has_value())
                {
                    for(auto &&w : *wait)
                    {
                        LockGuard lk(mMutex);
                        auto wt = mTask.find(w);
                        assert(wt != mTask.end());
                        lk.unlock();
                        wt->second.wait();
                    }
                }

                if(!t->precondition()) throw std::runtime_error("");
                t->on_started();
                t->run();
                t->on_finished();
                if(!t->postcondition()) throw std::runtime_error("");

                std::cout << clk.realtime_elapsed() << std::endl;
            }
        );

        mTask.emplace(task_id, std::move(future));

        return task_id;
    }
};

int main(int argc, char *argv[])
{
    ClangJIT jit;
    StdTaskExecutor executor;
    AssetManager asset_manager;
    asset_manager.add_package(std::make_shared<AssetPackageFilesystem>("."));
    const auto nt = 32;
    std::vector<std::thread> ts;
    ts.reserve(nt);

    for(int i = 0; i < nt; ++i)
    {
        ts.emplace_back([&]() {
            // PrimaryAsset asset;

            auto handler = std::make_unique<SahProgramModule>(
                jit,
                "foo2.pch",
                "test.cpp"
            );

            SecondaryAssetMeta asset_sec = asset_manager.secondary_asset(
                std::move(handler),
                executor
            );
            do
            {
                // asset = asset_manager.primary_asset(argv[1], &executor);
                asset_sec = asset_manager.secondary_asset(asset_sec.fingerprint_build);
                std::cout << (std::uint64_t)asset_sec.status << " ";
                // switch(asset.status)
                // {
                //     case AssetStatus::PRIMARY_FOUND:
                //         std::cout << "Found" << std::endl;
                //         break;
                //     case AssetStatus::PRIMARY_PENDING:
                //         std::cout << "Pending" << std::endl;
                //         break;
                //     case AssetStatus::PRIMARY_LOADING:
                //         std::cout << "Loading" << std::endl;
                //         break;
                //     case AssetStatus::PRIMARY_READY:
                //         std::cout << "Ready" << std::endl;
                //         break;
                //     default:
                //         throw 0;
                // }
            } while(asset_sec.status != AssetStatus::READY);

            auto mdl = asset_sec.asset->as<SahProgramModule::SecondaryAssetT>();
            auto ret = mdl->get_function_address<int(*)()>("bar")();

            std::cout << ret << std::endl;
        });
    }

    for(auto &&t : ts)
        t.join();
}
