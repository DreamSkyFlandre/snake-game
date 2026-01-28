#include <SFML/Graphics.hpp>
#include <SFML/Audio/Music.hpp>
#include <vector>
#include <string>
#include <ctime>
#include <cstdlib>
#include <iostream>

// --- 配置参数 ---
const int BLOCK_SIZE = 30;
const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 900;
const float MOVE_DELAY = 0.1f; // 蛇的速度

// --- 游戏状态枚举 ---
enum class GameState {
    MENU,
    PLAYING,
    GAME_OVER
};

// --- 蛇 类 ---
class Snake {
public:
    std::vector<sf::RectangleShape> body;
    sf::Vector2f direction;
    sf::Vector2f last_direction; // 防止急速掉头

    Snake() {
        reset();
    }

    void reset() {
        body.clear();
        // 初始化 3 节身体
        for (int i = 0; i < 3; ++i) {
            sf::RectangleShape part(sf::Vector2f(BLOCK_SIZE, BLOCK_SIZE));
            part.setFillColor(i == 0 ? sf::Color::Cyan : sf::Color::Green); // 头是青色，身子是绿色
            part.setPosition(WINDOW_WIDTH / 2 - i * BLOCK_SIZE, WINDOW_HEIGHT / 2);
            body.push_back(part);
        }
        direction = {BLOCK_SIZE, 0}; // 初始向右
        last_direction = direction;
    }

    void handleInput() {
        // 键盘输入处理
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            if (last_direction.y == 0) direction = {0, -BLOCK_SIZE};
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            if (last_direction.y == 0) direction = {0, BLOCK_SIZE};
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            if (last_direction.x == 0) direction = {-BLOCK_SIZE, 0};
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            if (last_direction.x == 0) direction = {BLOCK_SIZE, 0};
        }
    }

    void move() {
        last_direction = direction; // 更新这一帧的实际方向

        // 1. 移动身体（倒序）
        for (size_t i = body.size() - 1; i > 0; --i) {
            body[i].setPosition(body[i - 1].getPosition());
        }

        // 2. 移动头
        body[0].move(direction);

        // 3. 穿墙处理
        sf::Vector2f pos = body[0].getPosition();
        if (pos.x < 0) body[0].setPosition(WINDOW_WIDTH - BLOCK_SIZE, pos.y);
        if (pos.x >= WINDOW_WIDTH) body[0].setPosition(0, pos.y);
        if (pos.y < 0) body[0].setPosition(pos.x, WINDOW_HEIGHT - BLOCK_SIZE);
        if (pos.y >= WINDOW_HEIGHT) body[0].setPosition(pos.x, 0);
    }

    void grow() {
        sf::RectangleShape tail = body.back(); // 复制当前尾巴
        // 暂时不改位置，下一帧它会自动归位，或者你可以设在屏幕外
        body.push_back(tail);
    }

    bool checkSelfCollision() {
        for (size_t i = 1; i < body.size(); ++i) {
            if (body[0].getGlobalBounds().intersects(body[i].getGlobalBounds())) {
                return true;
            }
        }
        return false;
    }

    void draw(sf::RenderWindow& window) {
        for (const auto& part : body) {
            window.draw(part);
        }
    }
};

// --- 游戏主控 类 ---
class Game {
private:
    sf::RenderWindow window;
    Snake snake;
    sf::CircleShape food;
    sf::Clock clock;
    float timer;
    int score;
    GameState state;

    sf::Font font;
    sf::Text scoreText;
    sf::Text titleText;
    sf::Text subText;
    sf::Music bgm;

    float currentVolume;  // 当前音量
    const float MAX_VOLUME = 50.f;  // 最大音量
    const float FADE_SPEED = 30.f;  // 渐变速度 (每秒改变多少音量)

public:
    Game() : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Snake Game"), timer(0), score(0), state(GameState::MENU) {
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

    void run() {
        while (window.isOpen()) {
            processEvents();
            update();
            render();
        }
    }

private:
    void processEvents() {
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

    void update() {
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
            if (snake.body[0].getGlobalBounds().intersects(food.getGlobalBounds())) {
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

    void render() {
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

    void resetGame() {
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

    void respawnFood() {
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
            for (const auto& segment : snake.body) {
                if (food.getGlobalBounds().intersects(segment.getGlobalBounds())) {
                    validPosition = false;
                    break;
                }
            }
        }
    }

    // 辅助函数：居中显示两行文字
    void drawCenterText(std::string title, std::string sub, sf::Color color) {
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
};

int main() {
    Game game;
    game.run();
    return 0;
}