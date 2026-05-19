#pragma once
#include "Enemy.h"
#include <cmath>

class Viper : public Enemy {
public:
    Viper(float x, float y, float speedScale = 1.0f, float fireRateScale = 1.0f);
    virtual ~Viper() {}

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    Bullet* shoot() override;

private:
    float sineTimer;
    float sineAmplitude;
    float sineFreq;
    float baseX;

    sf::ConvexShape bodyShape;
    sf::ConvexShape wingL, wingR;
    sf::CircleShape eye;
    void buildSprite();
};
