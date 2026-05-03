#include "snake.h"

// 重置蛇到初始状态
void Snake::reset() {
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

// 键盘输入处理
void Snake::handleInput() {
    // WASD 或方向键控制方向，不能直接掉头
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

// 移动蛇
void Snake::move() {
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

// 蛇的增长
void Snake::grow() {
    sf::RectangleShape tail = body.back(); // 复制当前尾巴
    // 暂时不改位置，下一帧它会自动归位
    body.push_back(tail);
}

// 检查蛇头是否与身体其他部分碰撞
bool Snake::checkSelfCollision() {
    for (size_t i = 1; i < body.size(); ++i) {
        if (body[0].getGlobalBounds().intersects(body[i].getGlobalBounds())) {
            return true;
        }
    }
    return false;
}

// 绘制蛇
void Snake::draw(sf::RenderWindow& window) {
    for (const auto& part : body) {
        window.draw(part);
    }
}