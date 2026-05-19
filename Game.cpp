#include "Game.h"
#include "Settings.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <cstdio>
#include <cstdint>

// intToStr: replaces sstream for int conversion
std::string Game::intToStr(int n) {
    if (n == 0) return "0";
    bool neg = n < 0;
    if (neg) n = -n;
    char buf[20]; int idx = 19;
    buf[idx] = '\0';
    while (n > 0) { buf[--idx] = '0' + (n % 10); n /= 10; }
    if (neg) buf[--idx] = '-';
    return std::string(buf + idx);
}

// Constructor / Destructor
Game::Game()
    : window(Settings::fullscreen ? sf::VideoMode::getDesktopMode() : sf::VideoMode({SCREEN_W, SCREEN_H}), "Space Invaders - OOP Project",
        Settings::fullscreen ? sf::State::Fullscreen : sf::State::Windowed),
    score(0), multiplier(1), multiplierTimer(0), multResetTimer(0),
    wave(1), arcadeLevel(1), bossActive(false), bossDefeated(false),
    bossTransitionTimer(0),
    waveSpawnTimer(0), waveSpawning(false), enemiesLeftToSpawn(0), spawnIndex(0),
    shakeTimer(0), shakeMagnitude(0),
    menuSelection(0), pauseSelection(0), modeSelection(0),
    inModeSelect(false), menuAnimTime(0.f), settingsSelection(0), waitingForKey(0),
    waveCountdownTimer(0), isWaveTransition(false), pendingWaveNumber(1),
    multiplierPopupTimer(0), lastMultiplier(1),
    shieldPulseTimer(0), shieldPulseMaxRadius(60.f),
    leaderboardCount(0), textInputCount(0),
    lastWaveReached(1), lastLevelReached(1), lastPlayedMode(GameMode::ARCADE),
    pressStartActive(true), pendingMode(GameMode::ARCADE),
    pbCount(0), ebCount(0), eCount(0), puCount(0), astCount(0), expCount(0)
{
    std::srand((unsigned)std::time(nullptr));
    window.setFramerateLimit(60);

    if (!font.openFromFile("C:/Windows/Fonts/arial.ttf")) {
        font.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf");
    }

    starfield = new Starfield(SCREEN_W, SCREEN_H, 180);
    player = nullptr;
    boss   = nullptr;
    hud    = new HUD(font);

    playerBullets = new Bullet*[MAX_BULLETS];
    enemyBullets  = new Bullet*[MAX_BULLETS];
    enemies       = new Enemy*[MAX_ENEMIES];
    powerUps      = new PowerUp*[MAX_POWERUPS];
    asteroids     = new Asteroid*[MAX_ASTEROIDS];
    explosions    = new Explosion*[MAX_EXPLOSIONS];

    for (int i = 0; i < MAX_BULLETS;    i++) { playerBullets[i] = nullptr; enemyBullets[i] = nullptr; }
    for (int i = 0; i < MAX_ENEMIES;    i++) enemies[i]    = nullptr;
    for (int i = 0; i < MAX_POWERUPS;   i++) powerUps[i]   = nullptr;
    for (int i = 0; i < MAX_ASTEROIDS;  i++) asteroids[i]  = nullptr;
    for (int i = 0; i < MAX_EXPLOSIONS; i++) explosions[i] = nullptr;

    highScores[0] = 12500;
    highScores[1] = 8750;
    highScores[2] = 5000;
    highScores[3] = 2200;
    highScores[4] = 1000;

    for (int i = 0; i < MAX_POPUPS; i++) scorePopups[i].active = false;
    for (int i = 0; i < MAX_POPUPS; i++) scorePopups[i].text = nullptr;
    loadLeaderboard();
    AudioManager::getInstance().playMenuMusic();
}

Game::~Game() {
    delete starfield;
    delete player;
    delete boss;
    delete hud;

    for (int i = 0; i < MAX_BULLETS;    i++) { delete playerBullets[i]; delete enemyBullets[i]; }
    for (int i = 0; i < MAX_ENEMIES;    i++) delete enemies[i];
    for (int i = 0; i < MAX_POWERUPS;   i++) delete powerUps[i];
    for (int i = 0; i < MAX_ASTEROIDS;  i++) delete asteroids[i];
    for (int i = 0; i < MAX_EXPLOSIONS; i++) delete explosions[i];

    delete[] playerBullets;
    delete[] enemyBullets;
    delete[] enemies;
    delete[] powerUps;
    delete[] asteroids;
    delete[] explosions;

    for (int i = 0; i < MAX_POPUPS; i++) delete scorePopups[i].text;
}

void Game::saveHighScore(int s) {
    if (s <= highScores[MAX_HIGH_SCORES - 1]) return;
    highScores[MAX_HIGH_SCORES - 1] = s;
    for (int i = MAX_HIGH_SCORES - 1; i > 0 && highScores[i] > highScores[i-1]; i--) {
        int tmp = highScores[i]; highScores[i] = highScores[i-1]; highScores[i-1] = tmp;
    }
}

// Main run loop
void Game::run() {
    sf::Clock clock;
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        if (dt > 0.05f) dt = 0.05f;

        processEvents();

        GameState st = gsm.getState();
        if      (st == GameState::MENU)         handleMenu(dt);
        else if (st == GameState::PLAYING)      handlePlaying(dt);
        else if (st == GameState::PAUSED)       handlePaused(dt);
        else if (st == GameState::GAME_OVER)    handleGameOver(dt);
        else if (st == GameState::WIN)          handleWin(dt);
        else if (st == GameState::HIGH_SCORE)   handleHighScore(dt);
        else if (st == GameState::INSTRUCTIONS) handleInstructions(dt);
        else if (st == GameState::CREDITS)      handleCredits(dt);
        else if (st == GameState::USERNAME_INPUT) handleUsernameInput(dt);
        else if (st == GameState::SETTINGS)       handleSettings(dt);

        render();
    }
}

void Game::processEvents() {
    textInputCount = 0;
    while (const auto event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) window.close();
        if (const auto* re = event->getIf<sf::Event::Resized>()) {
            float windowRatio = (float)re->size.x / (float)re->size.y;
            float viewRatio = (float)SCREEN_W / (float)SCREEN_H;
            float sizeX = 1.f;
            float sizeY = 1.f;
            float posX = 0.f;
            float posY = 0.f;

            if (windowRatio >= viewRatio) {
                sizeX = viewRatio / windowRatio;
                posX = (1.f - sizeX) / 2.f;
            } else {
                sizeY = windowRatio / viewRatio;
                posY = (1.f - sizeY) / 2.f;
            }

            sf::View view({0.f, 0.f}, {(float)SCREEN_W, (float)SCREEN_H});
            view.setCenter({SCREEN_W / 2.f, SCREEN_H / 2.f});
            view.setViewport(sf::FloatRect({posX, posY}, {sizeX, sizeY}));
            window.setView(view);
        }
        if (const auto* te = event->getIf<sf::Event::TextEntered>()) {
            if (textInputCount < 8)
                textInputBuffer[textInputCount++] = te->unicode;
        }
    }
}

// Menu drawing helpers
void Game::drawArrow(sf::RenderWindow& window, float x, float y, sf::Color col) {
    sf::ConvexShape arr;
    arr.setPointCount(3);
    arr.setPoint(0, {0.f, 0.f});
    arr.setPoint(1, {0.f, 18.f});
    arr.setPoint(2, {14.f, 9.f});
    arr.setFillColor(col);
    arr.setPosition({x, y});
    window.draw(arr);
}

void Game::drawMenuBackground(sf::RenderWindow& window, float t) {
    window.clear(sf::Color(4, 8, 20));
    for (int row = 0; row < 30; row++) {
        float yPos = std::fmod((float)(row * 24) - t * 20.f, (float)SCREEN_H);
        sf::RectangleShape line({(float)SCREEN_W, 1.f});
        line.setPosition({0.f, yPos});
        int alpha = 18 + (int)(10.f * std::sin(t + row * 0.4f));
        line.setFillColor(sf::Color(0, 180, 255, alpha));
        window.draw(line);
    }
    for (int col = 0; col < 18; col++) {
        float xPos = std::fmod((float)(col * 50) + t * 8.f, (float)SCREEN_W);
        sf::RectangleShape vline({1.f, (float)SCREEN_H});
        vline.setPosition({xPos, 0.f});
        vline.setFillColor(sf::Color(0, 120, 200, 12));
        window.draw(vline);
    }
    starfield->draw(window);
    sf::RectangleShape lpanel({3.f, (float)SCREEN_H});
    lpanel.setPosition({120.f, 0.f});
    lpanel.setFillColor(sf::Color(0, 200, 255, 60));
    window.draw(lpanel);
    sf::RectangleShape rpanel({3.f, (float)SCREEN_H});
    rpanel.setPosition({SCREEN_W - 123.f, 0.f});
    rpanel.setFillColor(sf::Color(0, 200, 255, 60));
    window.draw(rpanel);
    sf::RectangleShape tl1({40.f, 3.f}); tl1.setPosition({30.f, 30.f});
    sf::RectangleShape tl2({3.f, 40.f}); tl2.setPosition({30.f, 30.f});
    tl1.setFillColor(sf::Color(0, 255, 200, 180));
    tl2.setFillColor(sf::Color(0, 255, 200, 180));
    window.draw(tl1); window.draw(tl2);
    sf::RectangleShape tr1({40.f, 3.f}); tr1.setPosition({SCREEN_W - 70.f, 30.f});
    sf::RectangleShape tr2({3.f, 40.f}); tr2.setPosition({SCREEN_W - 33.f, 30.f});
    tr1.setFillColor(sf::Color(0, 255, 200, 180));
    tr2.setFillColor(sf::Color(0, 255, 200, 180));
    window.draw(tr1); window.draw(tr2);
    sf::RectangleShape bl1({40.f, 3.f}); bl1.setPosition({30.f, SCREEN_H - 33.f});
    sf::RectangleShape bl2({3.f, 40.f}); bl2.setPosition({30.f, SCREEN_H - 70.f});
    bl1.setFillColor(sf::Color(0, 255, 200, 180));
    bl2.setFillColor(sf::Color(0, 255, 200, 180));
    window.draw(bl1); window.draw(bl2);
    sf::RectangleShape br1({40.f, 3.f}); br1.setPosition({SCREEN_W - 70.f, SCREEN_H - 33.f});
    sf::RectangleShape br2({3.f, 40.f}); br2.setPosition({SCREEN_W - 33.f, SCREEN_H - 70.f});
    br1.setFillColor(sf::Color(0, 255, 200, 180));
    br2.setFillColor(sf::Color(0, 255, 200, 180));
    window.draw(br1); window.draw(br2);
}

void Game::drawMenuTitle(sf::RenderWindow& window, float t) {
    float pulse = 0.85f + 0.15f * std::sin(t * 2.5f);
    sf::Text glow(font, "SPACE INVADERS", 54);
    glow.setFillColor(sf::Color(0, 200, 255, (int)(60 * pulse)));
    glow.setPosition({SCREEN_W / 2.f - glow.getGlobalBounds().size.x / 2.f - 2.f, 48.f});
    window.draw(glow);
    sf::Text title(font, "SPACE INVADERS", 54);
    title.setFillColor(sf::Color((int)(200 * pulse), (int)(240 * pulse), 255));
    title.setOutlineColor(sf::Color(0, 100, 200));
    title.setOutlineThickness(2.f);
    title.setPosition({SCREEN_W / 2.f - title.getGlobalBounds().size.x / 2.f, 50.f});
    window.draw(title);
    sf::Text sub(font, "OOP GALACTIC DEFENSE  |  BCS 2D  |  SPRING 2026", 13);
    sub.setFillColor(sf::Color(0, 160, 220, 200));
    sub.setPosition({SCREEN_W / 2.f - sub.getGlobalBounds().size.x / 2.f, 112.f});
    window.draw(sub);
    sf::RectangleShape div({600.f, 2.f});
    div.setOrigin({300.f, 1.f});
    div.setPosition({SCREEN_W / 2.f, 136.f});
    div.setFillColor(sf::Color(0, 200, 255, (int)(180 * pulse)));
    window.draw(div);
}

void Game::handleMenu(float dt) {
    menuAnimTime += dt;
    starfield->update(dt);
    static bool upHeld = false, downHeld = false, enterHeld = false, escHeld = false;
    bool upDn   = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up);
    bool downDn = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down);
    bool entDn  = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter);
    bool escDn  = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape);

    if (inModeSelect) {
        if (upDn   && !upHeld)   { modeSelection = 0; upHeld   = true; }
        if (!upDn)   upHeld   = false;
        if (downDn && !downHeld) { modeSelection = 1; downHeld = true; }
        if (!downDn) downHeld = false;
        if (entDn && !enterHeld) {
            enterHeld = true;
            inModeSelect = false;
            pendingMode = (modeSelection == 0) ? GameMode::ARCADE : GameMode::SURVIVAL;
            gsm.setMode(pendingMode);
            playerName = "";
            gsm.setState(GameState::USERNAME_INPUT);
        }
        if (!entDn) enterHeld = false;
        if (escDn && !escHeld) { escHeld = true; inModeSelect = false; }
        if (!escDn) escHeld = false;
        return;
    }
    if (upDn && !upHeld) { menuSelection = (menuSelection - 1 + 6) % 6; upHeld = true; }
    if (!upDn) upHeld = false;
    if (downDn && !downHeld) { menuSelection = (menuSelection + 1) % 6; downHeld = true; }
    if (!downDn) downHeld = false;
    if (entDn && !enterHeld) {
        enterHeld = true;
        switch (menuSelection) {
        case 0: inModeSelect = true; modeSelection = 0; break;
        case 1: gsm.setState(GameState::HIGH_SCORE); break;
        case 2: gsm.setState(GameState::INSTRUCTIONS); break;
        case 3: gsm.setState(GameState::CREDITS); break;
        case 4: gsm.setState(GameState::SETTINGS); break;
        case 5: window.close(); break;
        }
    }
    if (!entDn) enterHeld = false;
}

void Game::renderMenu() {
    drawMenuBackground(window, menuAnimTime);
    drawMenuTitle(window, menuAnimTime);
    if (inModeSelect) {
        sf::RectangleShape overlay({460.f, 180.f});
        overlay.setOrigin({230.f, 90.f});
        overlay.setPosition({SCREEN_W / 2.f, SCREEN_H / 2.f});
        overlay.setFillColor(sf::Color(5, 15, 35, 220));
        overlay.setOutlineColor(sf::Color(0, 200, 255, 180));
        overlay.setOutlineThickness(2.f);
        window.draw(overlay);
        sf::Text hdr(font, "SELECT MODE", 22);
        hdr.setFillColor(sf::Color(0, 220, 255));
        hdr.setPosition({SCREEN_W / 2.f - hdr.getGlobalBounds().size.x / 2.f, SCREEN_H / 2.f - 72.f});
        window.draw(hdr);
        const char* modes[] = { "ARCADE MODE", "SURVIVAL MODE" };
        const char* descs[] = { "3 Levels + Boss Fights", "Endless Escalating Waves" };
        for (int i = 0; i < 2; i++) {
            bool sel = (modeSelection == i);
            if (sel) {
                sf::RectangleShape box({380.f, 52.f});
                box.setOrigin({190.f, 26.f});
                box.setPosition({SCREEN_W / 2.f, SCREEN_H / 2.f - 18.f + i * 66.f});
                box.setFillColor(sf::Color(0, 80, 140, 120));
                box.setOutlineColor(sf::Color(0, 200, 255));
                box.setOutlineThickness(1.5f);
                window.draw(box);
                drawArrow(window, SCREEN_W / 2.f - 190.f + 6.f, SCREEN_H / 2.f - 27.f + i * 66.f, sf::Color(0, 255, 200));
            }
            sf::Text mo(font, modes[i], 24);
            mo.setFillColor(sel ? sf::Color(0, 255, 200) : sf::Color(140, 180, 200));
            mo.setPosition({SCREEN_W / 2.f - mo.getGlobalBounds().size.x / 2.f, SCREEN_H / 2.f - 30.f + i * 66.f});
            window.draw(mo);
            sf::Text ds(font, descs[i], 13);
            ds.setFillColor(sel ? sf::Color(0, 200, 160) : sf::Color(80, 100, 120));
            ds.setPosition({SCREEN_W / 2.f - ds.getGlobalBounds().size.x / 2.f, SCREEN_H / 2.f - 4.f + i * 66.f});
            window.draw(ds);
        }
        window.display();
        return;
    }
    const char* items[] = { "START GAME", "HIGH SCORE", "INSTRUCTION", "CREDITS", "SETTINGS", "EXIT" };
    sf::RectangleShape panel({340.f, 340.f});
    panel.setOrigin({170.f, 170.f});
    panel.setPosition({SCREEN_W / 2.f, 390.f});
    panel.setFillColor(sf::Color(4, 12, 28, 200));
    panel.setOutlineColor(sf::Color(0, 140, 200, 80));
    panel.setOutlineThickness(1.f);
    window.draw(panel);
    float startY = 210.f;
    float spacing = 48.f;
    for (int i = 0; i < 6; i++) {
        bool sel = (menuSelection == i);
        if (sel) {
            sf::RectangleShape bar({340.f, 46.f});
            bar.setOrigin({170.f, 23.f});
            bar.setPosition({SCREEN_W / 2.f, startY + i * spacing + 12.f});
            bar.setFillColor(sf::Color(0, 80, 160, 80));
            bar.setOutlineColor(sf::Color(0, 220, 255, 200));
            bar.setOutlineThickness(1.5f);
            window.draw(bar);
            float bounce = 3.f * std::sin(menuAnimTime * 6.f);
            drawArrow(window, SCREEN_W / 2.f - 158.f + bounce, startY + i * spacing - 3.f, sf::Color(0, 255, 200));
        }
        sf::Text item(font, items[i], 28);
        if (sel) {
            float p = 0.8f + 0.2f * std::sin(menuAnimTime * 4.f);
            item.setFillColor(sf::Color((int)(0*p), (int)(230*p), (int)(255*p)));
            item.setOutlineColor(sf::Color(0, 100, 150));
            item.setOutlineThickness(1.5f);
        } else {
            item.setFillColor(sf::Color(160, 190, 210));
        }
        item.setPosition({SCREEN_W / 2.f - item.getGlobalBounds().size.x / 2.f, startY + i * spacing});
        window.draw(item);
    }
    if (pressStartActive) {
        float alpha = 127.f + 128.f * std::sin(menuAnimTime * 3.5f);
        sf::Text ps(font, "PRESS START", 20);
        ps.setFillColor(sf::Color(0, 255, 200, (int)alpha));
        ps.setPosition({SCREEN_W / 2.f - ps.getGlobalBounds().size.x / 2.f, SCREEN_H - 70.f});
        window.draw(ps);
    }
    sf::Text hint(font, "UP/DOWN: Navigate     ENTER: Select", 13);
    hint.setFillColor(sf::Color(60, 100, 140));
    hint.setPosition({SCREEN_W / 2.f - hint.getGlobalBounds().size.x / 2.f, SCREEN_H - 36.f});
    window.draw(hint);
    window.display();
}

void Game::startGame(GameMode mode) {
    delete player; player = nullptr;
    delete boss;   boss   = nullptr;
    for (int i = 0; i < pbCount;  i++) { delete playerBullets[i]; playerBullets[i] = nullptr; } pbCount = 0;
    for (int i = 0; i < ebCount;  i++) { delete enemyBullets[i];  enemyBullets[i]  = nullptr; } ebCount = 0;
    for (int i = 0; i < eCount;   i++) { delete enemies[i];       enemies[i]       = nullptr; } eCount  = 0;
    for (int i = 0; i < puCount;  i++) { delete powerUps[i];      powerUps[i]      = nullptr; } puCount = 0;
    for (int i = 0; i < astCount; i++) { delete asteroids[i];     asteroids[i]     = nullptr; } astCount= 0;
    for (int i = 0; i < expCount; i++) { delete explosions[i];    explosions[i]    = nullptr; } expCount= 0;
    score = 0; multiplier = 1; multiplierTimer = 0;
    wave = 1; arcadeLevel = 1;
    lastMultiplier = 1; multiplierPopupTimer = 0;
    shieldPulseTimer = 0;
    isWaveTransition = false; waveCountdownTimer = 0;
    lastPlayedMode = mode;
    lastWaveReached = 1; lastLevelReached = 1;
    pressStartActive = false;
    for (int i = 0; i < MAX_POPUPS; i++) scorePopups[i].active = false;
    bossActive = false; bossDefeated = false;
    waveSpawning = false; enemiesLeftToSpawn = 0;
    player = new Player(SCREEN_W / 2.f, SCREEN_H - 80.f);
    gsm.setState(GameState::PLAYING);
    spawnAsteroids();
    spawnWave();
    AudioManager::getInstance().playGameMusic();
}

void Game::drawHealthBar(sf::RenderWindow& window) {
    if (!player) return;
    int lives = player->getLives();
    float barW = 160.f, barH = 14.f, bx = 8.f, by = 50.f;
    sf::RectangleShape bg({barW, barH});
    bg.setPosition({bx, by});
    bg.setFillColor(sf::Color(30, 10, 10, 200));
    bg.setOutlineColor(sf::Color(140, 30, 30, 180));
    bg.setOutlineThickness(1.f);
    window.draw(bg);
    float ratio = (float)lives / (float)PLAYER_MAX_HP;
    if (ratio < 0) ratio = 0; if (ratio > 1) ratio = 1;
    sf::Color fillCol;
    if (ratio > 0.6f) fillCol = sf::Color(60, 220, 60);
    else if (ratio > 0.3f) fillCol = sf::Color(255, 200, 0);
    else fillCol = sf::Color(255, 50, 50);
    if (lives == 1) {
        float p = 0.6f + 0.4f * std::sin(menuAnimTime * 8.f);
        fillCol.r = (std::uint8_t)(fillCol.r * p);
        fillCol.g = (std::uint8_t)(fillCol.g * p);
    }
    sf::RectangleShape fill({barW * ratio, barH});
    fill.setPosition({bx, by});
    fill.setFillColor(fillCol);
    window.draw(fill);
    for (int seg = 1; seg < PLAYER_MAX_HP; seg++) {
        float sx = bx + (barW / PLAYER_MAX_HP) * seg;
        sf::RectangleShape mark({2.f, barH});
        mark.setPosition({sx, by});
        mark.setFillColor(sf::Color(0, 0, 0, 160));
        window.draw(mark);
    }
    sf::Text label(font, "HP", 12);
    label.setFillColor(sf::Color(200, 200, 200));
    label.setPosition({bx, by - 15.f});
    window.draw(label);
    std::string hpStr = intToStr(lives) + " / " + intToStr(PLAYER_MAX_HP);
    sf::Text hpNum(font, hpStr, 11);
    hpNum.setFillColor(sf::Color(200, 200, 200));
    hpNum.setPosition({bx + barW + 6.f, by});
    window.draw(hpNum);
}

void Game::handlePlaying(float dt) {
    menuAnimTime += dt;
    static bool escWasHeld = false;
    bool escDn = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape);
    if (escDn && !escWasHeld) { gsm.setState(GameState::PAUSED); escWasHeld = true; return; }
    if (!escDn) escWasHeld = false;
    static bool nHeld = false;
    bool nDn = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::N);
    if (nDn && !nHeld && player->getEMPs() > 0) { activateEMP(); nHeld = true; }
    if (!nDn) nHeld = false;
    if (shakeTimer > 0) {
        shakeTimer -= dt;
        float mag = shakeMagnitude * (shakeTimer / 0.3f);
        shakeOffset = { (float)(std::rand() % ((int)(mag * 2) + 1)) - mag,
                        (float)(std::rand() % ((int)(mag * 2) + 1)) - mag };
    } else shakeOffset = {0,0};
    starfield->update(dt);
    player->update(dt);
    if (isWaveTransition) {
        waveCountdownTimer -= dt;
        if (waveCountdownTimer <= 0.f) {
            isWaveTransition = false;
            wave = pendingWaveNumber;
            spawnWave();
        }
        updateScorePopups(dt);
        return;
    }
    int cnt = 0;
    Bullet** newBullets = player->shoot(cnt);
    for (int i = 0; i < cnt && pbCount < MAX_BULLETS; i++)
        playerBullets[pbCount++] = newBullets[i];
    delete[] newBullets;
    for (int i = 0; i < eCount; i++) {
        if (!enemies[i] || !enemies[i]->isAlive()) continue;
        enemies[i]->update(dt);
        if (enemies[i]->getPosition().y > SCREEN_H + 40) { enemies[i]->destroy(); continue; }
        Bullet* eb = enemies[i]->shoot();
        if (eb && ebCount < MAX_BULLETS) enemyBullets[ebCount++] = eb;
    }
    if (bossActive && boss && boss->isAlive()) {
        boss->update(dt);
        int bc = 0;
        Bullet** bBullets = boss->attack(bc);
        for (int i = 0; i < bc && ebCount < MAX_BULLETS; i++)
            enemyBullets[ebCount++] = bBullets[i];
        delete[] bBullets;
        Mothership* ms = dynamic_cast<Mothership*>(boss);
        if (ms) {
            ms->setPlayerX(player->getPosition().x);
            int sc = 0;
            Enemy** spawned = ms->spawnEnemies(sc);
            for (int i = 0; i < sc && eCount < MAX_ENEMIES; i++)
                enemies[eCount++] = spawned[i];
            delete[] spawned;
            if (ms->isLaserFiring() && !player->isInvincible()) {
                float lY = ms->getLaserY();
                float pY = player->getPosition().y;
                if (std::abs(pY - lY) < 20.f) {
                    player->takeDamage(1);
                    if (player->wasHitThisFrame()) { shakeTimer = 0.3f; shakeMagnitude = 8; }
                }
            }
        }
        AudioManager::getInstance().playBossMusic();
    }
    int oldShieldHits = player ? player->getShieldHits() : 0;
    int livesLost = 0;
    TwinCannons* twin = dynamic_cast<TwinCannons*>(boss);
    int gained = CollisionManager::runAll(
        player, playerBullets, pbCount, enemyBullets, ebCount,
        enemies, eCount, boss, twin,
        powerUps, puCount, asteroids, astCount,
        explosions, expCount, MAX_EXPLOSIONS, livesLost
    );
    if (gained > 0) { score += gained * multiplier; updateMultiplier(dt); }
    int newShieldHits = player ? player->getShieldHits() : 0;
    if (newShieldHits > oldShieldHits) { shieldPulseTimer = 0.5f; shieldPulseMaxRadius = 60.f; }
    if (shieldPulseTimer > 0) shieldPulseTimer -= dt;
    updateScorePopups(dt);
    if (multiplierPopupTimer > 0) multiplierPopupTimer -= dt;
    if (player->wasHitThisFrame()) {
        shakeTimer = 0.3f; shakeMagnitude = 10.f;
        multiplier = 1;
        player->clearHitFlag();
    }
    if (bossActive && boss && boss->isDefeated()) {
        bossActive  = false;
        bossDefeated= true;
        addExplosion(boss->getPosition().x, boss->getPosition().y, sf::Color(255,200,80), 60, 150);
        score += 5000;
        AudioManager::getInstance().playGameMusic();
        if (gsm.getMode() == GameMode::ARCADE) bossTransitionTimer = 2.f;
    }
    for (int i = 0; i < pbCount; i++) if (playerBullets[i]) playerBullets[i]->update(dt);
    for (int i = 0; i < ebCount; i++) if (enemyBullets[i])  enemyBullets[i]->update(dt);
    for (int i = 0; i < puCount;  i++) if (powerUps[i])  powerUps[i]->update(dt);
    for (int i = 0; i < astCount; i++) if (asteroids[i]) asteroids[i]->update(dt);
    for (int i = 0; i < expCount; i++) if (explosions[i]) explosions[i]->update(dt);
    compactBullets(playerBullets, pbCount);
    compactBullets(enemyBullets, ebCount);
    compactEnemies(enemies, eCount);
    compactPowerUps(powerUps, puCount);
    compactExplosions();
    multResetTimer -= dt;
    if (multResetTimer < 0 && multiplier > 1) multiplier = 1;
    if (!bossActive && !bossDefeated) checkWaveComplete();
    if (bossDefeated && gsm.getMode() == GameMode::ARCADE) {
        bossTransitionTimer -= dt;
        if (bossTransitionTimer <= 0) { bossDefeated = false; advanceLevel(); }
    }
    if (player->isDead()) {
        saveHighScore(score);
        gsm.setState(GameState::GAME_OVER);
        AudioManager::getInstance().stopMusic();
    }
}

void Game::updateMultiplier(float dt) {
    multResetTimer = 3.0f;
    int oldMult = multiplier;
    if (multiplier == 1) multiplier = 2;
    else if (multiplier == 2) multiplier = 4;
    if (multiplier != oldMult) { multiplierPopupTimer = 1.5f; lastMultiplier = multiplier; }
}

void Game::checkWaveComplete() {
    if (bossActive) return;
    bool allDead = true;
    for (int i = 0; i < eCount; i++) {
        if (enemies[i] && enemies[i]->isAlive()) { allDead = false; break; }
    }
    if (!allDead || waveSpawning || enemiesLeftToSpawn > 0) return;
    if (gsm.getMode() == GameMode::ARCADE) {
        bossActive = true; bossDefeated = false;
        delete boss; boss = nullptr;
        spawnBoss(arcadeLevel);
        AudioManager::getInstance().playBossMusic();
    } else {
        pendingWaveNumber = wave + 1;
        isWaveTransition = true;
        waveCountdownTimer = 2.0f;
        lastWaveReached = pendingWaveNumber;
    }
}

void Game::spawnWave() {
    for (int i = 0; i < eCount; i++) { delete enemies[i]; enemies[i] = nullptr; } eCount = 0;
    int count = 8 + (wave - 1) * 2;
    if (count > MAX_ENEMIES) count = MAX_ENEMIES;
    float spd = 1.0f + (wave - 1) * 0.05f;
    float fire= 1.0f + (wave - 1) * 0.10f;
    for (int i = 0; i < count; i++) {
        float x = (float)(std::rand() % (SCREEN_W - 80) + 40);
        float y = -40.f - i * 40.f;
        Enemy* en = nullptr;
        int r = std::rand() % 100;
        int type = 0;
        if (wave >= 8 && r < 20) type = 2;
        else if (wave >= 5 && r < 40) type = 1;
        if (type == 0)      en = new Drone(x, y, spd, fire);
        else if (type == 1) en = new Viper(x, y, spd, fire);
        else                en = new Seeker(x, y, player->getPosition().x, spd);
        if (eCount < MAX_ENEMIES) enemies[eCount++] = en;
    }
    waveSpawning = false; enemiesLeftToSpawn = 0;
}

void Game::spawnBoss(int level) {
    delete boss; boss = nullptr;
    if (level == 1)      boss = new Cruiser((float)SCREEN_W);
    else if (level == 2) boss = new TwinCannons((float)SCREEN_W);
    else                 boss = new Mothership((float)SCREEN_W, player->getPosition().x);
}

void Game::advanceLevel() {
    arcadeLevel++;
    lastLevelReached = arcadeLevel;
    if (arcadeLevel > 3) {
        saveHighScore(score);
        gsm.setState(GameState::WIN);
        AudioManager::getInstance().stopMusic();
        return;
    }
    wave = arcadeLevel;
    spawnWave();
}

void Game::spawnAsteroids() {
    for (int i = 0; i < astCount; i++) { delete asteroids[i]; asteroids[i] = nullptr; } astCount = 0;
    int n = 3 + std::rand() % 3;
    for (int i = 0; i < n && astCount < MAX_ASTEROIDS; i++) {
        float sz  = (float)(std::rand() % 25 + 15);
        float spd = (float)(std::rand() % 30 + 20);
        float x   = (float)(std::rand() % (SCREEN_W - 80) + 40);
        float y   = (float)(SCREEN_H * 0.4f) + (float)(std::rand() % (int)(SCREEN_H * 0.55f));
        asteroids[astCount++] = new Asteroid(x, y, sz, spd);
    }
}

void Game::addExplosion(float x, float y, sf::Color col, int cnt, float r) {
    if (expCount < MAX_EXPLOSIONS)
        explosions[expCount++] = new Explosion(x, y, col, cnt, r);
}

void Game::activateEMP() {
    player->useEMP();
    for (int i = 0; i < eCount; i++) {
        if (enemies[i] && enemies[i]->isAlive()) {
            addExplosion(enemies[i]->getPosition().x, enemies[i]->getPosition().y,
                         sf::Color(255,220,60), 12, 50);
            score += 50 * multiplier;
            enemies[i]->destroy();
        }
    }
    if (boss && boss->isAlive()) boss->takeDamage(60);
    shakeTimer = 0.2f; shakeMagnitude = 15.f;
}

void Game::tryDropPowerUp(float x, float y) {
    if (puCount >= MAX_POWERUPS) return;
    if ((std::rand() % 100) >= 15) return;
    int t = std::rand() % 3;
    PowerUpType pt = (t == 0) ? PowerUpType::SPREAD :
                     (t == 1) ? PowerUpType::PIERCING : PowerUpType::SHIELD;
    powerUps[puCount++] = new PowerUp(x, y, pt);
}

void Game::tryDropEMP(float x, float y) {
    if (puCount >= MAX_POWERUPS) return;
    if ((std::rand() % 100) >= 5) return;
    powerUps[puCount++] = new PowerUp(x, y, PowerUpType::EMP);
}

void Game::compactBullets(Bullet** arr, int& count) {
    int w = 0;
    for (int i = 0; i < count; i++) {
        if (arr[i] && arr[i]->isAlive()) arr[w++] = arr[i];
        else { delete arr[i]; arr[i] = nullptr; }
    }
    for (int i = w; i < count; i++) arr[i] = nullptr;
    count = w;
}
void Game::compactEnemies(Enemy** arr, int& count) {
    int w = 0;
    for (int i = 0; i < count; i++) {
        if (arr[i] && arr[i]->isAlive()) {
            if (arr[i]->justDied()) { tryDropPowerUp(arr[i]->getPosition().x, arr[i]->getPosition().y); tryDropEMP(arr[i]->getPosition().x, arr[i]->getPosition().y); arr[i]->clearDiedFlag(); }
            arr[w++] = arr[i];
        } else {
            if (arr[i] && arr[i]->justDied()) {
                addScorePopup(arr[i]->getPosition().x, arr[i]->getPosition().y, 100, multiplier);
                tryDropPowerUp(arr[i]->getPosition().x, arr[i]->getPosition().y);
                tryDropEMP(arr[i]->getPosition().x, arr[i]->getPosition().y);
            }
            delete arr[i]; arr[i] = nullptr;
        }
    }
    for (int i = w; i < count; i++) arr[i] = nullptr;
    count = w;
}
void Game::compactPowerUps(PowerUp** arr, int& count) {
    int w = 0;
    for (int i = 0; i < count; i++) {
        if (arr[i] && arr[i]->isAlive()) arr[w++] = arr[i];
        else { delete arr[i]; arr[i] = nullptr; }
    }
    for (int i = w; i < count; i++) arr[i] = nullptr;
    count = w;
}
void Game::compactExplosions() {
    int w = 0;
    for (int i = 0; i < expCount; i++) {
        if (explosions[i] && !explosions[i]->isDone()) explosions[w++] = explosions[i];
        else { delete explosions[i]; explosions[i] = nullptr; }
    }
    for (int i = w; i < expCount; i++) explosions[i] = nullptr;
    expCount = w;
}

std::string Game::weaponName(WeaponType w) {
    switch (w) {
    case WeaponType::STANDARD: return "STANDARD";
    case WeaponType::SPREAD:   return "SPREAD";
    case WeaponType::PIERCING: return "PIERCING";
    }
    return "STANDARD";
}

void Game::renderPlaying() {
    window.clear(sf::Color(5, 5, 18));
    sf::View originalView = window.getView();
    sf::View shakeView = originalView;
    shakeView.move(shakeOffset);
    window.setView(shakeView);
    starfield->draw(window);
    for (int i = 0; i < astCount; i++) if (asteroids[i])  asteroids[i]->draw(window);
    for (int i = 0; i < puCount;  i++) if (powerUps[i])   powerUps[i]->draw(window);
    for (int i = 0; i < eCount;   i++) if (enemies[i] && enemies[i]->isAlive()) enemies[i]->draw(window);
    if (bossActive && boss && boss->isAlive()) boss->draw(window);
    for (int i = 0; i < pbCount;  i++) if (playerBullets[i] && playerBullets[i]->isAlive()) playerBullets[i]->draw(window);
    for (int i = 0; i < ebCount;  i++) if (enemyBullets[i]  && enemyBullets[i]->isAlive())  enemyBullets[i]->draw(window);
    for (int i = 0; i < expCount; i++) if (explosions[i]) explosions[i]->draw(window);
    if (player) player->draw(window);
    window.setView(originalView);
    int disp_wave = (gsm.getMode() == GameMode::ARCADE) ? arcadeLevel : wave;
    hud->draw(window, score, multiplier,
              player ? player->getLives() : 0,
              player ? weaponName(player->getWeapon()) : "STANDARD",
              player ? player->getShieldHits() : 0,
              player ? player->getEMPs() : 0,
              player ? player->getDashCooldownRatio() : 1.f,
              disp_wave, gsm.getMode());
    drawHealthBar(window);
    if (bossActive && boss && boss->isAlive()) {
        std::string bname;
        if (dynamic_cast<Cruiser*>(boss))      bname = "BOSS: THE CRUISER";
        else if (dynamic_cast<TwinCannons*>(boss)) bname = "BOSS: TWIN CANNONS";
        else                                   bname = "BOSS: THE MOTHERSHIP";
        sf::Text bl(font, bname, 14);
        bl.setFillColor(sf::Color(255, 100, 100));
        bl.setPosition({SCREEN_W / 2.f - bl.getGlobalBounds().size.x / 2.f, 34.f});
        window.draw(bl);
    }
    drawScorePopups(window);
    if (multiplierPopupTimer > 0) {
        float scale = 1.0f + 0.3f * std::sin(multiplierPopupTimer * 8.f);
        int alpha = (int)(255 * (multiplierPopupTimer / 1.5f));
        if (alpha > 255) alpha = 255;
        sf::Text mp(font, intToStr(lastMultiplier) + "x MULTIPLIER!", (int)(36 * scale));
        mp.setFillColor(sf::Color(255, 220, 60, alpha));
        mp.setOutlineColor(sf::Color(200, 120, 0, alpha));
        mp.setOutlineThickness(2.f);
        sf::FloatRect mb = mp.getLocalBounds();
        mp.setOrigin({mb.position.x + mb.size.x/2.f, mb.position.y + mb.size.y/2.f});
        mp.setPosition({SCREEN_W/2.f, SCREEN_H/2.f - 60.f});
        window.draw(mp);
    }
    if (shieldPulseTimer > 0 && player) {
        float t = shieldPulseTimer / 0.5f;
        float radius = shieldPulseMaxRadius * (1.0f - t) + 30.f;
        int alpha = (int)(200 * t);
        sf::CircleShape pulse(radius);
        pulse.setOrigin({radius, radius});
        pulse.setPosition(player->getPosition());
        pulse.setFillColor(sf::Color::Transparent);
        pulse.setOutlineThickness(3.f);
        pulse.setOutlineColor(sf::Color(80, 255, 200, alpha));
        window.draw(pulse);
    }
    if (isWaveTransition) {
        float t = waveCountdownTimer / 2.0f;
        float fadeAlpha;
        if (t > 0.85f) fadeAlpha = (1.0f - t) / 0.15f;
        else if (t < 0.15f) fadeAlpha = t / 0.15f;
        else fadeAlpha = 1.0f;
        int alpha = (int)(255 * fadeAlpha);
        std::string waveLabel = (gsm.getMode() == GameMode::SURVIVAL)
            ? "WAVE " + intToStr(pendingWaveNumber)
            : "LEVEL " + intToStr(pendingWaveNumber);
        sf::Text wt(font, waveLabel + " INCOMING", 48);
        wt.setFillColor(sf::Color(0, 220, 255, alpha));
        wt.setOutlineColor(sf::Color(0, 80, 140, alpha));
        wt.setOutlineThickness(2.f);
        sf::FloatRect wb = wt.getLocalBounds();
        wt.setOrigin({wb.position.x + wb.size.x/2.f, wb.position.y + wb.size.y/2.f});
        wt.setPosition({SCREEN_W/2.f, SCREEN_H/2.f});
        window.draw(wt);
    }
    window.display();
}

void Game::handlePaused(float dt) {
    static bool upHeld = false, downHeld = false, enterHeld = false, escHeldP = false;
    bool upDn   = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up);
    bool downDn = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down);
    bool entDn  = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter);
    bool escDn  = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape);
    bool pDn    = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::P);
    if (upDn   && !upHeld)   { pauseSelection = 0; upHeld   = true; }
    if (!upDn)   upHeld   = false;
    if (downDn && !downHeld) { pauseSelection = 1; downHeld = true; }
    if (!downDn) downHeld = false;
    if (entDn && !enterHeld) {
        enterHeld = true;
        if (pauseSelection == 0) gsm.setState(GameState::PLAYING);
        else {
            gsm.setState(GameState::MENU);
            for (int i = 0; i < MAX_POPUPS; i++) scorePopups[i].active = false;
            loadLeaderboard();
            AudioManager::getInstance().playMenuMusic();
        }
    }
    if (!entDn) enterHeld = false;
    if ((escDn || pDn) && !escHeldP) { escHeldP = true; gsm.setState(GameState::PLAYING); }
    if (!escDn) escHeldP = false;
}

void Game::renderPaused() {
    renderPlaying();
    sf::RectangleShape overlay({(float)SCREEN_W, (float)SCREEN_H});
    overlay.setFillColor(sf::Color(0, 0, 0, 170));
    window.draw(overlay);
    sf::RectangleShape box({340.f, 220.f});
    box.setOrigin({170.f, 110.f});
    box.setPosition({SCREEN_W / 2.f, SCREEN_H / 2.f});
    box.setFillColor(sf::Color(4, 12, 30, 220));
    box.setOutlineColor(sf::Color(0, 180, 255, 200));
    box.setOutlineThickness(2.f);
    window.draw(box);
    sf::Text t(font, "PAUSED", 44);
    t.setFillColor(sf::Color(0, 220, 255));
    t.setPosition({SCREEN_W / 2.f - t.getGlobalBounds().size.x / 2.f, SCREEN_H / 2.f - 90.f});
    window.draw(t);
    int pw = (gsm.getMode() == GameMode::ARCADE) ? arcadeLevel : wave;
    std::string pauseInfo = "SCORE: " + intToStr(score) + "    x" + intToStr(multiplier)
        + "    " + (gsm.getMode() == GameMode::SURVIVAL ? "WAVE: " : "LEVEL: ") + intToStr(pw);
    sf::Text pi(font, pauseInfo, 16);
    pi.setFillColor(sf::Color(160, 200, 220));
    sf::FloatRect pib = pi.getLocalBounds();
    pi.setOrigin({pib.position.x + pib.size.x/2.f, pib.position.y + pib.size.y/2.f});
    pi.setPosition({SCREEN_W/2.f, SCREEN_H/2.f - 55.f});
    window.draw(pi);
    sf::Text pr(font, "Press P to Resume", 13);
    pr.setFillColor(sf::Color(80, 120, 160));
    sf::FloatRect prb = pr.getLocalBounds();
    pr.setOrigin({prb.position.x + prb.size.x/2.f, prb.position.y + prb.size.y/2.f});
    pr.setPosition({SCREEN_W/2.f, SCREEN_H/2.f + 80.f});
    window.draw(pr);
    const char* opts[] = { "RESUME", "QUIT TO MENU" };
    for (int i = 0; i < 2; i++) {
        sf::Text o(font, opts[i], 26);
        o.setFillColor(pauseSelection == i ? sf::Color(0, 255, 200) : sf::Color(140, 180, 200));
        o.setPosition({SCREEN_W / 2.f - o.getGlobalBounds().size.x / 2.f, SCREEN_H / 2.f - 10.f + i * 56.f});
        if (pauseSelection == i)
            drawArrow(window, SCREEN_W / 2.f - 140.f, SCREEN_H / 2.f - 16.f + i * 56.f, sf::Color(0, 255, 200));
        window.draw(o);
    }
    window.display();
}

void Game::handleGameOver(float dt) {
    menuAnimTime += dt;
    starfield->update(dt);
    static bool enterHeld = false;
    bool entDn = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter);
    if (entDn && !enterHeld) {
        enterHeld = true;
        updatePlayerScore(playerName, score);
        gsm.setState(GameState::MENU);
        for (int i = 0; i < MAX_POPUPS; i++) scorePopups[i].active = false;
        loadLeaderboard();
        AudioManager::getInstance().playMenuMusic();
    }
    if (!entDn) enterHeld = false;
}

void Game::renderGameOver() {
    renderPlaying();
    sf::RectangleShape overlay({(float)SCREEN_W, (float)SCREEN_H});
    overlay.setFillColor(sf::Color(20, 0, 0, 160));
    window.draw(overlay);
    sf::Text t(font, "GAME OVER", 64);
    t.setFillColor(sf::Color(255, 60, 60));
    t.setPosition({SCREEN_W / 2.f - t.getGlobalBounds().size.x / 2.f, 200.f});
    window.draw(t);
    std::string reached;
    if (lastPlayedMode == GameMode::ARCADE)
        reached = "REACHED LEVEL " + intToStr(lastLevelReached);
    else
        reached = "SURVIVED TO WAVE " + intToStr(lastWaveReached);
    sf::Text sub(font, reached, 22);
    sub.setFillColor(sf::Color(255, 160, 160));
    sub.setPosition({SCREEN_W / 2.f - sub.getGlobalBounds().size.x / 2.f, 300.f});
    window.draw(sub);
    sf::Text s(font, "FINAL SCORE: " + intToStr(score), 32);
    s.setFillColor(sf::Color::White);
    s.setPosition({SCREEN_W / 2.f - s.getGlobalBounds().size.x / 2.f, 340.f});
    window.draw(s);
    sf::Text esc(font, "Press ENTER to return to Menu", 18);
    float p = 0.5f + 0.5f * std::sin(menuAnimTime * 4.f);
    esc.setFillColor(sf::Color(255, 255, 255, (int)(150 + 105 * p)));
    esc.setPosition({SCREEN_W / 2.f - esc.getGlobalBounds().size.x / 2.f, 500.f});
    window.draw(esc);
    window.display();
}

void Game::handleWin(float dt) {
    menuAnimTime += dt;
    starfield->update(dt);
    static bool enterHeld = false;
    bool entDn = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter);
    if (entDn && !enterHeld) {
        enterHeld = true;
        updatePlayerScore(playerName, score);
        gsm.setState(GameState::MENU);
        for (int i = 0; i < MAX_POPUPS; i++) scorePopups[i].active = false;
        loadLeaderboard();
        AudioManager::getInstance().playMenuMusic();
    }
    if (!entDn) enterHeld = false;
}

void Game::renderWin() {
    renderPlaying();
    sf::RectangleShape overlay({(float)SCREEN_W, (float)SCREEN_H});
    overlay.setFillColor(sf::Color(0, 15, 30, 190));
    window.draw(overlay);
    sf::Text t(font, "MISSION ACCOMPLISHED", 56);
    t.setFillColor(sf::Color(0, 255, 180));
    t.setOutlineColor(sf::Color(0, 100, 80));
    t.setOutlineThickness(2.f);
    t.setPosition({SCREEN_W / 2.f - t.getGlobalBounds().size.x / 2.f, 200.f});
    window.draw(t);
    sf::Text sub(font, "THE GALAXY IS SAFE ONCE MORE", 22);
    sub.setFillColor(sf::Color(100, 255, 200));
    sub.setPosition({SCREEN_W / 2.f - sub.getGlobalBounds().size.x / 2.f, 280.f});
    window.draw(sub);
    sf::Text s(font, "FINAL SCORE: " + intToStr(score), 32);
    s.setFillColor(sf::Color::White);
    s.setPosition({SCREEN_W / 2.f - s.getGlobalBounds().size.x / 2.f, 340.f});
    window.draw(s);
    sf::Text esc(font, "Press ENTER to return to Menu", 18);
    float p = 0.5f + 0.5f * std::sin(menuAnimTime * 4.f);
    esc.setFillColor(sf::Color(255, 255, 255, (int)(150 + 105 * p)));
    esc.setPosition({SCREEN_W / 2.f - esc.getGlobalBounds().size.x / 2.f, 500.f});
    window.draw(esc);
    window.display();
}

void Game::handleHighScore(float dt) {
    menuAnimTime += dt;
    starfield->update(dt);
    static bool escHeld = false;
    bool escDn = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape);
    if (escDn && !escHeld) { escHeld = true; gsm.setState(GameState::MENU); }
    if (!escDn) escHeld = false;
}

void Game::renderHighScore() {
    drawMenuBackground(window, menuAnimTime);
    drawMenuTitle(window, menuAnimTime);
    sf::RectangleShape panel({500.f, 440.f});
    panel.setOrigin({250.f, 0.f});
    panel.setPosition({SCREEN_W / 2.f, 180.f});
    panel.setFillColor(sf::Color(4, 12, 30, 220));
    panel.setOutlineColor(sf::Color(0, 160, 255, 180));
    panel.setOutlineThickness(2.f);
    window.draw(panel);
    sf::Text t(font, "TOP PILOTS (PERSISTENT)", 32);
    t.setFillColor(sf::Color(0, 220, 255));
    t.setPosition({SCREEN_W / 2.f - t.getGlobalBounds().size.x / 2.f, 200.f});
    window.draw(t);
    for (int i = 0; i < leaderboardCount; i++) {
        sf::Text r(font, intToStr(i + 1) + ".", 24);
        r.setFillColor(sf::Color(140, 180, 220));
        r.setPosition({250.f, 260.f + i * 32.f});
        window.draw(r);
        sf::Text n(font, leaderboard[i].name, 24);
        n.setFillColor(sf::Color(0, 255, 200));
        n.setPosition({310.f, 260.f + i * 32.f});
        window.draw(n);
        sf::Text s(font, intToStr(leaderboard[i].score), 24);
        s.setFillColor(sf::Color::White);
        s.setPosition({650.f - s.getGlobalBounds().size.x, 260.f + i * 32.f});
        window.draw(s);
    }
    if (leaderboardCount == 0) {
        sf::Text none(font, "NO SCORES YET", 20);
        none.setFillColor(sf::Color(100, 100, 100));
        none.setPosition({SCREEN_W / 2.f - none.getGlobalBounds().size.x / 2.f, 300.f});
        window.draw(none);
    }
    sf::Text esc(font, "Press ESC to return to Menu", 16);
    esc.setFillColor(sf::Color(80, 120, 160));
    esc.setPosition({SCREEN_W / 2.f - esc.getGlobalBounds().size.x / 2.f, 580.f});
    window.draw(esc);
    window.display();
}

void Game::handleInstructions(float dt) {
    menuAnimTime += dt;
    starfield->update(dt);
    static bool escHeld = false;
    bool escDn = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape);
    if (escDn && !escHeld) { escHeld = true; gsm.setState(GameState::MENU); }
    if (!escDn) escHeld = false;
}

void Game::renderInstructions() {
    drawMenuBackground(window, menuAnimTime);
    drawMenuTitle(window, menuAnimTime);
    sf::RectangleShape panel({600.f, 440.f});
    panel.setOrigin({300.f, 0.f});
    panel.setPosition({SCREEN_W / 2.f, 180.f});
    panel.setFillColor(sf::Color(4, 12, 30, 220));
    panel.setOutlineColor(sf::Color(0, 160, 255, 180));
    panel.setOutlineThickness(2.f);
    window.draw(panel);
    sf::Text t(font, "HOW TO PLAY", 32);
    t.setFillColor(sf::Color(0, 220, 255));
    t.setPosition({SCREEN_W / 2.f - t.getGlobalBounds().size.x / 2.f, 200.f});
    window.draw(t);
    auto drawLine = [&](const std::string& str, float y, sf::Color c) {
        sf::Text line(font, str, 18);
        line.setFillColor(c);
        line.setPosition({SCREEN_W / 2.f - line.getGlobalBounds().size.x / 2.f, y});
        window.draw(line);
    };
    drawLine("CONTROLS:", 260.f, sf::Color(0, 255, 200));
    drawLine("Arrow Keys : Move Ship", 290.f, sf::Color::White);
    drawLine("Spacebar   : Fire Weapon", 315.f, sf::Color::White);
    drawLine("E + Arrow  : Dash (Brief Invincibility)", 340.f, sf::Color::White);
    drawLine("N          : Trigger EMP (If Available)", 365.f, sf::Color::White);
    drawLine("ESC        : Pause Game", 390.f, sf::Color::White);
    drawLine("POWER-UPS:", 430.f, sf::Color(0, 255, 200));
    drawLine("S (Green) : Spread Shot    P (Red)  : Piercing Shot", 460.f, sf::Color(220, 220, 220));
    drawLine("O (Blue)  : Shield         N (Gold) : EMP Bomb", 485.f, sf::Color(220, 220, 220));
    drawLine("Kill enemies quickly to build your score multiplier!", 530.f, sf::Color(255, 220, 60));
    sf::Text esc(font, "Press ESC to return to Menu", 16);
    esc.setFillColor(sf::Color(80, 120, 160));
    esc.setPosition({SCREEN_W / 2.f - esc.getGlobalBounds().size.x / 2.f, 580.f});
    window.draw(esc);
    window.display();
}

void Game::handleCredits(float dt) {
    menuAnimTime += dt;
    starfield->update(dt);
    static bool escHeld = false;
    bool escDn = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape);
    if (escDn && !escHeld) { escHeld = true; gsm.setState(GameState::MENU); }
    if (!escDn) escHeld = false;
}

void Game::renderCredits() {
    drawMenuBackground(window, menuAnimTime);
    drawMenuTitle(window, menuAnimTime);
    sf::RectangleShape panel({500.f, 300.f});
    panel.setOrigin({250.f, 0.f});
    panel.setPosition({SCREEN_W / 2.f, 220.f});
    panel.setFillColor(sf::Color(4, 12, 30, 220));
    panel.setOutlineColor(sf::Color(0, 160, 255, 180));
    panel.setOutlineThickness(2.f);
    window.draw(panel);
    sf::Text t(font, "CREDITS", 32);
    t.setFillColor(sf::Color(0, 220, 255));
    t.setPosition({SCREEN_W / 2.f - t.getGlobalBounds().size.x / 2.f, 240.f});
    window.draw(t);
    sf::Text c(font, "Developed for BCS-2D OOP Project", 18);
    c.setFillColor(sf::Color::White);
    c.setPosition({SCREEN_W / 2.f - c.getGlobalBounds().size.x / 2.f, 320.f});
    window.draw(c);
    sf::Text dev(font, "By The Elite Coders Team", 22);
    dev.setFillColor(sf::Color(0, 255, 200));
    dev.setPosition({SCREEN_W / 2.f - dev.getGlobalBounds().size.x / 2.f, 360.f});
    window.draw(dev);
    sf::Text esc(font, "Press ESC to return to Menu", 16);
    esc.setFillColor(sf::Color(80, 120, 160));
    esc.setPosition({SCREEN_W / 2.f - esc.getGlobalBounds().size.x / 2.f, 470.f});
    window.draw(esc);
    window.display();
}

void Game::addScorePopup(float x, float y, int points, int mult) {
    for (int i = 0; i < MAX_POPUPS; i++) {
        if (!scorePopups[i].active) {
            scorePopups[i].active = true;
            scorePopups[i].lifetime = 1.0f;
            scorePopups[i].maxLifetime = 1.0f;
            scorePopups[i].vel = {0.f, -40.f};
            
            if (!scorePopups[i].text) {
                scorePopups[i].text = new sf::Text(font, "", 16);
            }
            
            std::string s = "+" + intToStr(points);
            if (mult > 1) s += " (x" + intToStr(mult) + ")";
            scorePopups[i].text->setString(s);
            scorePopups[i].text->setCharacterSize((mult > 1) ? 20 : 16);
            
            sf::FloatRect bounds = scorePopups[i].text->getLocalBounds();
            scorePopups[i].text->setOrigin({bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f});
            scorePopups[i].text->setPosition({x, y});
            
            scorePopups[i].text->setFillColor(sf::Color(255, 220, 60, 255));
            scorePopups[i].text->setOutlineColor(sf::Color(100, 50, 0, 255));
            scorePopups[i].text->setOutlineThickness(1.5f);
            break;
        }
    }
}

void Game::updateScorePopups(float dt) {
    for (int i = 0; i < MAX_POPUPS; i++) {
        if (!scorePopups[i].active) continue;
        scorePopups[i].lifetime -= dt;
        if (scorePopups[i].lifetime <= 0) {
            scorePopups[i].active = false;
            continue;
        }
        float t = scorePopups[i].lifetime / scorePopups[i].maxLifetime;
        sf::Vector2f pos = scorePopups[i].text->getPosition();
        pos += scorePopups[i].vel * dt;
        scorePopups[i].text->setPosition(pos);
        int alpha = (int)(255 * t);
        
        sf::Color fill = scorePopups[i].text->getFillColor();
        sf::Color outline = scorePopups[i].text->getOutlineColor();
        fill.a = (std::uint8_t)alpha;
        outline.a = (std::uint8_t)alpha;
        
        scorePopups[i].text->setFillColor(fill);
        scorePopups[i].text->setOutlineColor(outline);
    }
}

void Game::drawScorePopups(sf::RenderWindow& window) {
    for (int i = 0; i < MAX_POPUPS; i++) {
        if (scorePopups[i].active && scorePopups[i].text)
            window.draw(*scorePopups[i].text);
    }
}

// ─── Leaderboard System ──────────────────────────────────────────────────────
void Game::loadLeaderboard() {
    leaderboardCount = 0;
    FILE* f = fopen("leaderboard.txt", "r");
    if (!f) return;
    char nameBuf[64];
    int sc;
    while (leaderboardCount < MAX_LEADERBOARD && fscanf(f, "%63s %d", nameBuf, &sc) == 2) {
        leaderboard[leaderboardCount].name = nameBuf;
        leaderboard[leaderboardCount].score = sc;
        leaderboardCount++;
    }
    fclose(f);
    sortLeaderboard();
}

void Game::saveLeaderboard() {
    FILE* f = fopen("leaderboard.txt", "w");
    if (!f) return;
    for (int i = 0; i < leaderboardCount; i++)
        fprintf(f, "%s %d\n", leaderboard[i].name.c_str(), leaderboard[i].score);
    fclose(f);
}

void Game::sortLeaderboard() {
    for (int i = 0; i < leaderboardCount - 1; i++)
        for (int j = 0; j < leaderboardCount - 1 - i; j++)
            if (leaderboard[j].score < leaderboard[j+1].score) {
                HighscoreEntry tmp = leaderboard[j];
                leaderboard[j] = leaderboard[j+1];
                leaderboard[j+1] = tmp;
            }
}

void Game::updatePlayerScore(const std::string& name, int sc) {
    // Update existing entry if higher score
    for (int i = 0; i < leaderboardCount; i++) {
        if (leaderboard[i].name == name) {
            if (sc > leaderboard[i].score)
                leaderboard[i].score = sc;
            sortLeaderboard();
            saveLeaderboard();
            return;
        }
    }
    // New entry
    if (leaderboardCount < MAX_LEADERBOARD) {
        leaderboard[leaderboardCount].name = name;
        leaderboard[leaderboardCount].score = sc;
        leaderboardCount++;
    } else if (sc > leaderboard[leaderboardCount-1].score) {
        leaderboard[leaderboardCount-1].name = name;
        leaderboard[leaderboardCount-1].score = sc;
    }
    sortLeaderboard();
    saveLeaderboard();
}

// ─── Username Input ──────────────────────────────────────────────────────────
void Game::handleUsernameInput(float dt) {
    menuAnimTime += dt;
    starfield->update(dt);
    for (int i = 0; i < textInputCount; i++) {
        std::uint32_t ch = textInputBuffer[i];
        if (ch == 8) { // backspace
            if (!playerName.empty()) playerName.pop_back();
        } else if (ch == 13 || ch == 10) { // enter
            if (!playerName.empty()) {
                startGame(pendingMode);
                return;
            }
        } else if (ch >= 32 && ch < 127 && (int)playerName.length() < MAX_USERNAME) {
            char cc = (char)ch;
            if ((cc >= 'a' && cc <= 'z') || (cc >= 'A' && cc <= 'Z') || (cc >= '0' && cc <= '9')) {
                if (cc >= 'a' && cc <= 'z') cc -= 32; // uppercase
                playerName += cc;
            }
        }
    }
    static bool escHeld = false;
    bool escDn = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape);
    if (escDn && !escHeld) { escHeld = true; gsm.setState(GameState::MENU); inModeSelect = false; }
    if (!escDn) escHeld = false;
}

void Game::renderUsernameInput() {
    drawMenuBackground(window, menuAnimTime);
    drawMenuTitle(window, menuAnimTime);

    sf::RectangleShape box({440.f, 240.f});
    box.setOrigin({220.f, 120.f});
    box.setPosition({SCREEN_W / 2.f, SCREEN_H / 2.f});
    box.setFillColor(sf::Color(4, 12, 30, 230));
    box.setOutlineColor(sf::Color(0, 200, 255, 200));
    box.setOutlineThickness(2.f);
    window.draw(box);

    sf::Text hdr(font, "ENTER YOUR NAME", 26);
    hdr.setFillColor(sf::Color(0, 220, 255));
    hdr.setPosition({SCREEN_W/2.f - hdr.getGlobalBounds().size.x/2.f, SCREEN_H/2.f - 100.f});
    window.draw(hdr);

    // Input field
    sf::RectangleShape field({320.f, 44.f});
    field.setOrigin({160.f, 22.f});
    field.setPosition({SCREEN_W/2.f, SCREEN_H/2.f - 20.f});
    field.setFillColor(sf::Color(10, 20, 40));
    field.setOutlineColor(sf::Color(0, 160, 200));
    field.setOutlineThickness(1.5f);
    window.draw(field);

    // Blinking cursor
    std::string display = playerName;
    if ((int)(menuAnimTime * 2.f) % 2 == 0) display += "_";
    sf::Text nm(font, display, 28);
    nm.setFillColor(sf::Color::White);
    nm.setPosition({SCREEN_W/2.f - nm.getGlobalBounds().size.x/2.f, SCREEN_H/2.f - 36.f});
    window.draw(nm);

    sf::Text hint(font, "Max " + intToStr(MAX_USERNAME) + " characters  |  ENTER to confirm", 13);
    hint.setFillColor(sf::Color(80, 120, 160));
    hint.setPosition({SCREEN_W/2.f - hint.getGlobalBounds().size.x/2.f, SCREEN_H/2.f + 30.f});
    window.draw(hint);

    sf::Text esc(font, "ESC to go back", 13);
    esc.setFillColor(sf::Color(60, 90, 120));
    esc.setPosition({SCREEN_W/2.f - esc.getGlobalBounds().size.x/2.f, SCREEN_H/2.f + 60.f});
    window.draw(esc);

    window.display();
}

void Game::update(float dt) {
    // handled inside run() per-state
}

void Game::render() {
    GameState st = gsm.getState();
    if      (st == GameState::MENU)         renderMenu();
    else if (st == GameState::PLAYING)      renderPlaying();
    else if (st == GameState::PAUSED)       renderPaused();
    else if (st == GameState::GAME_OVER)    renderGameOver();
    else if (st == GameState::WIN)          renderWin();
    else if (st == GameState::HIGH_SCORE)   renderHighScore();
    else if (st == GameState::INSTRUCTIONS) renderInstructions();
    else if (st == GameState::CREDITS)      renderCredits();
    else if (st == GameState::USERNAME_INPUT) renderUsernameInput();
    else if (st == GameState::SETTINGS)       renderSettings();
}

std::string getKeyString(sf::Keyboard::Key key) {
    if (key >= sf::Keyboard::Key::A && key <= sf::Keyboard::Key::Z) return std::string(1, (char)('A' + ((int)key - (int)sf::Keyboard::Key::A)));
    if (key == sf::Keyboard::Key::Up) return "UP";
    if (key == sf::Keyboard::Key::Down) return "DOWN";
    if (key == sf::Keyboard::Key::Left) return "LEFT";
    if (key == sf::Keyboard::Key::Right) return "RIGHT";
    if (key == sf::Keyboard::Key::Space) return "SPACE";
    if (key == sf::Keyboard::Key::Enter) return "ENTER";
    if (key == sf::Keyboard::Key::Escape) return "ESC";
    if (key == sf::Keyboard::Key::LControl || key == sf::Keyboard::Key::RControl) return "CTRL";
    if (key == sf::Keyboard::Key::LShift || key == sf::Keyboard::Key::RShift) return "SHIFT";
    return "KEY_" + std::to_string((int)key);
}

void Game::handleSettings(float dt) {
    menuAnimTime += dt;
    starfield->update(dt);
    static bool upHeld = false, downHeld = false, enterHeld = false, escHeld = false, anyKeyHeld = false;
    
    if (waitingForKey > 0) {
        for (int i = 0; i < 105; i++) {
            if (sf::Keyboard::isKeyPressed((sf::Keyboard::Key)i)) {
                if (!anyKeyHeld) {
                    sf::Keyboard::Key k = (sf::Keyboard::Key)i;
                    if (waitingForKey == 1) Settings::keyUp = k;
                    if (waitingForKey == 2) Settings::keyDown = k;
                    if (waitingForKey == 3) Settings::keyLeft = k;
                    if (waitingForKey == 4) Settings::keyRight = k;
                    if (waitingForKey == 5) Settings::keyShoot = k;
                    if (waitingForKey == 6) Settings::keyDash = k;
                    if (waitingForKey == 7) Settings::keyEMP = k;
                    waitingForKey = 0;
                    anyKeyHeld = true;
                }
                return;
            }
        }
        anyKeyHeld = false;
        return;
    }
    
    bool upDn   = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up);
    bool downDn = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down);
    bool entDn  = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter);
    bool escDn  = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape);

    if (upDn && !upHeld) { settingsSelection = (settingsSelection - 1 + 9) % 9; upHeld = true; }
    if (!upDn) upHeld = false;
    if (downDn && !downHeld) { settingsSelection = (settingsSelection + 1) % 9; downHeld = true; }
    if (!downDn) downHeld = false;
    if (entDn && !enterHeld) {
        enterHeld = true;
        if (settingsSelection == 0) {
            Settings::fullscreen = !Settings::fullscreen;
            window.create(Settings::fullscreen ? sf::VideoMode::getDesktopMode() : sf::VideoMode({SCREEN_W, SCREEN_H}), "Space Invaders - OOP Project", 
                          Settings::fullscreen ? sf::State::Fullscreen : sf::State::Windowed);
            window.setFramerateLimit(60);
            
            sf::Vector2u winSize = window.getSize();
            float windowRatio = (float)winSize.x / (float)winSize.y;
            float viewRatio = (float)SCREEN_W / (float)SCREEN_H;
            float sizeX = 1.f;
            float sizeY = 1.f;
            float posX = 0.f;
            float posY = 0.f;

            if (windowRatio >= viewRatio) {
                sizeX = viewRatio / windowRatio;
                posX = (1.f - sizeX) / 2.f;
            } else {
                sizeY = windowRatio / viewRatio;
                posY = (1.f - sizeY) / 2.f;
            }

            sf::View view({0.f, 0.f}, {(float)SCREEN_W, (float)SCREEN_H});
            view.setCenter({SCREEN_W / 2.f, SCREEN_H / 2.f});
            view.setViewport(sf::FloatRect({posX, posY}, {sizeX, sizeY}));
            window.setView(view);
        }
        else if (settingsSelection >= 1 && settingsSelection <= 7) {
            waitingForKey = settingsSelection;
        }
        else if (settingsSelection == 8) {
            gsm.setState(GameState::MENU);
        }
    }
    if (!entDn) enterHeld = false;
    
    if (escDn && !escHeld) { escHeld = true; gsm.setState(GameState::MENU); }
    if (!escDn) escHeld = false;
}

void Game::renderSettings() {
    drawMenuBackground(window, menuAnimTime);
    drawMenuTitle(window, menuAnimTime);
    
    sf::RectangleShape panel({600.f, 480.f});
    panel.setOrigin({300.f, 0.f});
    panel.setPosition({SCREEN_W / 2.f, 160.f});
    panel.setFillColor(sf::Color(4, 12, 30, 220));
    panel.setOutlineColor(sf::Color(0, 160, 255, 180));
    panel.setOutlineThickness(2.f);
    window.draw(panel);
    
    sf::Text t(font, "SETTINGS", 32);
    t.setFillColor(sf::Color(0, 220, 255));
    t.setPosition({SCREEN_W / 2.f - t.getGlobalBounds().size.x / 2.f, 180.f});
    window.draw(t);
    
    const char* opts[] = { "FULLSCREEN", "MOVE UP", "MOVE DOWN", "MOVE LEFT", "MOVE RIGHT", "SHOOT", "DASH", "EMP", "BACK" };
    std::string vals[9];
    vals[0] = Settings::fullscreen ? "ON" : "OFF";
    vals[1] = getKeyString(Settings::keyUp);
    vals[2] = getKeyString(Settings::keyDown);
    vals[3] = getKeyString(Settings::keyLeft);
    vals[4] = getKeyString(Settings::keyRight);
    vals[5] = getKeyString(Settings::keyShoot);
    vals[6] = getKeyString(Settings::keyDash);
    vals[7] = getKeyString(Settings::keyEMP);
    vals[8] = "";
    
    for (int i = 0; i < 9; i++) {
        bool sel = (settingsSelection == i);
        float y = 240.f + i * 40.f;
        
        if (sel) {
            sf::RectangleShape bar({560.f, 36.f});
            bar.setOrigin({280.f, 18.f});
            bar.setPosition({SCREEN_W / 2.f, y + 14.f});
            bar.setFillColor(sf::Color(0, 80, 160, 80));
            bar.setOutlineColor(sf::Color(0, 220, 255, 200));
            bar.setOutlineThickness(1.5f);
            window.draw(bar);
            drawArrow(window, SCREEN_W / 2.f - 270.f, y + 5.f, sf::Color(0, 255, 200));
        }
        
        sf::Text label(font, opts[i], 20);
        label.setFillColor(sel ? sf::Color(0, 255, 200) : sf::Color(160, 190, 210));
        label.setPosition({SCREEN_W / 2.f - 240.f, y});
        window.draw(label);
        
        if (i < 8) {
            std::string displayVal = vals[i];
            if (waitingForKey == i && i >= 1 && i <= 7) displayVal = "PRESS ANY KEY...";
            
            sf::Text val(font, displayVal, 20);
            val.setFillColor(waitingForKey == i ? sf::Color(255, 100, 100) : sf::Color::White);
            val.setPosition({SCREEN_W / 2.f + 100.f, y});
            window.draw(val);
        }
    }
    
    window.display();
}
