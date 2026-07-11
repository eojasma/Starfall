#pragma once
#include "axmol.h"

class BulletSprite : public ax::Node
{
public:
    static BulletSprite* create();

    /// <summary>
    /// initialized the bullet sprite and schedules it to update
    /// </summary>
    /// <returns>true if initialized</returns>
    bool init();


private:
    ax ::Sprite* _sprite = nullptr;
};
