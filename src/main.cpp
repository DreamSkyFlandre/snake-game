#include <SFML/Graphics.hpp>
#include <stdlib.h>
#include <time.h>
#include <vector>

// 定义格子的尺寸（贪吃蛇通常是网格运动）
const int BLOCK_SIZE = 20; 
// 窗口大小
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const double DELAY = 0.1f; // 蛇移动的速度：0.2秒移动一次

int main()
{
    srand(static_cast<unsigned int>(time(NULL))); // 初始化随机数种子

    // 创建窗口
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "My Snake Game");
    
    // --- 1. 初始化蛇与食物 ---
    std::vector<sf::RectangleShape> snake_body; // 蛇的全体身体节点

    // 初始化蛇头
    sf::RectangleShape head(sf::Vector2f(BLOCK_SIZE, BLOCK_SIZE));
    head.setFillColor(sf::Color::Cyan);
    head.setPosition(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2); // 初始位置在窗口中心
    snake_body.push_back(head);
    sf::Keyboard::Key direction = sf::Keyboard::Right; // 默认移动方向向右
    sf::Keyboard::Key last_direction = sf::Keyboard::Right; // 记录上一帧蛇实际移动的方向

    // 初始化食物
    sf::Vector2f food_pos;
    food_pos.x = WINDOW_WIDTH * 3 / 4;
    food_pos.y = WINDOW_HEIGHT / 2;
    sf::CircleShape food(BLOCK_SIZE / 2);
    food.setFillColor(sf::Color::Red);
    food.setPosition(food_pos); // 初始食物位置固定在右侧3/4处
    
    // --- 时间控制变量 ---
    sf::Clock clock;
    double timer = 0.0f;

    while (window.isOpen())
    {
        double time = clock.getElapsedTime().asSeconds();
        clock.restart();
        timer += time;

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // --- 2. 处理键盘输入 ---
        // 注意：这里我们判断的是 last_direction，而不是 direction
        // 这样即使你在 0.2秒内狂按键盘，也只能基于“上一次移动的方向”来变向
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            if (last_direction != sf::Keyboard::Down) {
                direction = sf::Keyboard::Up;
            }
        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            if (last_direction != sf::Keyboard::Up) {
                direction = sf::Keyboard::Down;
            }
        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            if (last_direction != sf::Keyboard::Right) {
                direction = sf::Keyboard::Left;
            }
        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            if (last_direction != sf::Keyboard::Left) {
                direction = sf::Keyboard::Right;
            };
        } // 使用方向键或 WASD 控制方向，注意不能掉头

        // --- 3. 处理游戏逻辑 (核心) ---
        // 当计时器超过 0.2 秒时，蛇才动一次
        if (timer > DELAY)
        {
            timer -= DELAY; // 重置计时器

            // 记录这一帧真正要走的方向
            last_direction = direction;
            
            // 让蛇根据当前的“方向”移动一个 BLOCK_SIZE 的距离
            sf::Vector2f tail_pos = snake_body.back().getPosition(); // 记录蛇尾位置，可能用来增长蛇身

            for (size_t i = snake_body.size() - 1; i > 0; --i) {
                snake_body[i].setPosition(snake_body[i - 1].getPosition());
            }

            sf::Vector2f head_pos = snake_body[0].getPosition();
            
            if (direction == sf::Keyboard::Up) {
                if (head_pos.y >= BLOCK_SIZE) {
                    snake_body[0].move(0, -BLOCK_SIZE);
                } else {
                    snake_body[0].move(0, WINDOW_HEIGHT - BLOCK_SIZE);
                }
            } else if (direction == sf::Keyboard::Down) {
                if (head_pos.y <= WINDOW_HEIGHT - BLOCK_SIZE * 2) {
                    snake_body[0].move(0, BLOCK_SIZE);
                } else {
                    snake_body[0].move(0, BLOCK_SIZE - WINDOW_HEIGHT);
                }
            } else if (direction == sf::Keyboard::Left) {
                if (head_pos.x >= BLOCK_SIZE) {
                    snake_body[0].move(-BLOCK_SIZE, 0);
                } else {
                    snake_body[0].move(WINDOW_WIDTH - BLOCK_SIZE, 0);
                }
            } else {
                if (head_pos.x <= WINDOW_WIDTH - BLOCK_SIZE * 2) {
                    snake_body[0].move(BLOCK_SIZE, 0);
                } else {
                    snake_body[0].move(BLOCK_SIZE - WINDOW_WIDTH, 0);
                }
            } // 实现上下左右移动，以及穿屏

            if(snake_body[0].getGlobalBounds().intersects(food.getGlobalBounds())) {
                // 蛇吃到食物，重新生成食物位置
                food_pos.x = (rand() % (WINDOW_WIDTH / BLOCK_SIZE)) * BLOCK_SIZE;
                food_pos.y = (rand() % (WINDOW_HEIGHT / BLOCK_SIZE)) * BLOCK_SIZE;
                while (true) {
                    bool on_snake = false;
                    for (auto& segment : snake_body) {
                        if (segment.getPosition() == food_pos) {
                            on_snake = true;
                            break;
                        }
                    }
                    if (!on_snake) break; // 确保食物不生成在蛇身上
                    food_pos.x = (rand() % (WINDOW_WIDTH / BLOCK_SIZE)) * BLOCK_SIZE;
                    food_pos.y = (rand() % (WINDOW_HEIGHT / BLOCK_SIZE)) * BLOCK_SIZE;
                }
                food.setPosition(food_pos);

                sf::RectangleShape tail(sf::Vector2f(BLOCK_SIZE, BLOCK_SIZE));
                tail.setFillColor(sf::Color::Cyan);
                tail.setPosition(tail_pos);
                snake_body.push_back(tail); // 在蛇尾添加新节点，实现增长
            }

            // 碰撞检测：检查蛇头是否碰到蛇身
            for (size_t i = 1; i < snake_body.size(); ++i) {
                if (snake_body[0].getGlobalBounds().intersects(snake_body[i].getGlobalBounds())) {
                    // 游戏重置
                    // 重置蛇的位置和长度
                    snake_body.clear();
                    snake_body.push_back(head);
                    direction = sf::Keyboard::Right;
                    last_direction = sf::Keyboard::Right;
                    // 重置食物位置
                    food_pos.x = WINDOW_WIDTH * 3 / 4;
                    food_pos.y = WINDOW_HEIGHT / 2;
                    food.setPosition(food_pos);
                    break;
                }
            }
        }

        // --- 4. 渲染 (画画) ---
        window.clear(); // 清除上一帧
        for (auto& segment : snake_body) {
            window.draw(segment);
        } // 画蛇
        window.draw(food); // 画食物
        window.display(); // 显示
    }

    return 0;
}