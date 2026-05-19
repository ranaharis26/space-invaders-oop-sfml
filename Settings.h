#pragma once
#include <SFML/Window/Keyboard.hpp>

class Settings {
public:
    static sf::Keyboard::Key keyUp;
    static sf::Keyboard::Key keyDown;
    static sf::Keyboard::Key keyLeft;
    static sf::Keyboard::Key keyRight;
    static sf::Keyboard::Key keyShoot;
    static sf::Keyboard::Key keyDash;
    static sf::Keyboard::Key keyEMP;
    
    static bool fullscreen;
};
