#pragma once
#include "axmol.h"
#include <unordered_set>
#include "Entity.h"

class Player :public Entity
{
public:
    Player();
    
    void reset() override;

    bool init();
    void update(float dt);

    const float getRadius() const;
    const ax::Vec2 getPosition() const;
    void setPosition(const ax::Vec2 pos);

    ax::Node* getRenderNode();

    

    void applyDamage(Player& entity, int damage, GameplayManager& game);

    [[nodiscard]]
    bool isDead() const;

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
    bool canFire() const;

    /// <summary>
    /// sets the fire cool down
    /// </summary>
    void firedMainWeapon();

    void PlayerDied();

    bool canBeDamaged() const;

    void onKeyDown(ax::EventKeyboard::KeyCode code);
    void onKeyUp(ax::EventKeyboard::KeyCode code);

private:
   
    enum class ShipState
    {
        Alive,
        Dead
    };

    enum class VulnState
    {
        Open,
        Invuln
    };

    ShipState _state = ShipState::Alive;
    VulnState _vulnState = VulnState::Open;

    /// <summary>
    /// holds the currently held keys
    /// </summary>
    std::unordered_set<ax::EventKeyboard::KeyCode> _held;

    

    /// <summary>
    /// cooldow time beween shots
    /// </summary>
    float _mainWeaponCoolDown = 0.0f;

    float _vulnStateCoolDown = 0.0f;

};
