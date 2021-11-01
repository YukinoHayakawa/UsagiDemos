// This header is to be compiled into a PCH to provide the script source
// with access to the definition of the entity database including the
// definition of components.

// #pragma once

// Part 1: Basic edb definition with component access/filters/etc.

#include <Usagi/Entity/EntityDatabase.hpp>
#include <Usagi/Runtime/Memory/PagedStorage.hpp>
#include <Usagi/Modules/Gameplay/Scripting/ComponentCoroutineContinuation.hpp>
#include <Usagi/Modules/Runtime/Asset/ComponentSecondaryAssetRef.hpp>
#include <Usagi/Modules/Runtime/Executive/DatabaseTraits.hpp>

// Part 2: Component definitions

namespace usagi
{
struct TestComponent1
{
    std::uint64_t num = 0;
};

struct TestComponent2
{
    float flt = 0;
};

using AppDbTraits = DatabaseTraits<
    C<>,
    C<  ComponentCoroutineContinuation,
        ComponentSecondaryAssetRef,
        TestComponent1,
        TestComponent2
    >
>;
}

// Part 3: Script source (in Script.cpp, added as source during JIT compilation)

// Part 4: Template instantiation. The script template is instantiated with
// provided entity database access. todo
