#ifndef SNAKE_H
#define SNAKE_H

#include <SFML/Graphics.hpp>
#include <vector>
#include "constants.h"

// --- 蛇 类 ---
class Snake {
private:
    std::vector<sf::RectangleShape> body;
    sf::Vector2f direction; // 当前移动方向
    sf::Vector2f last_direction; // 上一次的移动方向，防止急速掉头

public:
    Snake() {
        reset();
    }

    std::vector<sf::RectangleShape> getBody() const {
        return body;
    }

    void reset();

    void handleInput();

    void move();

    void grow();

    bool checkSelfCollision();

    void draw(sf::RenderWindow& window);
};

#endif // SNAKE_H