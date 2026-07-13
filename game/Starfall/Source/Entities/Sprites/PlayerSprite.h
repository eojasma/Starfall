#pragma once
#include "axmol.h"

class PlayerSprite : public ax::Node
{
public:
    static PlayerSprite* create();

    /// <summary>
    /// initialized the enemy sprite and schedules it to update
    /// </summary>
    /// <returns>true if initialized</returns>
    bool init();

private:
    ax ::Sprite* _sprite = nullptr;
};
