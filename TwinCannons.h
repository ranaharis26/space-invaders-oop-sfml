#pragma once
#include "Boss.h"
#include "Turret.h"
#include <SFML/Graphics.hpp>

class TwinCannons : public Boss {
public:
    TwinCannons(float screenW);
    virtual ~TwinCannons() {}

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void onCollision(GameObject* other) override;
    Bullet** attack(int& count) override;

    // Called by CollisionManager to damage the correct part
    void hitByBullet(int dmg);

    Turret* getLeftTurret() { return &leftTurret; }
    Turret* getRightTurret() { return &rightTurret; }

    // Which part is currently the target (for hitbox routing)
    sf::FloatRect getActiveHitbox() const;

    bool coreIsVulnerable() const {
        return leftTurret.isDestroyed() && rightTurret.isDestroyed();
    }

private:
    Turret leftTurret;
    Turret rightTurret;
    float  screenWidth;
    float  sweepSpeed;
    bool   movingRight;

    sf::RectangleShape coreRect;
    sf::RectangleShape coreGlow;

    void buildSprite();
    void updateTurretPositions();
};