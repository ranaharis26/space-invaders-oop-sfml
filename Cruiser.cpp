#include "Cruiser.h"
#include <cstdlib>
#include <cmath>
#include <cstdint>

Cruiser::Cruiser(float screenW)
    : Boss(screenW / 2.f, 100.f, 180.f, 60.f, 60),
    screenWidth(screenW), sweepSpeed(140.f), movingRight(true),
    safeGap(0), laserWarning(false), warningTimer(0.f), firingNow(false)
{
    attackInterval = 3.0f;
    attackTimer = attackInterval;
    safeGap = std::rand() % 8;
    buildSprite();
}

void Cruiser::buildSprite() {
    // main hull: bright green square
    body.setSize({ 160.f, 54.f });
    body.setOrigin({80.f, 27.f});
    body.setFillColor(sf::Color(0, 200, 70));
    body.setOutlineColor(sf::Color(0, 255, 90));
    body.setOutlineThickness(2.5f);

    // dark panel strips top/bottom
    topPanel.setSize({ 160.f, 10.f });
    topPanel.setOrigin({80.f, 5.f});
    topPanel.setFillColor(sf::Color(0, 80, 30));

    botPanel.setSize({ 160.f, 10.f });
    botPanel.setOrigin({80.f, 5.f});
    botPanel.setFillColor(sf::Color(0, 80, 30));

    // wings
    wingL.setSize({ 50.f, 18.f });
    wingL.setOrigin({50.f, 9.f});
    wingL.setFillColor(sf::Color(0, 160, 50));
    wingL.setOutlineColor(sf::Color(0, 220, 70));
    wingL.setOutlineThickness(1.5f);

    wingR.setSize({ 50.f, 18.f });
    wingR.setOrigin({0.f, 9.f});
    wingR.setFillColor(sf::Color(0, 160, 50));
    wingR.setOutlineColor(sf::Color(0, 220, 70));
    wingR.setOutlineThickness(1.5f);

    // engine blocks (hang below wings)
    engineL.setSize({ 22.f, 14.f });
    engineL.setOrigin({11.f, 0.f});
    engineL.setFillColor(sf::Color(0, 100, 40));
    engineL.setOutlineColor(sf::Color(0, 200, 60));
    engineL.setOutlineThickness(1.f);

    engineR.setSize({ 22.f, 14.f });
    engineR.setOrigin({11.f, 0.f});
    engineR.setFillColor(sf::Color(0, 100, 40));
    engineR.setOutlineColor(sf::Color(0, 200, 60));
    engineR.setOutlineThickness(1.f);

    // engine glow rects (drawn below engines, pulsing)
    engGlowL.setSize({ 14.f, 8.f });
    engGlowL.setOrigin({7.f, 0.f});
    engGlowL.setFillColor(sf::Color(80, 255, 120, 180));

    engGlowR.setSize({ 14.f, 8.f });
    engGlowR.setOrigin({7.f, 0.f});
    engGlowR.setFillColor(sf::Color(80, 255, 120, 180));

    // antennas on top
    antennaL.setSize({ 3.f, 14.f });
    antennaL.setOrigin({1.5f, 14.f});
    antennaL.setFillColor(sf::Color(0, 200, 70));

    antennaR.setSize({ 3.f, 14.f });
    antennaR.setOrigin({1.5f, 14.f});
    antennaR.setFillColor(sf::Color(0, 200, 70));

    antTipL.setRadius(4.f);
    antTipL.setOrigin({4.f, 4.f});
    antTipL.setFillColor(sf::Color(0, 255, 120));

    antTipR.setRadius(4.f);
    antTipR.setOrigin({4.f, 4.f});
    antTipR.setFillColor(sf::Color(0, 255, 120));

    // 3 windows across the hull
    float winOffsets[3] = { -52.f, 0.f, 52.f };
    for (int i = 0; i < 3; i++) {
        win[i].setSize({ 18.f, 12.f });
        win[i].setOrigin({9.f, 6.f});
        win[i].setFillColor(sf::Color(0, 40, 20));
        win[i].setOutlineColor(sf::Color(0, 255, 100));
        win[i].setOutlineThickness(1.5f);

        winGlow[i].setSize({ 10.f, 6.f });
        winGlow[i].setOrigin({5.f, 3.f});
        winGlow[i].setFillColor(sf::Color(100, 255, 160, 160));
    }

    // pulsing core circle in dead centre
    core.setRadius(8.f);
    core.setOrigin({8.f, 8.f});
    core.setFillColor(sf::Color(0, 255, 120));
}

void Cruiser::update(float dt) {
    Boss::update(dt);
    if (entering) return; // skip AI during entrance

    if (movingRight) {
        position.x += sweepSpeed * dt;
        if (position.x > screenWidth - width / 2.f) movingRight = false;
    }
    else {
        position.x -= sweepSpeed * dt;
        if (position.x < width / 2.f) movingRight = true;
    }

    if (attackTimer <= 0.f && !laserWarning && !firingNow) {
        laserWarning = true;
        warningTimer = 1.0f;
        safeGap = std::rand() % 8;
        attackTimer = attackInterval;
    }
    if (laserWarning) {
        warningTimer -= dt;
        if (warningTimer <= 0.f) {
            laserWarning = false;
            firingNow = true;
        }
    }
}

void Cruiser::drawShip(sf::RenderWindow& window) {
    float cx = position.x;
    float cy = position.y;

    // engine glow pulse
    float glow = 120.f + 80.f * std::abs(std::sin(phase * 5.f));
    engGlowL.setFillColor(sf::Color(60, (std::uint8_t)glow, 100, 200));
    engGlowR.setFillColor(sf::Color(60, (std::uint8_t)glow, 100, 200));

    // when warning, tint ship red-ish
    sf::Color hullCol = laserWarning
        ? sf::Color(80, 200, 80)
        : sf::Color(0, 200, 70);
    sf::Color outlineCol = laserWarning
        ? sf::Color(255, 80, 80)
        : sf::Color(0, 255, 90);
    body.setFillColor(hullCol);
    body.setOutlineColor(outlineCol);

    // wings
    wingL.setPosition({cx - 80.f, cy});
    wingR.setPosition({cx + 80.f, cy});
    window.draw(wingL);
    window.draw(wingR);

    // engine blocks
    engineL.setPosition({cx - 55.f, cy + 9.f});
    engineR.setPosition({cx + 55.f, cy + 9.f});
    window.draw(engineL);
    window.draw(engineR);

    // engine glow (below engines)
    engGlowL.setPosition({cx - 55.f, cy + 23.f});
    engGlowR.setPosition({cx + 55.f, cy + 23.f});
    window.draw(engGlowL);
    window.draw(engGlowR);

    // main body
    body.setPosition({cx, cy});
    window.draw(body);

    // top and bottom panel strips
    topPanel.setPosition({cx, cy - 22.f});
    botPanel.setPosition({cx, cy + 22.f});
    window.draw(topPanel);
    window.draw(botPanel);

    // antennas on top
    antennaL.setPosition({cx - 40.f, cy - 27.f});
    antennaR.setPosition({cx + 40.f, cy - 27.f});
    window.draw(antennaL);
    window.draw(antennaR);

    // antenna tips - pulse brightness
    float tipAlpha = 160.f + 95.f * std::abs(std::sin(phase * 6.f));
    antTipL.setFillColor(sf::Color(0, 255, 120, (std::uint8_t)tipAlpha));
    antTipR.setFillColor(sf::Color(0, 255, 120, (std::uint8_t)tipAlpha));
    antTipL.setPosition({cx - 40.f, cy - 41.f});
    antTipR.setPosition({cx + 40.f, cy - 41.f});
    window.draw(antTipL);
    window.draw(antTipR);

    // windows
    float winOffsets[3] = { -52.f, 0.f, 52.f };
    float winGlowAlpha = 100.f + 80.f * std::abs(std::sin(phase * 4.f));
    for (int i = 0; i < 3; i++) {
        win[i].setPosition({cx + winOffsets[i], cy});
        winGlow[i].setPosition({cx + winOffsets[i], cy});
        winGlow[i].setFillColor(sf::Color(100, 255, 160, (std::uint8_t)winGlowAlpha));
        window.draw(win[i]);
        window.draw(winGlow[i]);
    }

    // pulsing core
    float coreAlpha = 180.f + 75.f * std::abs(std::sin(phase * 7.f));
    core.setFillColor(sf::Color(0, 255, 120, (std::uint8_t)coreAlpha));
    core.setPosition({cx, cy});
    window.draw(core);
}

void Cruiser::draw(sf::RenderWindow& window) {
    drawShip(window);

    // laser column warning
    if (laserWarning) {
        const int cols = 8;
        float colW = screenWidth / cols;
        float flashAlpha = 50.f + 60.f * std::abs(std::sin(warningTimer * 18.f));
        sf::RectangleShape warnRect;
        warnRect.setSize({ colW - 4.f, 600.f });
        warnRect.setFillColor(sf::Color(255, 80, 80, (std::uint8_t)flashAlpha));
        for (int i = 0; i < cols; i++) {
            if (i == safeGap) continue;
            warnRect.setPosition({i * colW + 2.f, position.y + height / 2.f});
            window.draw(warnRect);
        }
    }

    Boss::draw(window);  // HP bar
}

Bullet** Cruiser::attack(int& count) {
    if (entering || !firingNow) { count = 0; return nullptr; }
    firingNow = false;

    const int cols = 8;
    float colW = screenWidth / cols;
    Bullet** bullets = new Bullet * [cols];
    count = 0;
    for (int i = 0; i < cols; i++) {
        if (i == safeGap) continue;
        float bx = i * colW + colW / 2.f;
        bullets[count++] = new Bullet(bx, position.y + height / 2.f,
            0.f, 350.f, 1, false, Bullet::Owner::ENEMY);
    }
    return bullets;
}