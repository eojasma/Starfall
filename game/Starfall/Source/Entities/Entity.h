#pragma once

#include <unordered_map>
#include <string>


#include "axmol.h"

#include "Components/Transform.h"
#include "Components/Health.h"
#include "Components/Render.h"
#include "Components/Weapon.h"
#include "Components/Death.h"

/// <summary>
/// struct for holding visual entity data, kept in a pool to be reused and avoid instantiating
/// use composition over inheritance, inheritance should only be one level deep from the entity, adding components
/// </summary>
struct Entity
{
    /// <summary>
    /// holds positional data, velocity, bounding cirlce radius
    /// </summary>
    std::optional<TransformComp> transform = std::nullopt;

    /// <summary>
    /// holds health max/current
    /// </summary>
    std::optional<HealthComp> health       = std::nullopt;

    /// <summary>
    /// holds the render node if a visual entity
    /// </summary>
    std::optional<RenderComp> render       = std::nullopt;

    /// <summary>
    /// Optional components: present-or-not modelled by std::optional or a flag.std::optional<Weapon> weapon;
    /// only the player has one, for now
    /// </summary>
    std::optional<WeaponComp> weapon = std::nullopt;

    /// <summary>
    /// optional death component, handles how to die
    /// </summary>
    std::optional<DeathComp> death = std::nullopt;

    /// <summary>
    /// is the entity active, used for culling non active entities from the update loops
    /// </summary>
    bool active         = false;

    /// <summary>
    /// flag marking for removal, updates will not fire against these entities and the update loop will use this to send back to the pool at the end of the loop
    /// </summary>
    bool pendingRemoval = false;

    virtual void reset()
    {
        pendingRemoval = false;
    }

    
};
