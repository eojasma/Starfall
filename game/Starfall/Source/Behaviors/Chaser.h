#pragma once
#include "axmol.h"
#include "Entities/Entity.h"


struct Chaser
{
    static void Update(Entity* entity, Entity* target, float speed, float dt)
    {
        ax::Vec2 dir         = (target->transform->pos - entity->transform->pos).getNormalized();
        entity->transform->vel = dir * speed;
         //kEnemySpeed;
        entity->transform->pos += entity->transform->vel * dt;
    }
};
