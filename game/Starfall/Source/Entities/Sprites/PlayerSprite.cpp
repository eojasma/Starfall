#include "PlayerSprite.h"

USING_NS_AX;

PlayerSprite* PlayerSprite ::create()
{
    PlayerSprite* p = new (std::nothrow) PlayerSprite();
    if (p && p->init())
    {
        p->autorelease();
        return p;
    }

    AX_SAFE_DELETE(p);
    return nullptr;
}

bool PlayerSprite::init()
{
    if (!Node::init())
    {
        return false;
    }

    _sprite = Sprite::create("Sprites/ship.png");
    AXASSERT(_sprite, "ship.png missing from Content/");

    this->addChild(_sprite);
    this->scheduleUpdate();
    return true;
}
