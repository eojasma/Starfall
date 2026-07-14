#include "EnemySprite.h"

USING_NS_AX;

EnemySprite* EnemySprite::create()
{
    EnemySprite* p = new (std::nothrow) EnemySprite();
    if (p && p->init())
    {
        p->autorelease();
        return p;
    }

    AX_SAFE_DELETE(p);
    return nullptr;
}

bool EnemySprite::init()
{
    if (!Node::init())
    {
        return false;
    }

    _sprite = Sprite::create("Sprites/enemy.png");
    AXASSERT(_sprite, "Enemy.png missing from Content/");

    this->addChild(_sprite);
    //this->scheduleUpdate();
    return true;
}
