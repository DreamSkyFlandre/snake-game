#include "game.h"
#include <ctime>
#include <cstdlib>
#include <vector>

// 游戏初始化
Game::Game() : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Snake Game"), timer(0), score(0), state(GameState::MENU) {
    window.setFramerateLimit(60);
    srand(static_cast<unsigned int>(time(nullptr)));

    // 加载字体
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Failed to load font!" << std::endl;
        exit(EXIT_FAILURE);
    }

    // 加载背景音乐
    if (!bgm.openFromFile("man-is-he-mega-glbml-22045.mp3")) {
        std::cerr << "Failed to load background music!" << std::endl;
        exit(EXIT_FAILURE);
        // 音乐循环播放
    } else {
        bgm.setLoop(true);
        bgm.play();
        bgm.setVolume(MAX_VOLUME);
        currentVolume = MAX_VOLUME;
    }

    // 初始化文字
    scoreText.setFont(font);
    scoreText.setCharacterSize(36);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(10, 10);

    titleText.setFont(font);
    titleText.setCharacterSize(75);
    titleText.setFillColor(sf::Color::Yellow);
    
    subText.setFont(font);
    subText.setCharacterSize(30);
    subText.setFillColor(sf::Color::White);

    respawnFood();
}

// 游戏主循环
void Game::run() {
    while (window.isOpen()) {
        processEvents();
        update();
        render();
    }
}

// 处理输入事件
void Game::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();

        // 菜单/结束状态下的按键处理
        if (state == GameState::MENU || state == GameState::GAME_OVER) {
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
                resetGame(); // 开始新游戏
            }
        }

        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
            if (state != GameState::MENU) {
                state = GameState::MENU; // 暂停回菜单
            } else {
                window.close(); // 退出游戏
            }
        }
    }

    // 游戏中的实时按键
    if (state == GameState::PLAYING) {
        snake.handleInput();
    }
}

// 游戏更新逻辑
void Game::update() {
    // 获取两帧之间的时间差
    float dt = clock.getElapsedTime().asSeconds();
    clock.restart();

    // --- 新增：音乐渐变逻辑 (每一帧都运行) ---
    if (state == GameState::PLAYING || state == GameState::MENU) {
        // 如果没死，且音量还没满，就增加
        if (currentVolume < MAX_VOLUME) {
            currentVolume += FADE_SPEED * dt;
            if (currentVolume > MAX_VOLUME) currentVolume = MAX_VOLUME; // 封顶
            bgm.setVolume(currentVolume);
        }
    }
    else if (state == GameState::GAME_OVER) {
        // 如果死了，且还有声音，就减小
        if (currentVolume > 0.f) {
            currentVolume -= FADE_SPEED * dt;
            if (currentVolume < 0.f) currentVolume = 0.f; // 封底
            bgm.setVolume(currentVolume);
        }
    }

    // 如果不是在玩游戏，就不处理蛇的移动
    if (state != GameState::PLAYING) return;

    timer += dt;

    if (timer > MOVE_DELAY) {
        timer -= MOVE_DELAY;
        snake.move();

        // 吃食物检测
        if (snake.getBody()[0].getGlobalBounds().intersects(food.getGlobalBounds())) {
            snake.grow();
            score += 10;
            respawnFood();
        }

        // 碰撞检测
        if (snake.checkSelfCollision()) {
            state = GameState::GAME_OVER;
        }
    }
}

// 游戏渲染
void Game::render() {
    window.clear(sf::Color::Black);

    if (state == GameState::MENU) {
        drawCenterText("SNAKE GAME", "Press ENTER to Start\nPress ESC to Exit", sf::Color::Cyan);
    } 
    else if (state == GameState::GAME_OVER) {
        drawCenterText("GAME OVER", "Score: " + std::to_string(score) + "\nPress ENTER to Restart", sf::Color::Red);
    } 
    else {
        // 绘制游戏画面
        snake.draw(window);
        window.draw(food);
        
        // 绘制分数
        scoreText.setString("Score: " + std::to_string(score));
        window.draw(scoreText);
    }

    window.display();
}

// 重置游戏状态
void Game::resetGame() {
    state = GameState::PLAYING;
    score = 0;
    snake.reset();
    respawnFood();
    timer = 0;
    clock.restart();
    // 如果之前暂停了或者没播放，确保它在播放
    if (bgm.getStatus() != sf::Music::Playing)
        bgm.play();
}

// 重新生成食物位置
void Game::respawnFood() {
    food.setRadius(BLOCK_SIZE / 2.f);
    food.setFillColor(sf::Color::Red);
    
    // 生成并检查是否生成在蛇身上
    bool validPosition = false;
    while (!validPosition) {
        int cols = WINDOW_WIDTH / BLOCK_SIZE;
        int rows = WINDOW_HEIGHT / BLOCK_SIZE;
        food.setPosition(
            (rand() % cols) * BLOCK_SIZE,
            (rand() % rows) * BLOCK_SIZE
        );

        // 检查食物是否与蛇身重叠
        validPosition = true;
        for (const auto& segment : snake.getBody()) {
            if (food.getGlobalBounds().intersects(segment.getGlobalBounds())) {
                validPosition = false;
                break;
            }
        }
    }
}

// 辅助函数：居中显示两行文字
void Game::drawCenterText(std::string title, std::string sub, sf::Color color) {
    titleText.setString(title);
    titleText.setFillColor(color);
    // 简单的居中算法
    sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setOrigin(titleBounds.width / 2, titleBounds.height / 2);
    titleText.setPosition(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 - 75);

    subText.setString(sub);
    sf::FloatRect subBounds = subText.getLocalBounds();
    subText.setOrigin(subBounds.width / 2, subBounds.height / 2);
    subText.setPosition(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 30);

    window.draw(titleText);
    window.draw(subText);
}