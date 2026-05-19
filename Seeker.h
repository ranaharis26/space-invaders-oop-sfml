#pragma once
#include "Enemy.h"

class Seeker : public Enemy {
public:
    Seeker(float x, float y, float targetX, float speedScale = 1.0f);
    virtual ~Seeker() {}

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void onCollision(GameObject* other) override;
    // Seekers don't shoot
    Bullet* shoot() override { return nullptr; }

private:
    float lockedX;
    float accel;
    float currentSpeed;

    sf::ConvexShape bodyShape;
    sf::CircleShape core;
    sf::RectangleShape thruster;
    void buildSprite();
};
