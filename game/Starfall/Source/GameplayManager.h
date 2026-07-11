#pragma once
#include "axmol.h"
#include "Core/Pool.h"
#include "Entities/Player.h"
#include "Entities/Bullet.h"

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

    void onKeyDown(ax::EventKeyboard::KeyCode code);
    void onKeyUp(ax::EventKeyboard::KeyCode code);

private:
    ax::Scene* _scene   = nullptr;
    Player* _player = nullptr;
    Pool<Bullet> _bullets;
};
