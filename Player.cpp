#include "Player.h"
#include "AudioManager.h"
#include "Settings.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cmath>

const float Player::SPEED = 280.f;
const float Player::DASH_DISTANCE = 120.f;
const float Player::DASH_COOLDOWN = 3.f;
const float Player::INVINCIBLE_AFTER_DASH = 0.5f;
const float Player::SHOOT_COOLDOWN = 0.18f;

Player::Player(float x, float y)
    : Entity(x, y, 36.f, 40.f),
      lives(3), empCount(1), shieldHits(0),
      weapon(WeaponType::STANDARD),
      shootTimer(0), dashCooldown(0), invincibleTimer(0),
      flashTimer(0), hitThisFrame(false)
{
    buildSprite();
}

void Player::buildSprite() {
    // Main hull: pointed upward triangle-ish ship
    shipShape.setPointCount(7);
    shipShape.setPoint(0, {0, -20});
    shipShape.setPoint(1, {10, -5});
    shipShape.setPoint(2, {18, 10});
    shipShape.setPoint(3, {8,  20});
    shipShape.setPoint(4, {-8, 20});
    shipShape.setPoint(5, {-18,10});
    shipShape.setPoint(6, {-10,-5});
    shipShape.setFillColor(sf::Color(60, 180, 255));
    shipShape.setOutlineColor(sf::Color(160, 230, 255));
    shipShape.setOutlineThickness(1.5f);

    // Engine glow
    engineShape.setPointCount(4);
    engineShape.setPoint(0, {-8, 20});
    engineShape.setPoint(1, {8, 20});
    engineShape.setPoint(2, {5, 28});
    engineShape.setPoint(3, {-5, 28});
    engineShape.setFillColor(sf::Color(255, 160, 40));

    // Weapon barrel (changes with weapon type)
    weaponBarrel.setSize({4.f, 14.f});
    weaponBarrel.setOrigin({2.f, 14.f});
    weaponBarrel.setFillColor(sf::Color(200, 200, 255));

    // Shield ring
    shieldShape.setRadius(30.f);
    shieldShape.setOrigin({30.f, 30.f});
    shieldShape.setFillColor(sf::Color::Transparent);
    shieldShape.setOutlineThickness(3.f);
    shieldShape.setOutlineColor(sf::Color(80, 255, 200, 180));
}

void Player::update(float dt) {
    hitThisFrame = false;
    shootTimer    -= dt;
    invincibleTimer -= dt;
    flashTimer    -= dt;
    if (dashCooldown > 0) dashCooldown -= dt;

    // Movement
    sf::Vector2f dir(0,0);
    if (sf::Keyboard::isKeyPressed(Settings::keyLeft))  dir.x -= 1;
    if (sf::Keyboard::isKeyPressed(Settings::keyRight)) dir.x += 1;
    if (sf::Keyboard::isKeyPressed(Settings::keyUp))    dir.y -= 1;
    if (sf::Keyboard::isKeyPressed(Settings::keyDown))  dir.y += 1;

    // Normalize
    float len = std::sqrt(dir.x*dir.x + dir.y*dir.y);
    if (len > 0) dir /= len;

    velocity = dir * SPEED;

    // Dash: E + arrow key
    static bool dashHeld = false;
    bool eDn = sf::Keyboard::isKeyPressed(Settings::keyDash);
    if (eDn && !dashHeld && dashCooldown <= 0 && len > 0) {
        position += dir * DASH_DISTANCE;
        dashCooldown = DASH_COOLDOWN;
        invincibleTimer = INVINCIBLE_AFTER_DASH;
        dashHeld = true;
    }
    if (!eDn) dashHeld = false;

    Entity::update(dt);

    // Clamp to screen
    const float hw = width/2.f, hh = height/2.f;
    if (position.x < hw)       position.x = hw;
    if (position.x > 900-hw)   position.x = 900-hw;
    if (position.y < hh)       position.y = hh;
    if (position.y > 700-hh)   position.y = 700-hh;

    // Animate engine flame
    float flicker = (std::rand() % 3) * 0.5f;
    engineShape.setFillColor(sf::Color(255, 120 + (int)(flicker*40), 20));
}

Bullet** Player::shoot(int& count) {
    count = 0;
    if (shootTimer > 0) return nullptr;
    if (!sf::Keyboard::isKeyPressed(Settings::keyShoot)) return nullptr;

    shootTimer = SHOOT_COOLDOWN;

    Bullet** bullets = nullptr;
    float bx = position.x, by = position.y - height/2.f - 5.f;

    switch (weapon) {
    case WeaponType::STANDARD:
        bullets = new Bullet*[1];
        bullets[0] = new Bullet(bx, by, 0, -600.f, 1, false, Bullet::Owner::PLAYER);
        count = 1;
        AudioManager::getInstance().playShoot(0);
        break;

    case WeaponType::SPREAD:
        bullets = new Bullet*[3];
        bullets[0] = new Bullet(bx, by, -200.f, -550.f, 1, false, Bullet::Owner::PLAYER);
        bullets[1] = new Bullet(bx, by,  0,     -600.f, 1, false, Bullet::Owner::PLAYER);
        bullets[2] = new Bullet(bx, by,  200.f, -550.f, 1, false, Bullet::Owner::PLAYER);
        count = 3;
        AudioManager::getInstance().playShoot(1);
        break;

    case WeaponType::PIERCING:
        bullets = new Bullet*[1];
        bullets[0] = new Bullet(bx, by, 0, -580.f, 2, true, Bullet::Owner::PLAYER);
        count = 1;
        AudioManager::getInstance().playShoot(2);
        break;
    }
    return bullets;
}

void Player::takeDamage(int dmg) {
    if (invincibleTimer > 0) return;

    if (shieldHits > 0) {
        shieldHits--;
        flashTimer = 0.3f;
        hitThisFrame = true;
        if (shieldHits == 0) {
            AudioManager::getInstance().playShieldBreak();
        } else {
            AudioManager::getInstance().playPlayerHit();
        }
        return;
    }

    lives--;
    flashTimer = 0.5f;
    invincibleTimer = 2.0f;
    hitThisFrame = true;
    AudioManager::getInstance().playPlayerHit();
}

void Player::collectWeapon(WeaponType w) {
    weapon = w;
    AudioManager::getInstance().playPowerUp();
}

void Player::collectShield() {
    shieldHits = 2;
    AudioManager::getInstance().playPowerUp();
}

void Player::collectEMP() {
    if (empCount < 3) empCount++;
    AudioManager::getInstance().playPowerUp();
}

void Player::useEMP() {
    if (empCount > 0) {
        empCount--;
        AudioManager::getInstance().playEMP();
    }
}

float Player::getDashCooldownRatio() const {
    if (dashCooldown <= 0) return 1.f;
    return 1.f - (dashCooldown / DASH_COOLDOWN);
}

void Player::drawShield(sf::RenderWindow& window) {
    if (shieldHits <= 0) return;
    shieldShape.setPosition(position);
    // Pulse alpha
    float pulse = 140.f + 80.f * std::sin(invincibleTimer * 5.f);
    sf::Color sc = (shieldHits == 2) ? sf::Color(80,255,200,(int)pulse)
                                      : sf::Color(255,180,50,(int)pulse);
    shieldShape.setOutlineColor(sc);
    window.draw(shieldShape);
}

void Player::draw(sf::RenderWindow& window) {
    if (flashTimer > 0 && (int)(flashTimer * 10) % 2 == 0) return; // blink

    shipShape.setPosition(position);
    engineShape.setPosition(position);
    weaponBarrel.setPosition({position.x, position.y - height/2.f});

    // Color barrel by weapon
    switch(weapon) {
    case WeaponType::STANDARD:
        weaponBarrel.setSize({4,14}); weaponBarrel.setOrigin({2,14});
        weaponBarrel.setFillColor(sf::Color(200,200,255)); break;
    case WeaponType::SPREAD:
        weaponBarrel.setSize({14,12}); weaponBarrel.setOrigin({7,12});
        weaponBarrel.setFillColor(sf::Color(80,255,150)); break;
    case WeaponType::PIERCING:
        weaponBarrel.setSize({5,18}); weaponBarrel.setOrigin({2.5f,18});
        weaponBarrel.setFillColor(sf::Color(255,80,80)); break;
    }

    window.draw(engineShape);
    window.draw(shipShape);
    window.draw(weaponBarrel);
    drawShield(window);
}

void Player::onCollision(GameObject* other) {
    takeDamage(1);
}
