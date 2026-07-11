#include "GameplayManager.h"
#include "Entities/BulletSprite.h"

USING_NS_AX;

namespace
{
static constexpr int kMaxBullets           = 10;
static constexpr float kBulletVelocity    = 85.0f;
}

GameplayManager::GameplayManager() : _bullets(kMaxBullets)
{

}


void GameplayManager::setPlayer(Player* playerSprite)
{
    if (!playerSprite)
    {
        throw;
    }

    _player = playerSprite;
}


void GameplayManager::init(ax::Scene* scene, Vec2 visibleSize)
{
    _scene = scene;
    _player->setPosition(visibleSize / 2.0f);
}

void GameplayManager::pause()
{

}

void GameplayManager::update(const float dt)
{
    auto* director       = Director::getInstance();
    const ax::Vec2 vis    = director->getVisibleSize();
    const ax::Vec2 origin = director->getVisibleOrigin();

    constexpr float kPad = 64.f;  // ~one bullet-sprite beyond the edge
    const ax::Rect  bounds(origin.x - kPad, origin.y - kPad, vis.width + kPad * 2, vis.height + kPad * 2);

    _bullets.forEachActive([this, &bounds, dt](Bullet& b, std::size_t) {
            this->bulletUpdate(b, bounds,  dt);
        });
}

void GameplayManager::bulletUpdate(Bullet& b, const ax::Rect& bounds, const float dt)
{
    b.pos += b.vel * dt;

    b.node->setPosition(b.pos);
    if (!bounds.containsPoint(b.pos))
    {
        b.node->setVisible(false);
        _bullets.release(&b);  // safe: release only flips a flag + pushes an index
    }
       
}

void GameplayManager::firePlayerMainWeapon()
{
    if (_player->canFire())
    {
        _player->firedMainWeapon();
        auto* b  = _bullets.acquire();

        if (b) //make sure we actually got a bullet
        {
            if (!b->node)
            {
                b->node = BulletSprite::create();
                _scene->addChild(b->node);
            }

            b->node->setVisible(true);
            Vec2 pos = _player->getPosition(); //this will need to update to +offset for where the guns are.

            b->pos.x  = pos.x;
            b->pos.y  = pos.y;
            b->vel    = Vec2(0, kBulletVelocity);
        }
    }
}

void GameplayManager::onKeyDown(ax::EventKeyboard::KeyCode code)
{
    using KC = EventKeyboard::KeyCode;

    if (code == (KC::KEY_SPACE))
    {
        firePlayerMainWeapon();
    }

    _player->onKeyDown(code);
}

void  GameplayManager::onKeyUp(ax::EventKeyboard::KeyCode code)
{
    _player->onKeyUp(code);
}
