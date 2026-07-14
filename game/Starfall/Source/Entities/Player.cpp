#include "Player.h"
#include "Sprites/PlayerSprite.h"

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

constexpr int kMaxPlayerHealth = 10.0f;

constexpr float kIFrameWindow = 0.50f;
}


Player::Player()
{
    reset();
}

void Player::reset() 
{
    Entity::reset();
    transform = TransformComp
    {
        .radius = kPlayerRadius

    };

    health    = HealthComp
    {
        .current = kMaxPlayerHealth,
        .max     = kMaxPlayerHealth,
    };
    weapon = WeaponComp();
    render = RenderComp();
    // death  = DeathComp();

    _state = ShipState::Alive;
    _vulnState = VulnState::Open;
    _vulnStateCoolDown = 0.0f;
}

bool Player::init()
{
    render->node =  PlayerSprite::create();
    
    return true;
}

ax::Node* Player::getRenderNode()
{
    return render->node;
}

const float Player::getRadius() const
{
    return transform->radius;
}

const ax::Vec2 Player::getPosition() const
{
    return transform->pos;
}

void Player::setPosition(const ax::Vec2 pos)
{
    transform->pos.x = pos.x;
    transform->pos.y = pos.y;

    render->node->setPosition(pos);
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

    if (0 < _vulnStateCoolDown)
    {
        _vulnStateCoolDown -= dt;

        if (0 >= _vulnStateCoolDown)
        {
            _vulnState = VulnState::Open;
            //sometimes the actions arent done playing at this point and the player is invis, reset when we become vulnerable again
            render->node->stopAllActions();
            render->node->setOpacity(255);
            render->node->setScale(1.0f);
            render->node->setVisible(true);
        }
    }


    const Vec2 input = readInputDirection();

    if (!input.isZero())
    {
        transform->vel += input * kAccel * dt;  // velocity = speed *direction, veloctiy * deltatime
    }
    else
    {
        const float speed = transform->vel.length();
        if (speed > 0)
        {
            const float drop = kFriction * dt;
            transform->vel        = transform->vel * (std::max(0.0f, speed - drop) / speed);
        }
    }

    if (transform->vel.length() > kMaxSpeed)
    {
        transform->vel.normalize();
        transform->vel = transform->vel * kMaxSpeed;
    }

    Vec2 pos = getPosition() + transform->vel * dt;

    clampToScreen(pos);
    setPosition(pos);
}


bool Player::canFire() const
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


bool Player::canBeDamaged() const
{
    return _vulnState == VulnState::Open;
}

void Player::applyDamage(Player& entity, int damage, GameplayManager& game)
{
    //"iframe" invulnerable for a short time after damage so that the players health
    // doesnt drain on contact in a couple frames
    if (canBeDamaged())
    {
        health->applyDamage(entity, damage, game);

        _vulnState         = VulnState::Invuln;
        _vulnStateCoolDown = kIFrameWindow;

        auto blink = Blink::create(kIFrameWindow, 8);  // 8 flashes over the window
        render->node->runAction(blink);                // while _vulnState == Invuln
    }
}



[[nodiscard]]
bool Player::isDead() const
{
    return health->isDead();
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
    const auto half = render->node->getContentSize() / 2.0f;

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
