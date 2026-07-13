#pragma once
#include "axmol.h"
#include "Entity.h"
#include "Sprites/BulletSprite.h"

/// <summary>
/// struct for holding bullet data, kept in a pool to be reused and avoid instantiating
/// </summary>
struct Bullet : Entity
{
    Bullet()
    {
        render = RenderComp();
        death  = DeathComp();

        reset();
    }

    static ax::Node* createSprite() { return BulletSprite::create(); }

    //we dont reset the render here because its a allocated resource, we will resue
    void reset() override
    {
        Entity::reset();

        transform = TransformComp();
    }
};
