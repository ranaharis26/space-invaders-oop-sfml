#include "Boss.h"
#include <cmath>

Boss::Boss(float x, float y, float w, float h, int hp)
    : Enemy(x, y, w, h, hp),
      phase(0), attackTimer(0), attackInterval(2.f), phaseTwo(false),
      entering(true), entranceTimer(1.5f), displayHP((float)hp)
{
    // Save intended position, start above screen
    targetPosition = position;
    entranceStartY = -height - 40.f;
    position.y = entranceStartY;

    hpBarBg.setSize({400.f, 18.f});
    hpBarBg.setFillColor(sf::Color(40, 10, 10, 220));
    hpBarBg.setOutlineColor(sf::Color(200, 60, 60));
    hpBarBg.setOutlineThickness(2.f);

    hpBarFill.setSize({400.f, 18.f});
    hpBarFill.setFillColor(sf::Color(220, 50, 50));
}

void Boss::update(float dt) {
    // ── Entrance animation: slide into arena ────────────────────────────────
    if (entering) {
        entranceTimer -= dt;
        float t = 1.0f - (entranceTimer / 1.5f);
        if (t > 1.0f) t = 1.0f;
        if (t < 0.0f) t = 0.0f;
        // Smooth ease-out interpolation
        float smooth = 1.0f - (1.0f - t) * (1.0f - t);
        position.y = entranceStartY + (targetPosition.y - entranceStartY) * smooth;
        position.x = targetPosition.x;
        phase += dt; // keep phase timer for visual effects
        if (entranceTimer <= 0.f) {
            entering = false;
            position = targetPosition;
        }
        return; // skip normal update during entrance
    }

    Enemy::update(dt);
    phase += dt;
    attackTimer -= dt;
    if ((float)hp / maxHP < 0.4f) phaseTwo = true;

    // Smooth HP display animation
    if (displayHP > (float)hp) {
        displayHP -= (float)maxHP * dt * 0.8f;
        if (displayHP < (float)hp) displayHP = (float)hp;
    }
}

void Boss::draw(sf::RenderWindow& window) {
    drawHealthBar(window);
}

void Boss::onCollision(GameObject* other) {
    // handled by collision manager
}

void Boss::drawHealthBar(sf::RenderWindow& window) const {
    float barW = 400.f;
    float barX = 250.f; // (900 - 400) / 2
    float barY = 12.f;

    // Smooth display ratio
    if (displayHP > (float)maxHP) displayHP = (float)maxHP;
    float displayRatio = (maxHP > 0) ? displayHP / (float)maxHP : 0;
    float actualRatio  = (maxHP > 0) ? (float)hp / (float)maxHP : 0;
    if (displayRatio < 0) displayRatio = 0;
    if (actualRatio  < 0) actualRatio  = 0;

    // Dark background bar
    hpBarBg.setPosition({barX, barY});
    window.draw(hpBarBg);

    // Damage trail (yellow, shows the smooth decrease)
    if (displayRatio > actualRatio) {
        sf::RectangleShape trail({barW * displayRatio, 18.f});
        trail.setPosition({barX, barY});
        trail.setFillColor(sf::Color(255, 200, 60, 180));
        window.draw(trail);
    }

    // Red fill bar (actual HP)
    hpBarFill.setSize({barW * actualRatio, 18.f});
    hpBarFill.setPosition({barX, barY});

    // Color shift: red when low HP
    if (actualRatio < 0.3f)
        hpBarFill.setFillColor(sf::Color(255, 50, 50));
    else if (actualRatio < 0.6f)
        hpBarFill.setFillColor(sf::Color(220, 100, 40));
    else
        hpBarFill.setFillColor(sf::Color(220, 50, 50));

    window.draw(hpBarFill);

    // Segmented markers every 10%
    for (int seg = 1; seg < 10; seg++) {
        float sx = barX + barW * (seg / 10.f);
        sf::RectangleShape mark({1.f, 18.f});
        mark.setPosition({sx, barY});
        mark.setFillColor(sf::Color(0, 0, 0, 100));
        window.draw(mark);
    }
}
