#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cstdint>
#include "GameStateManager.h"
#include "Player.h"
#include "Bullet.h"
#include "Enemy.h"
#include "Drone.h"
#include "Viper.h"
#include "Seeker.h"
#include "Boss.h"
#include "Cruiser.h"
#include "TwinCannons.h"
#include "Mothership.h"
#include "PowerUp.h"
#include "Asteroid.h"
#include "Explosion.h"
#include "Starfield.h"
#include "CollisionManager.h"
#include "AudioManager.h"
#include "HUD.h"
#include "Utils.h"

static const int SCREEN_W = 900;
static const int SCREEN_H = 700;
static const int MAX_BULLETS   = 200;
static const int MAX_ENEMIES   = 60;
static const int MAX_POWERUPS  = 20;
static const int MAX_ASTEROIDS = 5;
static const int MAX_EXPLOSIONS= 40;
static const int MAX_HIGH_SCORES = 5;
static const int MAX_POPUPS      = 30;
static const int MAX_LEADERBOARD = 10;
static const int MAX_USERNAME    = 12;

// ─── Score popup struct ──────────────────────────────────────────────────────
struct ScorePopup {
    sf::Text* text;
    sf::Vector2f vel;
    float lifetime;
    float maxLifetime;
    bool active;
};

// ─── Leaderboard entry struct ────────────────────────────────────────────────
struct HighscoreEntry {
    std::string name;
    int score;
};

class Game {
public:
    Game();
    ~Game();
    void run();

private:
    // Core loop
    void processEvents();
    void update(float dt);
    void render();

    // State handlers
    void handleMenu(float dt);
    void handlePlaying(float dt);
    void handlePaused(float dt);
    void handleGameOver(float dt);
    void handleWin(float dt);
    void handleHighScore(float dt);
    void handleInstructions(float dt);
    void handleCredits(float dt);
    void handleUsernameInput(float dt);
    void handleSettings(float dt);

    void renderMenu();
    void renderPlaying();
    void renderPaused();
    void renderGameOver();
    void renderWin();
    void renderHighScore();
    void renderInstructions();
    void renderCredits();
    void renderUsernameInput();
    void renderSettings();

    // Gameplay helpers
    void startGame(GameMode mode);
    void resetLevel();
    void spawnWave();
    void spawnAsteroids();
    void spawnBoss(int level);
    void updateMultiplier(float dt);
    void addExplosion(float x, float y, sf::Color col, int count=20, float r=80.f);
    void activateEMP();
    void tryDropPowerUp(float x, float y);
    void tryDropEMP(float x, float y);
    void compactBullets(Bullet** arr, int& count);
    void compactEnemies(Enemy** arr, int& count);
    void compactPowerUps(PowerUp** arr, int& count);
    void compactExplosions();
    void checkWaveComplete();
    void advanceLevel();
    std::string weaponName(WeaponType w);
    void saveHighScore(int s);
    void drawHealthBar(sf::RenderWindow& window);
    void drawMenuBackground(sf::RenderWindow& window, float animTime);
    void drawMenuTitle(sf::RenderWindow& window, float animTime);
    void drawArrow(sf::RenderWindow& window, float x, float y, sf::Color col);

    // Score popups
    void addScorePopup(float x, float y, int points, int mult);
    void updateScorePopups(float dt);
    void drawScorePopups(sf::RenderWindow& window);

    // Leaderboard
    void loadLeaderboard();
    void saveLeaderboard();
    void updatePlayerScore(const std::string& name, int sc);
    void sortLeaderboard();

    // intToStr helper
    std::string intToStr(int n);

    sf::RenderWindow window;
    sf::Font font;

    GameStateManager gsm;
    Starfield* starfield;
    Player* player;
    HUD* hud;

    // Dynamic arrays
    Bullet**    playerBullets;  int pbCount;
    Bullet**    enemyBullets;   int ebCount;
    Enemy**     enemies;        int eCount;
    PowerUp**   powerUps;       int puCount;
    Asteroid**  asteroids;      int astCount;
    Explosion** explosions;     int expCount;

    Boss* boss;

    // Game state
    int   score;
    int   multiplier;
    float multiplierTimer;
    float multResetTimer;

    int   wave;
    int   arcadeLevel;
    bool  bossActive;
    bool  bossDefeated;
    float bossTransitionTimer;

    float waveSpawnTimer;
    bool  waveSpawning;
    int   enemiesLeftToSpawn;
    int   spawnIndex;

    // Screen shake
    float shakeTimer;
    float shakeMagnitude;
    sf::Vector2f shakeOffset;

    // Menu state
    int   menuSelection;    // 0=Start 1=HighScore 2=Instructions 3=Credits 4=Exit
    int   pauseSelection;   // 0=resume 1=quit
    int   modeSelection;    // 0=arcade 1=survival (sub-menu after Start)
    bool  inModeSelect;     // true when choosing game mode
    float menuAnimTime;     // for animations
    
    int   settingsSelection;
    int   waitingForKey;    // 0=none, 1=Up, 2=Down, 3=Left, 4=Right, 5=Shoot, 6=Dash, 7=EMP

    // High scores (raw array, no vector)
    int   highScores[MAX_HIGH_SCORES];

    // Player max HP for health bar
    static const int PLAYER_MAX_HP = 3;

    // ── Wave countdown timer ───────────────────────────────────────────
    float waveCountdownTimer;
    bool  isWaveTransition;
    int   pendingWaveNumber;

    // ── Score popups ───────────────────────────────────────────────────
    ScorePopup scorePopups[MAX_POPUPS];

    // ── Multiplier popup ───────────────────────────────────────────────
    float multiplierPopupTimer;
    int   lastMultiplier;

    // ── Shield pulse animation ─────────────────────────────────────────
    float shieldPulseTimer;
    float shieldPulseMaxRadius;

    // ── Leaderboard system ─────────────────────────────────────────────
    HighscoreEntry leaderboard[MAX_LEADERBOARD];
    int   leaderboardCount;
    std::string playerName;

    // ── Text input buffer (for username) ───────────────────────────────
    std::uint32_t textInputBuffer[8];
    int   textInputCount;

    // ── Game over detail tracking ──────────────────────────────────────
    int   lastWaveReached;
    int   lastLevelReached;
    GameMode lastPlayedMode;

    // ── Press start flash ──────────────────────────────────────────────
    bool  pressStartActive;

    // ── Selected mode for username flow ────────────────────────────────
    GameMode pendingMode;
};
