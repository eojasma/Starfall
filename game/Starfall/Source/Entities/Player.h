#pragma once
#include "axmol.h"
#include <unordered_set>

class Player : public ax::Node
{
public:
    static Player* create();

    bool init();
    void update(float dt);

    /// <summary>
    /// reads input keys held and gets a direction vector
    /// </summary>
    /// <returns>direction to move</returns>
    ax::Vec2 readInputDirection() const;

    /// <summary>
    /// locks the player position within the screen boundries
    /// </summary>
    /// <param name="pos"></param>
    void clampToScreen(ax::Vec2& pos);

    /// <summary>
    /// checks if the player can fire
    /// </summary>
    /// <returns>true if player can fire main weapon</returns>
    bool canFire();

    /// <summary>
    /// sets the fire cool down
    /// </summary>
    void firedMainWeapon();

    void onKeyDown(ax::EventKeyboard::KeyCode code);
    void onKeyUp(ax::EventKeyboard::KeyCode code);

private:
    ax ::Sprite* _sprite = nullptr;

    enum class ShipState
    {
        Alive,
        Dead
    };

    ShipState _state = ShipState::Alive;

    /// <summary>
    /// holds the currently held keys
    /// </summary>
    std::unordered_set<ax::EventKeyboard::KeyCode> _held;

    ax::Vec2 _velocity = { 0.0f, 0.0f};

    /// <summary>
    /// cooldow time beween shots
    /// </summary>
    float _mainWeaponCoolDown = 0.0f;

};
