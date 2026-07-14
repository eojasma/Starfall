#include "GameplayManager.h"
#include <random>
#include <algorithm>
#include <cmath>
#include "Entities/Sprites/BulletSprite.h"
#include "Behaviors/Chaser.h"


USING_NS_AX;

namespace
{
static constexpr int kMaxBullets            = 10;
static constexpr float kBulletVelocity      = 85.0f;
static constexpr float kBulletRadius        = 15.0f; //bullets collision circle

static constexpr int kMaxEnemies            = 100;
static constexpr float kEnemySpawnOffset    = 120.0f; //offest off screen to spawn the enemy, this allows us to spawn with out them popping in
static constexpr float kEnemySpawnTime      = 2.0f;
static constexpr float kEnemySpeed          = 50.0f;
static constexpr float kEnemyRadius         = 32.0f; // enemy collision cirlce
static constexpr float kPad                 = 64.f;  // ~one bullet-sprite beyond the edge

}

GameplayManager::GameplayManager()
    : _bullets(kMaxBullets),
    _enemies(kMaxEnemies),
    _enemyGrid(64),
    _random(32) //this needs to be replaced with a random seed on production, setting it to a set number for deterministic testing
    

{

}


void GameplayManager::setPlayer(Player* playerSprite)
{

    AXASSERT(playerSprite, "player is null");

    _player = playerSprite;
}


void GameplayManager::init(ax::Node* worldNode, ax::Node* hudNode, Vec2 visibleSize)
{
    _worldNode = worldNode;
    _hudNode   = hudNode;

    _worldHome = worldNode->getPosition();

    _player->setPosition(visibleSize / 2.0f);

    //reserve space on init to avoid a memory alloc/ resize mid update
     _bulletsToRemove.reserve(kMaxBullets);
    _enemiesToRemove.reserve(kMaxEnemies);
}

void GameplayManager::pause()
{

}

void GameplayManager::update(const float dt)
{
    auto* director        = Director::getInstance();
    const ax::Vec2 vis    = director->getVisibleSize();
    const ax::Vec2 origin = director->getVisibleOrigin();

    //clear and rebuild the grid at the beiging of the frame this saves us time
    // by only walking the enemy set once here helps our collision loop if there are 100s of enemies
    _enemyGrid.Clear();

    _enemies.forEachActive([this](Enemy& b, std::size_t)
    {  
        _enemyGrid.Insert(&b);
    });

    _player->update(dt);

    const ax::Rect  bounds(origin.x - kPad, origin.y - kPad, vis.width + kPad * 2, vis.height + kPad * 2);

    _bullets.forEachActive([this, &bounds, dt](Bullet& b, std::size_t) {
            this->bulletUpdate(b, bounds,  dt);
        });

    enemySpawnCoolDown -= dt;

     spawnEnemy(bounds);

    _enemies.forEachActive([this, &bounds, dt](Enemy& b, std::size_t) { this->enemyUpdate(b, dt); });

    updateShake(dt);

    //always wait till the end of the update frame to clear remove / release entities. that way we dont change the list during itteration
    for (auto* toRemove : _bulletsToRemove)
    {
        // an active bullet always has a live node (fire() guarantees it),
        // so no null-check here. If that ever breaks, this is where it crashes.
        toRemove->pendingRemoval = false;
        toRemove->render->node->setVisible(false);
        _bullets.release(toRemove);  // safe: release only flips a flag + pushes an index
    }

    for (auto* toRemove : _enemiesToRemove)
    {
        // an active enemy always has a live node (spawnEnemy() guarantees it),
        // so no null-check here. If that ever breaks, this is where it crashes.
        toRemove->pendingRemoval = false;
        toRemove->render->node->setVisible(false);
        _enemies.release(toRemove);  // safe: release only flips a flag + pushes an index
    }

    //clear the cull lists at end of loop 
    _bulletsToRemove.clear();
    _enemiesToRemove.clear();
}


template <typename T> requires std::derived_from<T, Entity>
T* GameplayManager::spawnSprite(Pool<T>& pool, const ax::Vec2 pos) //very important the pool is passed by reference or a bug where the pool is copied and acted on instead
{
    T* entity = pool.acquire();

    if (entity)  // acquire() returns null at cap — dont spawn a mob, don't grow the pool.
    {
        entity->reset();

        if (!entity->render->node)  // first use of this slot; reused slots keep their sprite.
        {
            entity->render->node = T::createSprite();  // per-type factory
            _worldNode->addChild(entity->render->node);
        }

        entity->render->node->stopAllActions();  // kill any lingering death anim and reset visibility
        entity->render->node->setOpacity(255);
        entity->render->node->setScale(1.0f);
        entity->render->node->setVisible(true);
        
        entity->transform->pos.x = pos.x;
        entity->transform->pos.y = pos.y;
        
    }

    return entity;
}

/// <summary>
/// using a spatial grid, allows us to detect enemies the bullets will hit by casting a linesegment along its path
/// </summary>
/// <param name="grid">spatial grid contianing cells of enemies</param>
/// <param name="start">starting position of the bullet</param>
/// <param name="end">end position of the bullet</param>
/// <returns>a vector of enemies that the bullet might have collided with</returns>
std::vector<Enemy*> GetBulletCandidates(const SpatialGrid& grid, Vec2 start, Vec2 end)
{
    // enemy can span multiple cells
    std::unordered_set<Enemy*> uniqueCandidates;

    const Vec2 movement  = end - start;
    const float distance = movement.length();
    
    if (distance == 0.0f)
        return {};

    //we use a half step here to avoid a fast moving bullet from skipping past a cell
    const float stepSize = grid.GetCellSize() * 0.5f;
    const int steps      = std::max(1, static_cast<int>(std::ceil(distance / stepSize)));

    for (int i = 0; i <= steps; ++i)
    {
        const float t = static_cast<float>(i) / static_cast<float>(steps);

        const Vec2 point = start + movement * t;
        const Cell cell  = grid.ToCell(point);

        const auto* enemies = grid.GetCell(cell.x, cell.y);

        if (enemies == nullptr)
            continue;

        for (Enemy* enemy : *enemies)
        {
            if (!enemy->pendingRemoval)
            {
                uniqueCandidates.insert(enemy);
            }
        }
    }

    return {uniqueCandidates.begin(), uniqueCandidates.end()};
}

bool SegmentIntersectsCircle(Vec2 start, Vec2 end, Vec2 circleCenter, float circleRadius)
{
    const Vec2 segment               = end - start;
    const float segmentLengthSquared = segment.lengthSquared();

    const Vec2 circent = (circleCenter - start);

    if (segmentLengthSquared == 0.0f)
    {
        return circent.lengthSquared() <= circleRadius * circleRadius;
    }

    //projecting the bullets circle along the line segment to see if we will collide during this frame
    float t = circent.dot(segment) / segmentLengthSquared; //Dot(circleCenter - start, segment) / segmentLengthSquared;

    t = std::clamp(t, 0.0f, 1.0f);

    const Vec2 closestPoint = start + segment * t;
    const Vec2 difference   = circleCenter - closestPoint;

    return difference.lengthSquared() <= circleRadius * circleRadius;
}

bool Intersects(Vec2 posA, Vec2 posB, float radiusA, float radiusB)//const Circle& a, const Circle& b)
{
    const Vec2 difference      = posB - posA;
    const float combinedRadius = radiusA + radiusB;

    //circle-vs-circle, squared to skip the sqrt
    return difference.lengthSquared() <= combinedRadius * combinedRadius;
}

void GameplayManager::bulletUpdate(Bullet& bullet, const ax::Rect& bounds, const float dt)
{
    Vec2 start = bullet.transform->pos;
    bullet.transform->pos += bullet.transform->vel * dt;

    Vec2 end = bullet.transform->pos;  // position + bullet.velocity* deltaTime;

    bullet.render->node->setPosition(bullet.transform->pos);
    if (!bounds.containsPoint(bullet.transform->pos))
    {
        bullet.pendingRemoval = true;
        _bulletsToRemove.push_back(&bullet);
        return;
    }

    const auto candidates = GetBulletCandidates(_enemyGrid, start, end);

    for (Enemy* enemy : candidates)
    {
        if (enemy->pendingRemoval)
        {
            continue;
        }

        if (SegmentIntersectsCircle(start, end, enemy->transform->pos,
                enemy->transform->radius + bullet.transform->radius))  // include the bullet radius to act as a capsule
        {
            // Bullet hit enemy.
            //TODO: start enemy death animation
            bullet.pendingRemoval = true;
            
            _bulletsToRemove.push_back(&bullet);

            enemy->health->applyDamage(*enemy, 1, *this);

            if (enemy->health->isDead())
            {
                //TODO move all of this to a self contained function
                enemy->pendingRemoval = true;

                auto pop      = ScaleTo::create(0.15f, 1.4f);
                auto fade     = FadeOut::create(0.15f);
                auto puff     = Spawn::create(pop, fade, nullptr);  // both together
                
                const Handle h = _enemies.handleOf(enemy);
                auto done      = CallFunc::create([this, h] {
                    if (Enemy* e = _enemies.resolve(h))  // nullptr if the slot was reused/freed
                        EnemyDeath(*e);                  // only touch it if it's still the same enemy
                });

                enemy->render->node->runAction(Sequence::create(puff, done, nullptr));

                //TODO move these to a pool
                auto boom = ParticleExplosion::create();
                boom->setPosition(enemy->transform->pos);
                boom->setAutoRemoveOnFinish(true);  // frees itself when the last particle dies
                _worldNode->addChild(boom);
                
            }
            else
            {
                auto blink = Blink::create(0.25f, 8);  // 8 flashes over the window
                enemy->render->node->runAction(blink);   // while _vulnState == Invuln
            }
            return;
        }
    }
}

void GameplayManager::firePlayerMainWeapon()
{
    if (_player->canFire())
    {
        Vec2 pos = _player->getPosition();  // this will need to update to +offset for where the guns are.
        
        if (auto* b = spawnSprite(_bullets, pos))
        {
            b->transform->vel = Vec2(0, kBulletVelocity);
            b->transform->radius = kBulletRadius;
            _player->firedMainWeapon();
        }
    }
}

bool GameplayManager::isTimeToSpawnEnemy() const
{
    return 0 > enemySpawnCoolDown;
}

void GameplayManager::enemySpawned()
{
    enemySpawnCoolDown = kEnemySpawnTime;
}

void GameplayManager::spawnEnemy(const ax::Rect& bounds)
{

    if (!isTimeToSpawnEnemy())
        return;

    // spawn random off-screen edge 
    float percent = _random.NextFloat(0, 1);
    int randSide = _random.NextInt(0, 3);

    Vec2 pos = {0.0f, 0.0f}; 

    // 0=left 1=right 2=bottom 3=top, could have been an enum but no real need its just a quick rand
    // using a percent float that we multiply against the max position on a side, this gives us a random point along that side
    switch (randSide)
    {
    case 0:
        pos.x = -kEnemySpawnOffset;
        pos.y = bounds.size.y * percent;
        break;
    case 1:
        pos.x = bounds.size.x + kEnemySpawnOffset;
        pos.y = bounds.size.y * percent;
        break;
    case 2:
        pos.x = bounds.size.x * percent;
        pos.y = -kEnemySpawnOffset;        
        break;
    case 3:
    default:
        pos.x = bounds.size.x * percent;
        pos.y = bounds.size.y + kEnemySpawnOffset;
        break;
    }

    
    if (auto* e = spawnSprite(_enemies, pos))
    {
        e->transform->radius = kEnemyRadius;  // this should come from data based on enemy type.
        enemySpawned();
    }

}

void GameplayManager::enemyUpdate(Enemy& enemy,  const float dt)
{
    if (enemy.pendingRemoval) //dont update corpses, TODO: when we have death anims this will prolly change
    {
        return;
    }

    //TODO: enemy 'ai' loop, for now pure seek
    Chaser::Update(&enemy, _player, kEnemySpeed, dt);

    enemy.render->node->setPosition(enemy.transform->pos);

    if (Intersects(enemy.transform->pos, _player->getPosition(), enemy.transform->radius, _player->getRadius()))
    {
        //player hit
        _player->applyDamage(*_player, 1, *this);

        addTrauma(0.05f);

        if (_player->isDead())
        {
            auto boom = ParticleExplosion::create();
            boom->setPosition(_player->getPosition());
            boom->setAutoRemoveOnFinish(true);  // frees itself when the last particle dies
            _worldNode->addChild(boom);

            _player->PlayerDied();
        }

        

        return;
    }
}

void GameplayManager::EnemyDeath(Enemy& e)
{
    e.render->node->stopAllActions();

    _enemiesToRemove.push_back(&e);
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


void GameplayManager::addTrauma(float t)
{
    _trauma = std::min(1.0f, _trauma + t);
}

void GameplayManager::updateShake(float dt)
{
    if (_trauma <= 0.f)
    {
        _worldNode->setPosition(_worldHome);
        return;
    }

    const float r = _random.NextFloat(-1.f, 1.f);  // one source of randomness
    const float r2 = _random.NextFloat(-1.f, 1.f);  // one source of randomness
    const float r3 = _random.NextFloat(-1.f, 1.f);  // one source of randomness

    const float shake = _trauma * _trauma;                        // square → punchy, tapers soft
    const float ang   = (r * 2.f - 1.f) * 12.f * shake;  // degrees
    const Vec2 off{(r2 * 2.f - 1.f) * 20.f * shake, (r3 * 2.f - 1.f) * 20.f * shake};

    _worldNode->setPosition(_worldHome + off);
    _worldNode->setRotation(ang);
    _trauma = std::max(0.f, _trauma - dt * 1.5f);  // decay per second
}
