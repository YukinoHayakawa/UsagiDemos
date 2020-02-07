#pragma once

#include <Usagi/Math/Matrix.hpp>
#include <Usagi/Experimental/v2/Game/Entity/Archetype.hpp>
#include <Usagi/Experimental/v2/Game/Database/EntityDatabase.hpp>
#include <Usagi/Experimental/v2/Game/_detail/ComponentFilter.hpp>

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
    float brightness;
    float fade_speed;
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
    int r, g, b;
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
    16,
    ComponentFilter<
        ComponentFireworks,
        ComponentSpark,
        ComponentPosition,
        ComponentPhysics,
        ComponentSprite,
        ComponentColor
    >
>;
