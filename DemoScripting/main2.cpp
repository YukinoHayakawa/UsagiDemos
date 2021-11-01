#define _CRT_SECURE_NO_WARNINGS

#include <future>
#include <map>
#include <iostream>

#include <Usagi/Runtime/Task.hpp>
#include <Usagi/Runtime/TaskExecutor.hpp>
#include <Usagi/Runtime/Service/SimpleService.hpp>
#include <Usagi/Modules/Runtime/Executive/AppHost.hpp>
#include <Usagi/Modules/Runtime/Executive/SystemTaskList.hpp>
#include <Usagi/Modules/Common/Math/ComponentRegion2D.hpp>
#include <Usagi/Modules/Common/Time/Clock.hpp>
#include <Usagi/Modules/Runtime/Asset/Package/AssetPackageFilesystem.hpp>
#include <Usagi/Modules/Runtime/Executive/ServiceStateTransitionGraph.hpp>

#include "Database.hpp"

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

using ServiceAsyncWorker = SimpleService<StdTaskExecutor>;

#include <Usagi/Modules/Gameplay/Scripting/SystemInvokeScriptCoroutine.hpp>

struct Services
    : ServiceStateTransitionGraph
    , ServiceAsyncWorker
    , ServiceAssetManager
    , ServiceJitCompilation
{
    Services()
    {
    }
};

using TaskList = SystemTaskList<
    SystemInvokeScriptCoroutine
>;

using App = AppHost<
    Services,
    TaskList,
    // the following two were not included by the script executioner
    C<  ComponentCoroutineContinuation,
        ComponentSecondaryAssetRef,
        TestComponent1,
        TestComponent2
    >
>;

using Components = App::EnabledComponents;

static_assert(std::is_same_v<Components, AppDbTraits::EnabledComponents>);

int main(int argc, char *argv[])
{
    const auto data_folder = "demo_script";
    std::filesystem::remove_all(data_folder);

    App app { data_folder };
    auto &svc = app.services();
    auto &db = app.database_world();

    // allow SystemInvokeScriptCoroutine to find the script from current folder
    auto &assets = USAGI_SERVICE(svc, ServiceAssetManager);
    assets.add_package(std::make_unique<AssetPackageFilesystem>("."));

    Archetype<ComponentSecondaryAssetRef, ComponentCoroutineContinuation>
        script_archetype;
    script_archetype.component<ComponentCoroutineContinuation>().resume_condition =
        ComponentCoroutineContinuation::NEXT_FRAME;

    db.insert(script_archetype);

    auto &tg = USAGI_SERVICE(app.services(), ServiceStateTransitionGraph);
    while(!tg.should_exit)
    {
        app.update();
    }
}
