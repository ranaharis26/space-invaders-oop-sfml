#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "GameStateManager.h"

class HUD {
public:
    HUD(sf::Font& font);
    void draw(sf::RenderWindow& window,
              int score, int multiplier, int lives,
              const std::string& weaponName, int shieldHits,
              int empCount, float dashCooldownRatio,
              int wave, GameMode mode);
private:
    sf::Font& font;
    std::string intToStr(int n) const;
};
