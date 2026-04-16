#pragma once
#include "Field.hpp"
#include <list>
#include <cmath>
#include <set>

class WarField: public Field {
public:
    int gridXMouse = 255;
    int gridYMouse = 255;

    std::list<Position> _shots;
    WarField(int x, int y, bool e = false) : Field(x,y, e)
    {
        
    }

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override 
    {
        Field::draw(target, states);

        for(const auto& shot: _shots)
        {
            float x = static_cast<float>(shot.x) * step_x + _x;
            float y = static_cast<float>(shot.y) * step_y + _y;

            if(haveShip(shot.x, shot.y)) {
                static float lengthDiagonal = std::sqrtf( step_x*step_x + step_y*step_y );

                sf::RectangleShape line({lengthDiagonal, 4});
                line.rotate(sf::degrees(45));
                line.setPosition({x,y});

                sf::RectangleShape line_s({lengthDiagonal, 4});
                line_s.rotate(sf::degrees(-45));
                line_s.setPosition({x,y + step_y});

                line.setFillColor(sf::Color::Blue);
                line_s.setFillColor(sf::Color::Blue);

                target.draw(line);
                target.draw(line_s);
            } else {
                sf::RectangleShape rect({(float)step_x, (float)step_y});
                rect.setPosition({x, y});
                rect.setFillColor(sf::Color(127, 127, 127, 100));
                target.draw(rect);

                sf::CircleShape circle(step_x/4);
                circle.setFillColor(sf::Color::Blue);
                circle.setPosition({x + (step_x / 2.0f) - step_x/4, y + (step_y / 2.0f) - step_y/4});
                target.draw(circle);
            }
        }

        if(gridXMouse >= 0 && gridXMouse <= 9 && gridYMouse >= 0 && gridYMouse <= 9) {
            float x = static_cast<float>(gridXMouse) * step_x + _x;
            float y = static_cast<float>(gridYMouse) * step_y + _y;
            sf::RectangleShape rect({(float)step_x, (float)step_y});
            rect.setSize({(float)step_x, (float)step_y});
            rect.setPosition({(float)x, (float)y});
            rect.setFillColor(sf::Color(255, 20, 20, 50));

            target.draw(rect);
        }
    }

    bool isEnd() const
    {
        std::set<uint16_t> positions;
        for(const auto& ship: _ships)
        {
            for(int i = 0; i < ship.rank; ++i)
            {
                uint16_t pos = ship._arr[i].x;
                pos <<= 8;
                pos |= ship._arr[i].y;
                positions.insert(pos);
            }
        }
        for(const auto& shot: _shots)
        {
            uint16_t pos = shot.x;
            pos <<= 8;
            pos |= shot.y;
            positions.erase(pos);
        }
        return (positions.size() < 1);
    }

    bool haveShip(int gridX, int gridY) const
    {
        auto it = std::find_if(_ships.begin(), _ships.end(), [gridX, gridY](const Ship& s) {
            for(int i = 0; i < s.rank; ++i){
                if(s._arr[i].x == gridX && s._arr[i].y == gridY) return true;

            }
            return false;
        });
        return (it != _ships.end());
    }

    void addShip(int gridX, int gridY)
    {
        Ship s(1, v2f{
            (float)(gridX * step_x + _x),
            (float)(gridY * step_y + _y)
        });
        s._arr[0] = Position(gridX, gridY);
        _ships.emplace_back(s);
    }

    void clearShips()
    {
        this->_ships.clear();
        if(!this->thisEnemy) {
            this->spawnShips();
        }
    }

    std::optional<Position> handleEvent(const std::optional<sf::Event>& event, sf::RenderWindow& window) 
    {
        if(auto* b = event->getIf<sf::Event::MouseButtonReleased>()) {
            if(b->button == sf::Mouse::Button::Left) {
                auto mousePos = sf::Mouse::getPosition(window);
                int gridX = (mousePos.x - _x) / step_x;
                int gridY = (mousePos.y - _y) / step_y;

                if(gridX >= 0 && gridX <= 9 && gridY >= 0 && gridY <= 9) {
                    auto pos = Position{
                        .x = static_cast<uint8_t>(gridX),
                        .y = static_cast<uint8_t>(gridY)
                    };

                    auto it = std::find_if(_shots.begin(), _shots.end(), [&pos](const Position& shot){
                        return shot.x == pos.x && shot.y == pos.y;
                    });

                    if(it == _shots.end()) {
                        _shots.push_back(pos);
                        return pos;
                    }
                }
            }
        }
        if(event->is<sf::Event::MouseMoved>())
        {
            auto mousePos = sf::Mouse::getPosition(window);
            gridXMouse = (mousePos.x - _x) / step_x;
            gridYMouse = (mousePos.y - _y) / step_y;
        }

        return {};
    }
};