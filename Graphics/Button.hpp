#pragma once 
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Font.hpp>
#include <string>
#include <functional>
#include <optional>

extern sf::Font font;

class Button: public sf::Drawable{
    int _lengthChars = 0;
    mutable sf::Text _text;
    std::function<void()> _cb;

    sf::Vector2f _pos;

    bool isSelected = false;

    inline static int offset_x = 16;
    inline static int offset_y = 12;
public:

    Button(std::wstring title, std::function<void()> cb, sf::Vector2f pos): _cb(cb), _text(font, title, 24), _pos(pos)
    {
        _lengthChars = static_cast<int>(title.size()); 
    }

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {

        sf::RectangleShape rect({(float)(offset_x * _lengthChars), (float)_text.getCharacterSize() + offset_y});
        rect.setPosition(_pos);
        rect.setFillColor(sf::Color(20, 20, 20, 255));
        rect.setOutlineColor(isSelected ? sf::Color::Green : sf::Color(127, 127, 127));
        rect.setOutlineThickness(2);

        target.draw(rect);

        _text.setPosition(_pos + sf::Vector2f{(float)offset_x, .0f});
        _text.setFillColor(sf::Color::White);

        target.draw(_text);
    }

    void setPosition(sf::Vector2f pos_)
    {
        this->_pos = pos_;
    }

    void handleEvent(const std::optional<sf::Event>& event, sf::RenderWindow& window)
    {
        if (event->is<sf::Event::MouseMoved>())
        {
            auto pos = sf::Mouse::getPosition(window);
            isSelected = sf::FloatRect(_pos, {(float)(offset_x * _lengthChars), (float)_text.getCharacterSize() + offset_y}).contains(sf::Vector2f{ (float)pos.x, (float)pos.y });
        }
        
        if(auto* d = event->getIf<sf::Event::MouseButtonReleased>())
        {
            if(d->button == sf::Mouse::Button::Left && isSelected) _cb();
        }
    }
};