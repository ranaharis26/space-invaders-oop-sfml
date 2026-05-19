#include "HUD.h"
#include <string>

HUD::HUD(sf::Font& f) : font(f) {}

std::string HUD::intToStr(int n) const {
    if (n == 0) return "0";
    bool neg = n < 0;
    if (neg) n = -n;
    char buf[20]; int idx = 19;
    buf[idx] = '\0';
    while (n > 0) { buf[--idx] = '0' + (n % 10); n /= 10; }
    if (neg) buf[--idx] = '-';
    return std::string(buf + idx);
}

void HUD::draw(sf::RenderWindow& window,
               int score, int multiplier, int lives,
               const std::string& weaponName, int shieldHits,
               int empCount, float dashRatio,
               int wave, GameMode mode)
{
    auto drawText = [&](const std::string& str, float x, float y,
                        unsigned int size, sf::Color col) {
        sf::Text t(font, str, size);
        t.setFillColor(col);
        t.setPosition({x, y});
        window.draw(t);
    };

    // HUD background bar
    sf::RectangleShape bar({900.f, 46.f});
    bar.setFillColor(sf::Color(0, 0, 0, 170));
    bar.setPosition({0, 0});
    window.draw(bar);

    // Score
    drawText("SCORE: " + intToStr(score), 230, 8, 18, sf::Color::White);

    // Multiplier
    sf::Color multCol = (multiplier > 1) ? sf::Color(255,220,60) : sf::Color(180,180,180);
    drawText("x" + intToStr(multiplier), 430, 8, 18, multCol);

    // Weapon
    sf::Color wCol = sf::Color(80, 220, 255);
    if (weaponName == "SPREAD")   wCol = sf::Color(80, 255, 150);
    if (weaponName == "PIERCING") wCol = sf::Color(255, 80, 80);
    drawText("WPN: " + weaponName, 490, 8, 16, wCol);

    // Shield
    if (shieldHits > 0)
        drawText("SHIELD: " + intToStr(shieldHits), 640, 8, 16, sf::Color(80, 200, 255));
    else
        drawText("SHIELD: --", 640, 8, 16, sf::Color(100,100,100));

    // EMP count
    drawText("EMP: " + intToStr(empCount), 760, 8, 16, sf::Color(255, 220, 60));

    // Dash cooldown bar (bottom right)
    sf::RectangleShape dashBg({80.f, 10.f});
    dashBg.setFillColor(sf::Color(40,40,40));
    dashBg.setPosition({812.f, 680.f});
    window.draw(dashBg);

    sf::RectangleShape dashFill({80.f * dashRatio, 10.f});
    sf::Color dashCol = (dashRatio >= 1.f) ? sf::Color(60,255,120) : sf::Color(255,180,40);
    dashFill.setFillColor(dashCol);
    dashFill.setPosition({812.f, 680.f});
    window.draw(dashFill);
    drawText("DASH", 812, 662, 13, sf::Color(180,180,180));

    // Wave / Level
    std::string waveStr = (mode == GameMode::SURVIVAL)
        ? ("WAVE: " + intToStr(wave))
        : ("LEVEL: " + intToStr(wave));
    drawText(waveStr, 200, 678, 16, sf::Color(200,200,200));
}
