#pragma once
#include "axmol.h"
#include "VisualEntity.h"
#include "BulletSprite.h"

/// <summary>
/// struct for holding bullet data, kept in a pool to be reused and avoid instantiating
/// </summary>
struct Bullet : VisualEntity
{
    static ax::Node* createSprite() { return BulletSprite::create(); }
};
