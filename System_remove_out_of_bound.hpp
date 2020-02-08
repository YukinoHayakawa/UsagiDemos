#pragma once

#include "Type.hpp"

struct System_remove_out_of_bound
{
    using ReadAccess = ComponentFilter<ComponentPosition>;

    template <typename RuntimeServices, typename EntityDatabaseAccess>
    void update(RuntimeServices &&rt, EntityDatabaseAccess &&db)
    {
        for(auto &&e : db.view(ReadAccess()))
        {
            auto &c_pos = USAGI_COMPONENT(e, ComponentPosition);
            if(c_pos.position.y() < 0 || c_pos.position.x() > 1920)
            {
                e.destroy();
            }
        }
    }
};
