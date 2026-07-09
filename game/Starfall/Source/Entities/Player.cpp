#include "Player.h"
USING_NS_AX;

Player* Player::create()
{
    Player* p = new (std::nothrow) Player();
    if (p && p->init())
    {
        p->autorelease();
        return p;
    }

    AX_SAFE_DELETE(p);
    return nullptr;
}

bool Player::init()
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

void Player::update(float dt)
{
    using KC = EventKeyboard::KeyCode;

    Vec2 dir{0.0f, 0.0f};

    auto down = [this](KC k) { return _held.count(k) > 0; };

    if (down(KC::KEY_W) || down(KC::KEY_DPAD_UP))
    {
        dir.y += 1.0f;
    }
    if (down(KC::KEY_A) || down(KC::KEY_DPAD_LEFT))
    {
        dir.x -= 1.0f;
    }
    if (down(KC::KEY_S) || down(KC::KEY_DPAD_DOWN))
    {
        dir.y -= 1.0f;
    }
    if (down(KC::KEY_D) || down(KC::KEY_DPAD_RIGHT))
    {
        dir.x += 1.0f;
    }

    if (!dir.isZero())
    {
        auto* director = Director::getInstance();
        
        dir.normalize();

        Vec2 pos = getPosition() + dir*_speed * dt; //velocity = speed *direction, veloctiy * deltatime

        const auto vs = director->getVisibleSize();
        const auto half = _sprite->getContentSize() / 2.0f;

        pos.x = clampf(pos.x, half.width, vs.width - half.width);
        pos.y = clampf(pos.y, half.height, vs.height - half.height);

        setPosition(pos);
    }
}

void Player::onKeyDown(ax::EventKeyboard::KeyCode code)
{
    _held.insert(code);
}

void Player::onKeyUp(ax::EventKeyboard::KeyCode code)
{
    _held.erase(code);
}
