#pragma once

#include <Usagi/Module/Common/Math/Matrix.hpp>
#include <Usagi/Game/Entity/Archetype.hpp>
#include <Usagi/Game/Database/EntityDatabase.hpp>

using namespace usagi;

// ============================== Components ================================ //

struct ComponentFireworks
{
    int num_sparks;
    float time_to_explode;
};
static_assert(Component<ComponentFireworks>);

struct ComponentSpark
{
    float fade_time_total;
    float fade_time_left;
    Vector3f base_color;
};
static_assert(Component<ComponentSpark>);

struct ComponentPosition
{
    Vector2f position;
};
static_assert(Component<ComponentPosition>);

struct ComponentPhysics
{
    Vector2f velocity;
    Vector2f acceleration;
};
static_assert(Component<ComponentPhysics>);

struct ComponentSprite
{
    int size;
};
static_assert(Component<ComponentSprite>);

struct ComponentColor
{
    Vector3f rgb;
};
static_assert(Component<ComponentColor>);

// ============================== Archetypes ================================ //

using ArchetypeFireworks = Archetype<
    ComponentFireworks,
    ComponentPosition,
    ComponentPhysics,
    ComponentSprite,
    ComponentColor
>;

using ArchetypeSpark = Archetype<
    ComponentSpark,
    ComponentPosition,
    ComponentPhysics,
    ComponentSprite,
    ComponentColor
>;

// todo infer components from systems
using Database = EntityDatabase<
    ComponentFilter<
        ComponentFireworks,
        ComponentSpark,
        ComponentPosition,
        ComponentPhysics,
        ComponentSprite,
        ComponentColor
    >
>;
