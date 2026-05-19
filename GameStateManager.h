#pragma once
#include <SFML/Graphics.hpp>
#include <string>

enum class GameState { MENU, PLAYING, PAUSED, GAME_OVER, WIN, HIGH_SCORE, INSTRUCTIONS, CREDITS, USERNAME_INPUT, SETTINGS };
enum class GameMode  { ARCADE, SURVIVAL };

class GameStateManager {
public:
    GameStateManager();
    GameState getState() const { return state; }
    GameMode  getMode()  const { return mode; }
    void setState(GameState s) { state = s; }
    void setMode (GameMode  m) { mode  = m; }

private:
    GameState state;
    GameMode  mode;
};
