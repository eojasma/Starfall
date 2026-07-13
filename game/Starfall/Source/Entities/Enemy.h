#pragma once
#include "axmol.h"
#include "Entity.h"
#include "Sprites/EnemySprite.h"

/// <summary>
/// struct for holding enemy data, kept in a pool to be reused and avoid instantiating
/// </summary>
struct Enemy : Entity
{
    Enemy()
    {
        transform = TransformComp();
        health    = HealthComp{
               .current = 1,
               .max     = 1,
        };

        render = RenderComp();
        //death  = DeathComp();
    }

    static ax::Node* createSprite() { return EnemySprite::create(); }

    // we dont reset the render here because its a allocated resource, we will resue
    void reset() override
    {
        Entity::reset();

        transform = TransformComp();
        health    = HealthComp{
               .current = 1,
               .max     = 1,
        };
    }
};
