#include "Viper.h"
#include <cstdlib>
#include <cmath>

Viper::Viper(float x, float y, float spd, float fire)
    : Enemy(x, y, 34.f, 30.f, 3),
      sineTimer(0), sineAmplitude(70.f), sineFreq(2.0f), baseX(x)
{
    speedScale = spd;
    fireRateScale = fire;
    velocity = {0.f, 75.f * speedScale};
    fireInterval = 2.2f / fireRateScale;
    fireTimer = (float)(std::rand() % 220) / 100.f;
    buildSprite();
}

void Viper::buildSprite() {
    bodyShape.setPointCount(5);
    bodyShape.setPoint(0, {0, -15});
    bodyShape.setPoint(1, {12, 0});
    bodyShape.setPoint(2, {8,  14});
    bodyShape.setPoint(3, {-8, 14});
    bodyShape.setPoint(4, {-12, 0});
    bodyShape.setFillColor(sf::Color(80, 180, 60));
    bodyShape.setOutlineColor(sf::Color(140, 255, 100));
    bodyShape.setOutlineThickness(1.5f);

    wingL.setPointCount(4);
    wingL.setPoint(0, {0, -5});
    wingL.setPoint(1, {-20, 5});
    wingL.setPoint(2, {-18, 14});
    wingL.setPoint(3, {-5,  10});
    wingL.setFillColor(sf::Color(50, 130, 40));

    wingR.setPointCount(4);
    wingR.setPoint(0, {0,  -5});
    wingR.setPoint(1, {20,  5});
    wingR.setPoint(2, {18, 14});
    wingR.setPoint(3, {5,  10});
    wingR.setFillColor(sf::Color(50, 130, 40));

    eye.setRadius(5.f);
    eye.setOrigin({5.f, 5.f});
    eye.setFillColor(sf::Color(255, 80, 200));
}

void Viper::update(float dt) {
    sineTimer += dt;
    position.y += velocity.y * dt;
    position.x = baseX + sineAmplitude * std::sin(sineFreq * sineTimer);
    // clamp x
    if (position.x < 20) position.x = 20;
    if (position.x > 880) position.x = 880;
    fireTimer -= dt;
    if (position.y > 760.f) destroy();
}

void Viper::draw(sf::RenderWindow& window) {
    bodyShape.setPosition(position);
    wingL.setPosition(position);
    wingR.setPosition(position);
    eye.setPosition({position.x, position.y - 4.f});
    window.draw(wingL);
    window.draw(wingR);
    window.draw(bodyShape);
    window.draw(eye);
}

Bullet* Viper::shoot() {
    if (fireTimer > 0) return nullptr;
    fireTimer = fireInterval;
    // shoot slightly angled toward center of screen
    float dx = 450.f - position.x;
    float len = std::sqrt(dx*dx + 300.f*300.f);
    return new Bullet(position.x, position.y + height/2.f,
                      dx/len * 280.f, 300.f/len * 280.f,
                      1, false, Bullet::Owner::ENEMY);
}
