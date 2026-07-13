#include "Player.h"
USING_NS_AX;

namespace
{
/// <summary>
/// px/sec
/// </summary>
constexpr float kMaxSpeed = 340.0f;

/// <summary>
/// px/sec^2 
/// </summary>
constexpr float kAccel    = 2400.0f;
/// <summary>
/// px/sec^2 
/// </summary>
constexpr float kFriction = 1800.0f;

/// <summary>
/// px/sec
/// </summary>
constexpr float kPlayerFireInterval = 0.45f;
constexpr float kPlayerRadius       = 32.0f;
}

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

    _radius = kPlayerRadius;

    _sprite = Sprite::create("Sprites/ship.png");
    AXASSERT(_sprite, "ship.png missing from Content/");

    this->addChild(_sprite);
    this->scheduleUpdate();
    return true;
}


const float Player::getRadius() const
{
    return _radius;
}

void Player::update(float dt)
{
    if (_state != ShipState::Alive)
    {
        return;
    }

    if (0 < _mainWeaponCoolDown)
    {
        _mainWeaponCoolDown -= dt;
    }

    const Vec2 input = readInputDirection();

    if (!input.isZero())
    {
        _velocity += input * kAccel * dt;  // velocity = speed *direction, veloctiy * deltatime
    }
    else
    {
        const float speed = _velocity.length();
        if (speed > 0)
        {
            const float drop = kFriction * dt;
            _velocity        = _velocity * (std::max(0.0f, speed - drop) / speed);
        }
    }

    if (_velocity.length() > kMaxSpeed)
    {
        _velocity.normalize();
        _velocity = _velocity * kMaxSpeed;
    }

    Vec2 pos = getPosition() + _velocity * dt;

    clampToScreen(pos);
    setPosition(pos);
}


bool Player::canFire()
{
    return _mainWeaponCoolDown <= 0;
}

void Player::firedMainWeapon()
{
    _mainWeaponCoolDown = kPlayerFireInterval;
}

void Player::PlayerDied()
{
    _state = ShipState::Dead;
}

ax::Vec2 Player::readInputDirection() const
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

    dir.normalize();

    return dir;
}

void Player::clampToScreen(ax::Vec2& pos)
{
    auto* director  = Director::getInstance();
    const auto vs   = director->getVisibleSize();
    const auto half = _sprite->getContentSize() / 2.0f;

    pos.x = clampf(pos.x, half.width, vs.width - half.width); //this seems wrong,
    pos.y = clampf(pos.y, half.height, vs.height - half.height);
}

void Player::onKeyDown(ax::EventKeyboard::KeyCode code)
{
    _held.insert(code);
}

void Player::onKeyUp(ax::EventKeyboard::KeyCode code)
{
    _held.erase(code);
}
