# GitHub Upload Steps

Repository: https://github.com/ranaharis26/space-invaders-oop-sfml.git

## Method 1: GitHub Website

1. Open the repository on GitHub.
2. Click **Add file** → **Upload files**.
3. Drag and drop all files from this folder.
4. Write commit message: `Add Space Invaders SFML project`
5. Click **Commit changes**.

## Method 2: Command Line

Open terminal inside this folder and run:

```bash
git init
git add .
git commit -m "Add Space Invaders SFML project"
git branch -M main
git remote add origin https://github.com/ranaharis26/space-invaders-oop-sfml.git
git push -u origin main
```

If Git says the remote already has files, use:

```bash
git pull origin main --allow-unrelated-histories
git push -u origin main
```

## Repo Description

Use this GitHub description:

```text
C++17/SFML object-oriented arcade shooter with enemy waves, boss fights, power-ups, audio, HUD, settings, and leaderboard support.
```

## Suggested Topics

Add these GitHub topics:

```text
cpp, cpp17, sfml, game-development, oop, object-oriented-programming, space-invaders, arcade-game, cmake, data-structures
```

## Screenshot Names

Add your screenshots inside the `screenshots/` folder using these names:

```text
screenshots/menu.png
screenshots/gameplay.png
screenshots/boss.png
screenshots/leaderboard.png
```

After adding screenshots, update the README table if needed.
