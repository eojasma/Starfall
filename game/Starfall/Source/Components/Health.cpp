#include "Health.h"
#include "Entities/Entity.h"



void HealthComp::applyDamage(Entity& entity, int damage, GameplayManager& game)
{
    // no reason to apply to a corpse
    if (!entity.active || entity.pendingRemoval || isDead())
    {
        return;
    }

    current = std::max(0, current - damage);

    if (!isDead())  // if it still has health work is done return
    {
        return;
    }

    /*if (entity.death.has_value())
    {
        entity.death->Invoke(entity, game);
    }*/

    entity.pendingRemoval = true;
}
