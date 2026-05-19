#pragma once
#include "Enemy.h"
#include <SFML/Graphics.hpp>

class Boss : public Enemy {
public:
    Boss(float x, float y, float w, float h, int hp);
    virtual ~Boss() {}

    virtual void update(float dt) override;
    virtual void draw(sf::RenderWindow& window) override;
    virtual void onCollision(GameObject* other) override;

    // Returns newly spawned bullets – caller owns them
    virtual Bullet** attack(int& count) = 0;
    // Returns newly spawned enemies – caller owns them (Mothership uses this)
    virtual Enemy** spawnEnemies(int& count) { count = 0; return nullptr; }

    void drawHealthBar(sf::RenderWindow& window) const;
    bool isDefeated() const { return hp <= 0; }
    bool isEntering() const { return entering; }

protected:
    float phase;   // generic phase timer
    float attackTimer;
    float attackInterval;
    bool phaseTwo;

    // Entrance animation
    bool entering;
    float entranceTimer;
    float entranceStartY;
    sf::Vector2f targetPosition;

    // Smooth HP display
    mutable float displayHP;

    mutable sf::RectangleShape hpBarBg;
    mutable sf::RectangleShape hpBarFill;
};
