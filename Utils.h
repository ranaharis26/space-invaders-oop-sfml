#pragma once
#include <SFML/Graphics.hpp>
#include <string>

// ─── Lightweight int-to-string (no sstream) ─────────────────────────────────
inline std::string intToStr(int n) {
    if (n == 0) return "0";
    bool neg = n < 0;
    if (neg) n = -n;
    char buf[20]; int idx = 19;
    buf[idx] = '\0';
    while (n > 0) { buf[--idx] = '0' + (n % 10); n /= 10; }
    if (neg) buf[--idx] = '-';
    return std::string(buf + idx);
}

// ─── Draw text centered at (cx, cy) ─────────────────────────────────────────
inline void drawTextCentered(sf::RenderWindow& window, sf::Text& text, float cx, float cy) {
    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin({bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f});
    text.setPosition({cx, cy});
    window.draw(text);
}
