#include "Mothership.h"
#include <cstdlib>
#include <cmath>
#include <cstdint>

Mothership::Mothership(float screenW, float playerX)
    : Boss(screenW / 2.f, 130.f, 220.f, 80.f, 350),
    screenWidth(screenW), playerXRef(playerX),
    sweepSpeed(120.f), movingRight(true),
    laserWarning(false), laserWarningTimer(0.f),
    laserFiring(false), laserFireTimer(0.f), laserFireDuration(0.9f),
    aimedShotWarning(false), aimedShotWarningTimer(0.f), aimedShotX(0.f),
    pendingAimedShot(false),
    fireFlicker(0.f),
    seekerSpawnTimer(0.f), seekerSpawnInterval(5.f)
{
    attackInterval = 6.f;
    buildSprite();
}

void Mothership::buildSprite() {
    bodyRect.setSize({ 220.f, 60.f });
    bodyRect.setOrigin({110.f, 30.f});
    bodyRect.setFillColor(sf::Color(60, 30, 80));
    bodyRect.setOutlineColor(sf::Color(180, 80, 255));
    bodyRect.setOutlineThickness(3.f);

    wingL.setPointCount(4);
    wingL.setPoint(0, { 0,-10 }); wingL.setPoint(1, { -80,0 });
    wingL.setPoint(2, { -70,30 }); wingL.setPoint(3, { 0,25 });
    wingL.setFillColor(sf::Color(50, 20, 70));

    wingR.setPointCount(4);
    wingR.setPoint(0, { 0,-10 }); wingR.setPoint(1, { 80,0 });
    wingR.setPoint(2, { 70,30 }); wingR.setPoint(3, { 0,25 });
    wingR.setFillColor(sf::Color(50, 20, 70));

    core.setRadius(20.f);
    core.setOrigin({20.f, 20.f});
    core.setFillColor(sf::Color(200, 100, 255));

    laserOuter.setSize({ screenWidth, 48.f });
    laserOuter.setOrigin({screenWidth / 2.f, 24.f});

    laserMid.setSize({ screenWidth, 28.f });
    laserMid.setOrigin({screenWidth / 2.f, 14.f});

    laserCore.setSize({ screenWidth, 10.f });
    laserCore.setOrigin({screenWidth / 2.f, 5.f});

    warnRect.setSize({ screenWidth, 14.f });
    warnRect.setOrigin({screenWidth / 2.f, 7.f});

    aimedWarnRect.setSize({ 24.f, 600.f });
    aimedWarnRect.setOrigin({12.f, 0.f});
}

void Mothership::update(float dt) {
    Boss::update(dt);
    if (entering) return; // skip AI during entrance

    if (movingRight) {
        position.x += sweepSpeed * dt;
        if (position.x > screenWidth - 120.f) movingRight = false;
    }
    else {
        position.x -= sweepSpeed * dt;
        if (position.x < 120.f) movingRight = true;
    }

    fireFlicker += dt;
    seekerSpawnInterval = phaseTwo ? 3.f : 5.f;

    // IDLE: start attack cycle
    if (attackTimer <= 0.f && !laserWarning && !laserFiring
        && !aimedShotWarning && !pendingAimedShot) {
        attackTimer = attackInterval;
        laserWarning = true;
        laserWarningTimer = 1.0f;
    }

    // WARNING: red bar flickers for 1 second
    if (laserWarning) {
        laserWarningTimer -= dt;
        if (laserWarningTimer <= 0.f) {
            laserWarning = false;
            laserFiring = true;
            laserFireTimer = laserFireDuration;
        }
    }

    // FIRING: fire laser beam
    if (laserFiring) {
        laserFireTimer -= dt;
        if (laserFireTimer <= 0.f) {
            laserFiring = false;
            aimedShotWarning = true;
            aimedShotWarningTimer = 1.0f;
            aimedShotX = playerXRef; // lock player X now
        }
    }

    // AIMED WARNING: column flickers over player position for 1 second
    if (aimedShotWarning) {
        aimedShotWarningTimer -= dt;
        if (aimedShotWarningTimer <= 0.f) {
            aimedShotWarning = false;
            pendingAimedShot = true;
        }
    }

    seekerSpawnTimer -= dt;
}

void Mothership::drawLaserWarning(sf::RenderWindow& window, float laserY) {
    float speed = 12.f + (1.f - laserWarningTimer) * 20.f;
    float alpha = 80.f + 160.f * std::abs(std::sin(fireFlicker * speed));
    warnRect.setFillColor(sf::Color(255, 40, 40, (std::uint8_t)alpha));
    warnRect.setPosition({screenWidth / 2.f, laserY});
    window.draw(warnRect);
}

void Mothership::drawAimedWarning(sf::RenderWindow& window) {
    float speed = 10.f + (1.f - aimedShotWarningTimer) * 25.f;
    float alpha = 80.f + 160.f * std::abs(std::sin(fireFlicker * speed));

    // wide outer glow
    sf::RectangleShape glow({ 60.f, 600.f });
    glow.setOrigin({30.f, 0.f});
    glow.setPosition({aimedShotX, position.y + height / 2.f});
    glow.setFillColor(sf::Color(255, 80, 0, (std::uint8_t)(alpha * 0.4f)));
    window.draw(glow);

    // tight inner column
    aimedWarnRect.setPosition({aimedShotX, position.y + height / 2.f});
    aimedWarnRect.setFillColor(sf::Color(255, 40, 40, (std::uint8_t)alpha));
    window.draw(aimedWarnRect);
}

void Mothership::drawFireLaser(sf::RenderWindow& window, float laserY) {
    float cx = screenWidth / 2.f;

    float outerAlpha = 100.f + 80.f * std::abs(std::sin(fireFlicker * 8.f));
    float outerH = 44.f + 12.f * std::abs(std::sin(fireFlicker * 6.f));
    laserOuter.setSize({ screenWidth, outerH });
    laserOuter.setOrigin({screenWidth / 2.f, outerH / 2.f});
    laserOuter.setFillColor(sf::Color(255, 60, 0, (std::uint8_t)outerAlpha));
    laserOuter.setPosition({cx, laserY});
    window.draw(laserOuter);

    float midOff = 3.f * std::sin(fireFlicker * 14.f);
    float midAlpha = 180.f + 60.f * std::abs(std::sin(fireFlicker * 11.f));
    laserMid.setPosition({cx, laserY + midOff});
    laserMid.setFillColor(sf::Color(255, 200, 30, (std::uint8_t)midAlpha));
    window.draw(laserMid);

    float coreAlpha = 220.f + 35.f * std::abs(std::sin(fireFlicker * 20.f));
    laserCore.setPosition({cx, laserY});
    laserCore.setFillColor(sf::Color(255, 255, 200, (std::uint8_t)coreAlpha));
    window.draw(laserCore);

    sf::RectangleShape spark({ 6.f, 6.f });
    spark.setOrigin({3.f, 3.f});
    for (int i = 0; i < 6; i++) {
        float sx = (float)(std::rand() % (int)screenWidth);
        float sy = laserY + (float)((std::rand() % 20) - 10);
        spark.setPosition({sx, sy});
        spark.setFillColor(sf::Color(255, 220, 80,
            (std::uint8_t)(100 + std::rand() % 155)));
        window.draw(spark);
    }
}

void Mothership::draw(sf::RenderWindow& window) {
    bodyRect.setPosition(position);
    wingL.setPosition(position);
    wingR.setPosition(position);
    core.setPosition({position.x, position.y});

    window.draw(wingL);
    window.draw(wingR);
    window.draw(bodyRect);

    float pulse = 150.f + 80.f * std::abs(std::sin(phase * 3.f));
    core.setFillColor(sf::Color(200, 100, 255, (std::uint8_t)pulse));
    window.draw(core);

    float laserY = getLaserY();
    if (laserWarning)     drawLaserWarning(window, laserY);
    if (laserFiring)      drawFireLaser(window, laserY);
    if (aimedShotWarning) drawAimedWarning(window);

    Boss::draw(window);
}

Bullet** Mothership::attack(int& count) {
    if (entering) { count = 0; return nullptr; }
    count = 0;
    if (!pendingAimedShot) return nullptr;
    pendingAimedShot = false;

    // 20 bullets evenly spaced across full screen width,
    // all converging toward the locked player X
    const int NUM = 20;
    float spacing = screenWidth / NUM;
    float fireY = position.y + height / 2.f;

    Bullet** arr = new Bullet * [NUM];
    count = 0;

    for (int i = 0; i < NUM; i++) {
        float bx = spacing * 0.5f + i * spacing;
        float dx = aimedShotX - bx;
        float dy = 600.f;
        float len = std::sqrt(dx * dx + dy * dy);
        if (len == 0.f) len = 1.f;
        float speed = 300.f;
        arr[count++] = new Bullet(bx, fireY,
            (dx / len) * speed,
            (dy / len) * speed,
            1, false, Bullet::Owner::ENEMY);
    }
    return arr;
}

Enemy** Mothership::spawnEnemies(int& count) {
    if (entering) { count = 0; return nullptr; }
    if (seekerSpawnTimer > 0.f) { count = 0; return nullptr; }
    seekerSpawnTimer = seekerSpawnInterval;

    count = phaseTwo ? 3 : 2;
    Enemy** arr = new Enemy * [count];
    for (int i = 0; i < count; i++) {
        float sx = (float)(std::rand() % (int)(screenWidth - 100) + 50);
        float sy = -30.f - i * 35.f;
        arr[i] = new Seeker(sx, sy, playerXRef);
    }
    return arr;
}