#pragma once
#include <SFML/Graphics.hpp>
#include "../Graphics/Button.hpp"
#include "../Graphics/WarField.hpp"
#include "./Network/Connection.hpp"
#include "Types.hpp"

extern std::vector<Button> buttons;
extern WarField _mainField;
extern WarField _enemyField;
extern Button readyButton;
extern States_t currentState;
extern WHO_MOVE currentMove;
extern Connection connection;
extern Button finishRoundButton;

void MENU_event(sf::RenderWindow& window, const std::optional<sf::Event>& event)
{
    for(auto& button: buttons) button.handleEvent(event, window);
}

void BUILDING_event(sf::RenderWindow& window, const std::optional<sf::Event>& event)
{
    Field* buildingField = &_mainField;

    buildingField->handleEvent(event, window); 
    readyButton.handleEvent(event, window);
}

void WAR_event(sf::RenderWindow& window, const std::optional<sf::Event>& event)
{
    if(currentMove == WHO_MOVE::ME) {
        const auto step = _enemyField.handleEvent(event, window);
        if(step) {
            connection.write(
                Message {
                    .type = Message::type_message::MAKE_STEP,
                    .payload = {
                        .pos = *step
                    }
                }
            );
            currentMove = WHO_MOVE::ENEMY;
        }
    }
}

void END_ROUND_event(sf::RenderWindow& window, const std::optional<sf::Event>& event)
{
    finishRoundButton.handleEvent(event, window);

    _enemyField._shots.clear();
    _mainField._shots.clear();

    _enemyField.clearShips();
    _mainField.clearShips();
}