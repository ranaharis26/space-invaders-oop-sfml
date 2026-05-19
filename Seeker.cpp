#include "Seeker.h"

Seeker::Seeker(float x, float y, float targetX, float spd)
    : Enemy(x, y, 28.f, 28.f, 1),
      lockedX(targetX), accel(180.f), currentSpeed(50.f)
{
    speedScale = spd;
    fireInterval = 9999.f; // never fires
    buildSprite();
}

void Seeker::buildSprite() {
    bodyShape.setPointCount(4);
    bodyShape.setPoint(0, {0, -14});
    bodyShape.setPoint(1, {10, 6});
    bodyShape.setPoint(2, {0,  14});
    bodyShape.setPoint(3, {-10, 6});
    bodyShape.setFillColor(sf::Color(255, 140, 0));
    bodyShape.setOutlineColor(sf::Color(255, 220, 60));
    bodyShape.setOutlineThickness(1.5f);

    core.setRadius(6.f);
    core.setOrigin({6.f, 6.f});
    core.setFillColor(sf::Color(255, 255, 100));

    thruster.setSize({8.f, 12.f});
    thruster.setOrigin({4.f, 0.f});
    thruster.setFillColor(sf::Color(255, 60, 0));
}

void Seeker::update(float dt) {
    currentSpeed += accel * speedScale * dt;
    position.x = lockedX;          // locked X on spawn
    position.y += currentSpeed * dt;
    fireTimer -= dt;
    if (position.y > 760.f) destroy();
}

void Seeker::draw(sf::RenderWindow& window) {
    bodyShape.setPosition(position);
    core.setPosition({position.x, position.y});
    thruster.setPosition({position.x, position.y + 10.f});
    window.draw(thruster);
    window.draw(bodyShape);
    window.draw(core);
}

void Seeker::onCollision(GameObject* other) {
    destroy(); // destroys on contact with player
}
