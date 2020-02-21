#pragma once

#include <Usagi/Game/Entity/Archetype.hpp>
#include <Usagi/Game/Database/EntityDatabase.hpp>

namespace usagi
{
struct System_star_render
{
    using WriteAccess = ComponentFilter<>;
    using ReadAccess = ComponentFilter<>;

    template <typename RuntimeServices, typename EntityDatabaseAccess>
    void update(RuntimeServices &&svc, EntityDatabaseAccess &&db)
    {
        for(auto &&e : db.view(ComponentFilter<>()))
        {

        }
    }
};
}
