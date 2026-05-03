#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio/Music.hpp>
#include <iostream>
#include <string>
#include "snake.h"
#include "constants.h"

// --- 游戏状态枚举 ---
enum class GameState {
    MENU,
    PLAYING,
    GAME_OVER
};

// --- 游戏主控 类 ---
class Game {
private:
    sf::RenderWindow window;
    Snake snake;
    sf::CircleShape food;
    sf::Clock clock;
    float timer; // 移动计时器
    int score;
    GameState state;

    sf::Font font;
    sf::Text scoreText; // 显示分数
    sf::Text titleText; // 显示标题和结束信息
    sf::Text subText; // 显示副标题（如操作提示）

    sf::Music bgm;
    float currentVolume;  // 当前音量

public:
    Game();

    void run();

private:
    void processEvents();

    void update();

    void render();

    void resetGame();

    void respawnFood();

    void drawCenterText(std::string title, std::string sub, sf::Color color);
};

#endif // GAME_H