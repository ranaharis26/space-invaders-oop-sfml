#include "Drone.h"
#include <cstdlib>

Drone::Drone(float x, float y, float spd, float fire)
    : Enemy(x, y, 32.f, 28.f, 2)
{
    speedScale = spd;
    fireRateScale = fire;
    velocity = {0.f, 90.f * speedScale};
    fireInterval = 2.5f / fireRateScale;
    fireTimer = (float)(std::rand() % 250) / 100.f;
    buildSprite();
}

void Drone::buildSprite() {
    // Body
    bodyShape.setPointCount(6);
    bodyShape.setPoint(0, {0, -14});
    bodyShape.setPoint(1, {10, -5});
    bodyShape.setPoint(2, {10,  10});
    bodyShape.setPoint(3, {0,   14});
    bodyShape.setPoint(4, {-10, 10});
    bodyShape.setPoint(5, {-10, -5});
    bodyShape.setFillColor(sf::Color(180, 60, 60));
    bodyShape.setOutlineColor(sf::Color(255, 100, 80));
    bodyShape.setOutlineThickness(1.5f);

    wingL.setSize({20, 8});
    wingL.setOrigin({20, 4});
    wingL.setFillColor(sf::Color(140, 40, 40));

    wingR.setSize({20, 8});
    wingR.setOrigin({0, 4});
    wingR.setFillColor(sf::Color(140, 40, 40));

    cockpit.setRadius(5.f);
    cockpit.setOrigin({5.f, 5.f});
    cockpit.setFillColor(sf::Color(255, 220, 80));
}

void Drone::update(float dt) {
    Enemy::update(dt);
    if (position.y > 760.f) destroy();
}

void Drone::draw(sf::RenderWindow& window) {
    bodyShape.setPosition(position);
    wingL.setPosition({position.x, position.y});
    wingR.setPosition({position.x, position.y});
    cockpit.setPosition({position.x, position.y - 3.f});
    window.draw(wingL);
    window.draw(wingR);
    window.draw(bodyShape);
    window.draw(cockpit);
}

Bullet* Drone::shoot() {
    if (fireTimer > 0) return nullptr;
    fireTimer = fireInterval;
    return new Bullet(position.x, position.y + height/2.f,
                      0.f, 300.f, 1, false, Bullet::Owner::ENEMY);
}
