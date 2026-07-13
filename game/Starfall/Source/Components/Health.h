#pragma once
#include "axmol.h"

struct Entity;
class GameplayManager;

struct HealthComp
{
    int current = 1, max = 1;

    [[nodiscard]]
    bool isDead() const
    {
        return current <= 0;
    }


    void applyDamage(Entity& entity, int damage, GameplayManager& game);
};
