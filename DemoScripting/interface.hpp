// This header is to be compiled into a PCH to provide the script source
// with access to the definition of the entity database including the
// definition of components. It's also ok to use this for main.

#include <Usagi/Modules/Runtime/Executive/AppHost.hpp>
#include <Usagi/Modules/Runtime/Executive/SystemTaskList.hpp>
#include <Usagi/Modules/Runtime/Executive/ServiceAsyncWorker.hpp>
#include <Usagi/Modules/Runtime/Executive/ServiceStateTransitionGraph.hpp>

using namespace usagi;

#include <Usagi/Modules/Gameplay/Scripting/SystemInvokeScriptCoroutine.hpp>

struct Services
    : ServiceStateTransitionGraph
    , ServiceAsyncWorker
    , ServiceJitCompilation
    // Asset manager must be put at the last because assets may refer to
    // other services.
    , ServiceAssetManager
{
    Services()
    {
    }
};

using TaskList = SystemTaskList<
    SystemInvokeScriptCoroutine
>;

struct TestComponent1
{
    std::uint64_t num = 0;
};

struct TestComponent2
{
    float flt = 0;
};

// Ensure the database type is defined and all components are accessible
// by the script.
using App = AppHost<
    Services,
    TaskList,
    // The followings were not included by the script executor due to its
    // declared component access.
    C<  ComponentCoroutineContinuation,
        ComponentSecondaryAssetRef,
        ComponentScript,
        ComponentScriptPCH,
        TestComponent1,
        TestComponent2
    >
>;
