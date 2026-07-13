#pragma once
#include "axmol.h"
#include "VisualEntity.h"
#include "EnemySprite.h"

/// <summary>
/// struct for holding enemy data, kept in a pool to be reused and avoid instantiating
/// </summary>
struct Enemy : VisualEntity
{
    static ax::Node* createSprite() { return EnemySprite::create(); }
};
