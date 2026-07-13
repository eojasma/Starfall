#pragma once

struct Entity;
class GameplayManager;

using OnDeathFn = void (*)(Entity&, GameplayManager&);


struct DeathComp
{
    OnDeathFn onDeath = nullptr;

    void Invoke(Entity& entity, GameplayManager& game) const;
};
