#pragma once
#include "axmol.h"
#include "Core/Pool.h"
#include "Entities/Player.h"
#include "Entities/Bullet.h"
#include "Entities/Enemy.h"
#include "Core/SpatialGrid.h"
#include "Core/Random.h"

class GameplayManager
{
public:
    GameplayManager();

    /// <summary>
    /// sets the player reference so that Gameplay manager can access it
    /// </summary>
    /// <param name="playerSprite"></param>
    void setPlayer(Player* playerSprite);

    /// <summary>
    /// initializes GameplayManager
    /// </summary>
    /// <param name="scene">Gameplay scene</param>
    /// <param name="visibleSize">Visible size of the window</param>
    void init(ax::Scene* scene, ax::Vec2 visibleSize);

    /// <summary>
    /// TODO: pauses the gameplay loop
    /// </summary>
    void pause();

    /// <summary>
    /// main gameplay update loop
    /// </summary>
    /// <param name="dt"></param>
    void update(const float dt);

   
    /// <summary>
    /// updates bullet, position and visibility
    /// </summary>
    /// <param name="b">Bullet to update</param>
    /// <param name="bounds">screen bounds</param>
    /// <param name="dt">delta time since last update</param>
    void bulletUpdate(Bullet& b, const ax::Rect& bounds, const float dt);

    /// <summary>
    /// fires the player main weapon 
    /// </summary>
    void firePlayerMainWeapon();

    /// <summary>
    /// check if we can spawn an enemy now
    /// </summary>
    /// <returns></returns>
    bool isTimeToSpawnEnemy();

    /// <summary>
    /// called when enemy has spawned, starts the cooldown
    /// </summary>
    void enemySpawned();
    void spawnEnemy(const ax::Rect& bounds);
    void enemyUpdate(Enemy& e, const ax::Rect& bounds, const float dt);

    void onKeyDown(ax::EventKeyboard::KeyCode code);
    void onKeyUp(ax::EventKeyboard::KeyCode code);

private:
    ax::Scene* _scene   = nullptr;
    Player* _player = nullptr;

    /// <summary>
    /// preset pool of bullet structs to use during gameplay, number never changes
    /// </summary>
    Pool<Bullet> _bullets;

    
    /// <summary>
    /// preset pool of enemy structs to use during gameplay, number never changes
    /// </summary>
    Pool<Enemy> _enemies;

    /// <summary>
    /// list of bullets to remove at the end of a update loop
    /// we're going to initialize these to sized to the max number
    /// of bullets so we dont have an extra memory pull during the update loop
    /// </summary>
    std::vector<Bullet*> _bulletsToRemove;

    /// <summary>
    /// list of enemies to remove at the end of a update loop
    /// we're going to initialize these to sized to the max number
    /// of enemies so we dont have an extra memory pull during the update loop
    /// </summary>
    std::vector<Enemy*> _enemiesToRemove;

    /// <summary>
    /// this coolddown acts as a timer of sorts between enemy spawns
    /// </summary>
    float enemySpawnCoolDown = 0.0f;

    SpatialGrid _enemyGrid;

    Random _random;

     /// <summary>
     /// spawns a sprite into the gameworld
     /// spawn sprite is private becuase it should never be called outside the manager
     /// 
     /// </summary>
     /// <typeparam name="T">T: A visual Enttity Derrived struct/class</typeparam>
     /// <param name="pool">Pool: a pool of entities to request from</param>
     /// <param name="pos">Pos: position to spawn at</param>
     /// <returns></returns>
     template <typename T>
        requires std::derived_from<T, VisualEntity>
    T* spawnSprite(Pool<T>& pool, const ax::Vec2 pos);

    
};
