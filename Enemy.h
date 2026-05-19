#pragma once
#include "Entity.h"
#include "Bullet.h"
#include <SFML/Graphics.hpp>

class Enemy : public Entity {
public:
    Enemy(float x, float y, float w, float h, int hp);
    virtual ~Enemy() {}

    virtual void update(float dt) override;
    virtual void draw(sf::RenderWindow& window) override {}
    virtual void onCollision(GameObject* other) override;
    virtual Bullet* shoot() { return nullptr; }

    int getHP() const { return hp; }
    int getMaxHP() const { return maxHP; }
    void takeDamage(int dmg);
    bool justDied() const { return diedThisFrame; }
    void clearDiedFlag() { diedThisFrame = false; }

    // Drop chances
    float getDropChance() const { return 0.15f; }
    float getEMPDropChance() const { return 0.05f; }

    void applySpeedScale(float s) { speedScale = s; }
    void applyFireRateScale(float s) { fireRateScale = s; }

protected:
    int hp, maxHP;
    float fireTimer;
    float fireInterval;
    float speedScale;
    float fireRateScale;
    bool diedThisFrame;
};
