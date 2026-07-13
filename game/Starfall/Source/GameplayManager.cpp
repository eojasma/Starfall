#include "GameplayManager.h"
#include <random>
#include <algorithm>
#include <cmath>
#include "Entities/BulletSprite.h"

USING_NS_AX;

namespace
{
static constexpr int kMaxBullets            = 10;
static constexpr float kBulletVelocity      = 85.0f;
static constexpr float kBulletRadius        = 15.0f;

static constexpr int kMaxEnemies            = 10;
static constexpr float kEnemySpawnOffset    = 120.0f;
static constexpr float kEnemySpawnTime      = 5.0f;
static constexpr float kEnemySpeed          = 50.0f;
static constexpr float kEnemyRadius         = 32.0f;

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


void GameplayManager::init(ax::Scene* scene, Vec2 visibleSize)
{
    _scene = scene;
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
    auto* director       = Director::getInstance();
    const ax::Vec2 vis    = director->getVisibleSize();
    const ax::Vec2 origin = director->getVisibleOrigin();

    //clear and rebuild the grid at the beiging of the frame this saves us time
    // by only walking the enemy set once here helps our collision loop if there are 100s of enemies
    _enemyGrid.Clear();

    _enemies.forEachActive([this](Enemy& b, std::size_t)
    {  
        _enemyGrid.Insert(&b);
    });

    constexpr float kPad = 64.f;  // ~one bullet-sprite beyond the edge
    const ax::Rect  bounds(origin.x - kPad, origin.y - kPad, vis.width + kPad * 2, vis.height + kPad * 2);

    _bullets.forEachActive([this, &bounds, dt](Bullet& b, std::size_t) {
            this->bulletUpdate(b, bounds,  dt);
        });

    enemySpawnCoolDown -= dt;

     spawnEnemy(bounds);

    _enemies.forEachActive([this, &bounds, dt](Enemy& b, std::size_t) { this->enemyUpdate(b, bounds, dt); });

    //always wait till the end of the update frame to clear remove / release entities. that way we dont change the list during itteration
    for (auto* toRemove : _bulletsToRemove)
    {
        // an active bullet always has a live node (fire() guarantees it),
        // so no null-check here. If that ever breaks, this is where it crashes.
        toRemove->pendingRemoval = false;
        toRemove->node->setVisible(false);
        _bullets.release(toRemove);  // safe: release only flips a flag + pushes an index
    }

    for (auto* toRemove : _enemiesToRemove)
    {
        // an active enemy always has a live node (spawnEnemy() guarantees it),
        // so no null-check here. If that ever breaks, this is where it crashes.
        toRemove->pendingRemoval = false;
        toRemove->node->setVisible(false);
        _enemies.release(toRemove);  // safe: release only flips a flag + pushes an index
    }

    //clear the cull lists at end of loop 
    _bulletsToRemove.clear();
    _enemiesToRemove.clear();
}


template <typename T> requires std::derived_from<T, VisualEntity>
T* GameplayManager::spawnSprite(Pool<T>& pool, const ax::Vec2 pos) //very important the pool is passed by reference or a bug where the pool is copied and acted on instead
{
    T* entity = pool.acquire();

    if (entity)  // acquire() returns null at cap — dont spawn a mob, don't grow the pool.
    {
        if (!entity->node)  // first use of this slot; reused slots keep their sprite.
        {
            entity->node = T::createSprite();  // per-type factory
            _scene->addChild(entity->node);
        }

        entity->node->setVisible(true);
        
        entity->pos.x = pos.x;
        entity->pos.y = pos.y;
        
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
    Vec2 start = bullet.pos;
    bullet.pos += bullet.vel * dt;

    Vec2 end = bullet.pos; //position + bullet.velocity* deltaTime;

    bullet.node->setPosition(bullet.pos);
    if (!bounds.containsPoint(bullet.pos))
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

        if (SegmentIntersectsCircle(start, end, enemy->pos, enemy->radius + bullet.radius)) //include the bullet radius to act as a capsule
        {
            // Bullet hit enemy.
            //TODO: start enemy death animation
            bullet.pendingRemoval = true;
            enemy->pendingRemoval = true;
            _bulletsToRemove.push_back(&bullet);
            _enemiesToRemove.push_back(enemy);
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
            b->vel = Vec2(0, kBulletVelocity);
            b->radius = kBulletRadius;
            _player->firedMainWeapon();
        }
    }
}



bool GameplayManager::isTimeToSpawnEnemy()
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
        e->radius = kEnemyRadius; //this should come from data based on enemy type.
        enemySpawned();
    }

}

void GameplayManager::enemyUpdate(Enemy& enemy, const ax::Rect& bounds,  const float dt)
{
    if (enemy.pendingRemoval) //dont update corpses, TODO: when we have death anims this will prolly change
    {
        return;
    }

    //TODO: enemy 'ai' loop, for now pure seek
    ax::Vec2 dir = (_player->getPosition() - enemy.pos).getNormalized();
    enemy.vel = dir * kEnemySpeed;
    enemy.pos += enemy.vel * dt;
    
    enemy.node->setPosition(enemy.pos);
   /* if (!bounds.containsPoint(enemy.pos))
    {
        enemy.pendingRemoval = true;
        _enemiesToRemove.push_back(&enemy);
        return;
    }*/

    if (Intersects(enemy.pos, _player->getPosition(), enemy.radius, _player->getRadius()))
    {
        //player hit
        //TODO handle player collision
        _player->PlayerDied();

        return;
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
