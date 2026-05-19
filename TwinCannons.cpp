#include "TwinCannons.h"
#include <cmath>
#include <cstdint>

TwinCannons::TwinCannons(float screenW)
    : Boss(screenW / 2.f, 120.f, 100.f, 50.f, 30),   // core has 30 HP
    leftTurret(screenW / 2.f - 90.f, 135.f),
    rightTurret(screenW / 2.f + 90.f, 135.f),
    screenWidth(screenW), sweepSpeed(100.f), movingRight(true)
{
    attackInterval = 99.f;
    buildSprite();
}

void TwinCannons::buildSprite() {
    coreRect.setSize({ 100.f, 50.f });
    coreRect.setOrigin({50.f, 25.f});
    coreRect.setFillColor(sf::Color(40, 60, 140));
    coreRect.setOutlineColor(sf::Color(80, 120, 255));
    coreRect.setOutlineThickness(3.f);

    coreGlow.setSize({ 100.f, 50.f });
    coreGlow.setOrigin({50.f, 25.f});
    coreGlow.setFillColor(sf::Color(60, 80, 200, 80));
}

void TwinCannons::updateTurretPositions() {
    leftTurret.setPosition(position.x - 90.f, position.y + 15.f);
    rightTurret.setPosition(position.x + 90.f, position.y + 15.f);
}

void TwinCannons::update(float dt) {
    Boss::update(dt);
    if (entering) return; // skip AI during entrance

    if (movingRight) {
        position.x += sweepSpeed * dt;
        if (position.x > screenWidth - width / 2.f - 80.f) movingRight = false;
    }
    else {
        position.x -= sweepSpeed * dt;
        if (position.x < width / 2.f + 80.f) movingRight = true;
    }

    updateTurretPositions();
    leftTurret.update(dt);
    rightTurret.update(dt);
}

// ── Returns the hitbox of whichever part should be hit next ──────────────────
sf::FloatRect TwinCannons::getActiveHitbox() const {
    // Priority: left turret → right turret → core
    if (!leftTurret.isDestroyed())
        return leftTurret.getHitbox();
    if (!rightTurret.isDestroyed())
        return rightTurret.getHitbox();
    return getHitbox(); // core (base class hitbox)
}

// ── Route damage to the correct part ─────────────────────────────────────────
void TwinCannons::hitByBullet(int dmg) {
    if (!leftTurret.isDestroyed()) {
        leftTurret.takeDamage(dmg);
    }
    else if (!rightTurret.isDestroyed()) {
        rightTurret.takeDamage(dmg);
    }
    else {
        // Core is now exposed
        takeDamage(dmg);          // base Enemy::takeDamage → sets alive=false at 0
    }
}

void TwinCannons::draw(sf::RenderWindow& window) {
    coreRect.setPosition(position);
    coreGlow.setPosition(position);

    // Glow pulses when core is vulnerable
    if (coreIsVulnerable()) {
        float alpha = 100 + (int)(80 * std::abs(std::sin(phase * 4)));
        coreGlow.setFillColor(sf::Color(60, 180, 255, (std::uint8_t)alpha));
        window.draw(coreGlow);
    }
    window.draw(coreRect);

    leftTurret.draw(window);
    rightTurret.draw(window);

    // ── HP bar shows the current active part's HP ─────────────────────────
    // Temporarily override hp/maxHP so Boss::drawHealthBar shows the right values
    int  savedHP = hp, savedMax = maxHP;
    bool leftAlive = !leftTurret.isDestroyed();
    bool rightAlive = !rightTurret.isDestroyed();

    if (leftAlive) {
        // Show left turret HP
        const_cast<TwinCannons*>(this)->hp = leftTurret.getHP();
        const_cast<TwinCannons*>(this)->maxHP = 30;
    }
    else if (rightAlive) {
        // Show right turret HP
        const_cast<TwinCannons*>(this)->hp = rightTurret.getHP();
        const_cast<TwinCannons*>(this)->maxHP = 30;
    }
    // else core — hp/maxHP are already correct (30/30)

    Boss::draw(window);   // draws health bar

    // Restore real core hp/maxHP
    const_cast<TwinCannons*>(this)->hp = savedHP;
    const_cast<TwinCannons*>(this)->maxHP = savedMax;
}

void TwinCannons::onCollision(GameObject* other) {
    // Handled by hitByBullet() via CollisionManager
}

Bullet** TwinCannons::attack(int& count) {
    if (entering) { count = 0; return nullptr; }
    Bullet* lb = leftTurret.shoot();
    Bullet* rb = rightTurret.shoot();
    count = 0;
    if (!lb && !rb) return nullptr;
    Bullet** arr = new Bullet * [2];
    if (lb) arr[count++] = lb;
    if (rb) arr[count++] = rb;
    return arr;
}