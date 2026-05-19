#pragma once
#include "Entity.h"
#include "Bullet.h"
#include <SFML/Graphics.hpp>

enum class WeaponType { STANDARD, SPREAD, PIERCING };

class Player : public Entity {
public:
    static const float SPEED;
    static const float DASH_DISTANCE;
    static const float DASH_COOLDOWN;
    static const float INVINCIBLE_AFTER_DASH;
    static const float SHOOT_COOLDOWN;

    Player(float x, float y);
    virtual ~Player() {}

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void onCollision(GameObject* other) override;

    Bullet** shoot(int& count);
    void takeDamage(int dmg);

    int getLives() const { return lives; }
    int getEMPs() const { return empCount; }
    int getShieldHits() const { return shieldHits; }
    WeaponType getWeapon() const { return weapon; }
    bool isDead() const { return lives <= 0; }
    bool isInvincible() const { return invincibleTimer > 0; }
    bool wasHitThisFrame() const { return hitThisFrame; }
    void clearHitFlag() { hitThisFrame = false; }
    float getDashCooldownRatio() const;

    void collectWeapon(WeaponType w);
    void collectShield();
    void collectEMP();
    void useEMP();

private:
    int lives;
    int empCount;
    int shieldHits;
    WeaponType weapon;

    float shootTimer;
    float dashCooldown;
    float invincibleTimer;
    float flashTimer;
    bool hitThisFrame;

    sf::ConvexShape shipShape;
    sf::ConvexShape engineShape;
    sf::RectangleShape weaponBarrel;
    sf::CircleShape shieldShape;

    void buildSprite();
    void drawShield(sf::RenderWindow& window);
};
