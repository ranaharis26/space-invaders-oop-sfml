cmake_minimum_required(VERSION 3.10)
project(SpaceInvaders LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Requires SFML 3.x.
# If SFML is not found automatically on Windows, configure with:
# cmake -S . -B build -DSFML_DIR="C:/SFML/lib/cmake/SFML"
find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System Audio)

set(SOURCES
    main.cpp
    Entity.cpp
    Bullet.cpp
    Player.cpp
    Enemy.cpp
    Drone.cpp
    Viper.cpp
    Seeker.cpp
    Boss.cpp
    Cruiser.cpp
    TwinCannons.cpp
    Mothership.cpp
    PowerUp.cpp
    Asteroid.cpp
    Explosion.cpp
    Starfield.cpp
    CollisionManager.cpp
    AudioManager.cpp
    GameStateManager.cpp
    HUD.cpp
    Settings.cpp
    Game.cpp
)

add_executable(SpaceInvaders ${SOURCES})

target_link_libraries(SpaceInvaders
    sfml-graphics
    sfml-window
    sfml-audio
    sfml-system
)
