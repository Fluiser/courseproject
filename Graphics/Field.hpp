#pragma once
#include <vector>
#include <array>
#include <cstdint>
#include <SFML/Graphics.hpp>
#include <iostream>

using v2u = sf::Vector2u;
using v2f = sf::Vector2f;

extern sf::Texture field_background_texture;

struct Position {
    uint8_t x;
    uint8_t y;
};

class Field;

constexpr int width = 500-5;
constexpr int height = 500-5;
constexpr int step_x = width / 10;
constexpr int step_y = height / 10;


class Ship : public sf::Drawable {
    bool isVertical = false;
    
    v2f pixelPos; 
    bool isDragging = false;

    friend Field;
public:
    std::array<Position, 4> _arr;
    uint8_t rank; 
    
    Ship(uint8_t r, v2f startPixelPos) : rank(r), pixelPos(startPixelPos) 
    {
        for(int i = 0; i < 4; ++i) _arr[i] = {255, 255}; 
    }

    bool contains(v2f mousePos, int step_x, int step_y) const 
    {
        float w = static_cast<float>(isVertical ? step_x : step_x * rank);
        float h = static_cast<float>(isVertical ? step_y * rank : step_y);
        return sf::FloatRect({pixelPos.x, pixelPos.y}, {w, h}).contains(mousePos);
    }

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override 
    {

        constexpr float s_x = step_x * 0.65f; 
        constexpr float s_y = step_y * 0.65f;
        constexpr float offset_x = (step_x - s_x) / 2;
        constexpr float offset_y = (step_y - s_y) / 2;

        sf::RectangleShape shape;
        shape.setFillColor(isDragging ? sf::Color(100, 255, 100, 150) : sf::Color(70,172,173, 170));
        shape.setOutlineThickness(1);
        shape.setOutlineColor(sf::Color::Black);
        

        if (isVertical) {
            shape.setPosition({pixelPos.x + offset_x, pixelPos.y + offset_y});
            shape.setSize({s_x, step_y * (float)rank - offset_y*2});
        } else {
            shape.setPosition({pixelPos.x + offset_x, pixelPos.y + offset_y});
            shape.setSize({step_x * (float)rank - offset_x*2, s_y});
        }

        target.draw(shape);
    }
};


class Field : public sf::Drawable {
    Ship* selectedShip = nullptr;
    v2f dragOffset;

protected:
    friend void render(sf::RenderWindow& window);
    int _x = 0, _y = 0;
    std::vector<Ship> _ships;

    void drawBackground(sf::RenderTarget& target) const
    {
        sf::Sprite sprite(field_background_texture);
        sprite.setPosition({(float)_x - 87, (float)_y - 92});
        target.draw(sprite);
    }

    void drawGrid(sf::RenderTarget& target) const
    {
        sf::VertexArray lines(sf::PrimitiveType::Lines, 44);
        int i = 0;
        for(int x = 0; x <= width; x += step_x, i += 2) 
        {
            lines[i].position = v2f{(float)x + _x, (float)_y};
            lines[i+1].position = v2f{(float)x + _x, (float)height + _y};
        }
        for(int y = 0; y <= height; y += step_y, i += 2) 
        {
            lines[i].position = v2f{(float)_x, (float)y + _y};
            lines[i+1].position = v2f{(float)width + _x, (float)y + _y};
        }
        for(int j = 0; j < 44; ++j) 
        {
            lines[j].color = sf::Color(127,127,127);
        }

        target.draw(lines);
    }

    bool thisEnemy = false;
public:
    Field(int x, int y, bool enemy) : _x(x), _y(y), thisEnemy(enemy) 
    {
    }

    const std::vector<Ship>& getShips() const
    {
        return _ships;
    }

    void spawnShips() 
    {
        struct Config { uint8_t rank; int count; };
        std::vector<Config> configs = {{4, 1}, {3, 2}, {2, 3}, {1, 4}};
        
        float start_y = static_cast<float>(_y + height + 20);
        float curr_x = static_cast<float>(_x);

        for (auto& conf : configs) 
        {
            for (int i = 0; i < conf.count; ++i)
            {
                _ships.emplace_back(conf.rank, v2f(curr_x, start_y));
                curr_x += (conf.rank * step_x) + 10;
                if (curr_x > _x + width) 
                {
                    curr_x = (float)_x;
                    start_y += step_y + 10;
                }
            }
        }
    }


    void handleEvent(const std::optional<sf::Event>& event, sf::RenderWindow& window) 
    {
        v2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        if(const auto* key = event->getIf<sf::Event::KeyPressed>()) 
        {
            if(key->scancode == sf::Keyboard::Scancode::R) 
            {
                if(selectedShip) 
                {

                    for(int i = 1; i < selectedShip->rank; ++i)
                    {
                        if(selectedShip->isVertical)
                        {
                            selectedShip->_arr[i].x = selectedShip->_arr[0].x + i;
                            selectedShip->_arr[i].y = selectedShip->_arr[0].y;
                        } else {
                            selectedShip->_arr[i].x = selectedShip->_arr[0].x;
                            selectedShip->_arr[i].y = selectedShip->_arr[0].y + i;
                        }
                    }
                    selectedShip->isVertical = !selectedShip->isVertical;
                }
            }
        }

        if (event->is<sf::Event::MouseButtonPressed>()) 
        {
            for (auto& ship : _ships) 
            {
                if (ship.contains(mousePos, step_x, step_y)) 
                {
                    selectedShip = &ship;
                    selectedShip->isDragging = true;
                    dragOffset = selectedShip->pixelPos - mousePos;
                    break;
                }
            }
        }

        if (event->is<sf::Event::MouseButtonReleased>() && selectedShip) 
        {
            snapToGrid(selectedShip);
            selectedShip->isDragging = false;
            selectedShip = nullptr;
        }

        if (event->is<sf::Event::MouseMoved>() && selectedShip) 
        {
            selectedShip->pixelPos = mousePos + dragOffset;
        }
    }

    void snapToGrid(Ship* ship) {
        constexpr float s_x = step_x * 0.65f; 
        constexpr float s_y = step_y * 0.65f;
        constexpr float offset_x = (step_x - s_x) / 2;
        constexpr float offset_y = (step_y - s_y) / 2;

        float relX = ship->pixelPos.x - _x + offset_x;
        float relY = ship->pixelPos.y - _y + offset_y;

        int gridX = static_cast<int>(relX / step_x);
        int gridY = static_cast<int>(relY / step_y);

        if (gridX >= 0 && gridX < 10 && gridY >= 0 && gridY < 10) 
        {
            ship->pixelPos.x = static_cast<float>(_x + gridX * step_x);
            ship->pixelPos.y = static_cast<float>(_y + gridY * step_y);
            
            for(int i = 0; i < ship->rank; ++i) 
            {
                ship->_arr[i].x = (uint8_t)(gridX + (ship->isVertical ? 0 : i));
                ship->_arr[i].y = (uint8_t)(gridY + (ship->isVertical ? i : 0));
            }
        }
    }

    
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const 
    {
        this->drawBackground(target);
        this->drawGrid(target);
        for (const auto& ship : _ships) 
        {
            target.draw(ship);
        }
    }
};