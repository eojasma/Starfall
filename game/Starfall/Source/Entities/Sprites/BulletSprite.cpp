#include "BulletSprite.h"

USING_NS_AX;

BulletSprite* BulletSprite::create()
{
    BulletSprite* p = new (std::nothrow) BulletSprite();
    if (p && p->init())
    {
        p->autorelease();
        return p;
    }

    AX_SAFE_DELETE(p);
    return nullptr;
}

bool BulletSprite::init()
{
    if (!Node::init())
    {
        return false;
    }

    _sprite = Sprite::create("Sprites/bullet.png");
    AXASSERT(_sprite, "bullet.png missing from Content/");

    this->addChild(_sprite);
    //this->scheduleUpdate();
    return true;
}

