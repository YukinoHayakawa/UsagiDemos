
#include <cassert>
#include <future>
#include <iostream>

#include <Usagi/Modules/Common/Time/Clock.hpp>
#include <Usagi/Modules/Runtime/Asset/AssetManager.hpp>
#include <Usagi/Modules/Runtime/Asset/SecondaryAssetConstructor.hpp>
#include <Usagi/Modules/Runtime/Asset/Package/AssetPackageFilesystem.hpp>
#include <Usagi/Modules/Runtime/ProgramModule/ClangJIT.hpp>
#include <Usagi/Modules/Runtime/ProgramModule/RuntimeModule.hpp>
#include <Usagi/Runtime/Task.hpp>
#include <Usagi/Runtime/File/RegularFile.hpp>

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
        std::uint64_t wait_on) override
    {
        std::lock_guard lk(mMutex);

        // find the dependent task
        TaskRef wait_it = mTask.end();
        if(wait_on != INVALID_TASK)
        {
            wait_it = mTask.find(wait_on);
            assert(wait_it != mTask.end());
        }

        auto task_id = ++mTaskId;

        auto future = std::async(
            std::launch::async,
            [t = std::move(task), wait = wait_it, end = mTask.end()]() {
                Clock clk;
                // using namespace std::chrono_literals;
                // std::this_thread::sleep_for(5ms);

                if(wait != end)
                    wait->second.wait();

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

class ScriptJitConstructor : public SecondaryAssetConstructor
{
    static ClangJIT jit;

public:
    AssetCacheSignature signature() override
    {
        return { };
    }

    std::any construct(ReadonlyMemoryRegion primary) override
    {
        std::cout << "construct called" << std::endl;

        RegularFile file { "foo2.pch" };
        MappedFileView view = file.create_view();

        auto compiler = jit.create_compiler();
        compiler.set_pch(view.memory_region());
        compiler.add_source("test", primary);

        auto mdl = compiler.compile();
        assert(mdl);

        return std::shared_ptr(std::move(mdl));
    }
};

ClangJIT ScriptJitConstructor::jit;

int main(int argc, char *argv[])
{
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
            SecondaryAsset asset_sec = asset_manager.secondary_asset(
                "test.cpp",
                std::make_unique<ScriptJitConstructor>(),
                &executor
            );
            do
            {
                // asset = asset_manager.primary_asset(argv[1], &executor);
                asset_sec = asset_manager.secondary_asset(asset_sec.signature);
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
            } while(asset_sec.status != AssetStatus::SECONDARY_READY);

            auto mdl = std::any_cast<std::shared_ptr<RuntimeModule>>(
                asset_sec.object
            );
            auto ret = mdl->get_function_address<int()>("bar")();

            std::cout << ret << std::endl;
        });
    }

    for(auto &&t : ts)
        t.join();
}
