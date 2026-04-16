#include "Button.hpp"
#include "../System/Network/Connection.hpp"
#include "../System/Types.hpp"
#include "WarField.hpp"
#include <set>
#include <cstdint>
#include <vector>

extern States_t currentState;
extern Connection connection;
extern input_t inputAddr;
extern WarField _mainField;
extern WarField _enemyField;
extern WHO_MOVE currentMove;

std::vector<Button> buttons { 
    Button(L"Создать игру", []() {
        connection.connect(true);
        currentState = BUILDING_FIELD_SERVERWAITINGCLIENT;
    }, v2f{50, 600}),
    Button(L"Присоединиться", [](){
        if(!inputAddr.isOpened) {
            inputAddr.isOpened = true; 
            inputAddr.isCompleted = false;
            inputAddr.addr.resize(64);
            return;
        } else inputAddr.isCompleted = true;

        if(inputAddr.isCompleted) {
            for(int i = 0; i < inputAddr.addr.size(); ++i){
                char* str = inputAddr.addr.data();
                if(str[i] == ' ') {
                    str[i] = '\0';
                    break;
                }
            }

            std::cout << "addr: \"" << inputAddr.addr.data() << "\";\n";
            sf::IpAddress addr = sf::IpAddress::LocalHost;
            if(Connection::parseAddr(inputAddr.addr, addr)) {
                if(connection.connect(addr, inputAddr.port)) {
                    inputAddr.isOpened = false; 
                    inputAddr.isCompleted = false;

                    currentState = BUILDING_FIELD;
                }
            }
        }
    }, v2f{250, 600})
};
Button readyButton(L"Готово ", [](){ 
    std::set<uint16_t> collision;
    for(const auto& ship: _mainField.getShips())
    {
        for(int i = 0; i < ship.rank; ++i)
        {
            uint16_t pos = ship._arr[i].x;
            pos <<= 8;
            pos |= ship._arr[i].y;

            if(collision.contains(pos)) {
                std::cout << "Collision!\n";
                return;
            }
            collision.insert(pos);

            if(ship._arr[i].x > 9 || ship._arr[i].y > 9) {
                std::cout << "Not setuped all ships!\n";
                return;
            }
        }
    }

    currentState = BUILDING_FIELD_COMPLETE;
    connection.write(
        Message{.type = Message::type_message::READY_FOR_START}
    );
}, v2f{300, 660});
Button finishRoundButton(L"В меню ", []() {
    currentState = MENU;
}, v2f{300, 660});

void MENU_render(sf::RenderWindow& window)
{
    for(const auto& b: buttons)
    {
        window.draw(b);
    }
}

void BUILDING_render(sf::RenderWindow& window)
{
    _mainField.Field::draw(window, sf::RenderStates::Default);
    window.draw(readyButton);
}

void BUILDING_READY_render(sf::RenderWindow& window)
{
    auto msg = connection.read();
    if(msg) {
        if(msg->type == Message::type_message::READY_FOR_START) {
            currentState = WAR;
            currentMove = connection.isServer() ? WHO_MOVE::ME : WHO_MOVE::ENEMY;
        } else {
            std::cout << "unexcepted type msg: " << (int)msg->type << "\n";
        }
    } else {
        static sf::Text _text(font, L"Ждём противника...", 24);
        window.draw(_text);
    }
}

void SERVERWAIT_CLIENT_render(sf::RenderWindow& window)
{
    sf::Text _text(font, sf::String(L"Ждём подключение по порту: ") + std::to_string(connection.get_port()), 24);
    
    window.draw(_text);

    if(connection.accept()) currentState = BUILDING_FIELD;
}

void WAR_render(sf::RenderWindow& window)
{
    window.draw(_mainField);
    window.draw(_enemyField);

    const auto msg = connection.read();
    if(!msg) return;

    switch(msg->type) {
        case Message::type_message::MAKE_STEP: {

            if(_mainField.haveShip(msg->payload.pos.x, msg->payload.pos.y)) {
                connection.write(
                    Message {
                        .type = Message::type_message::ANSWER_STEP,
                        .payload = {
                            .pos = msg->payload.pos
                        }
                    }
                );
            }

            _mainField._shots.emplace_back(msg->payload.pos);

            if(_mainField.isEnd()) {
                currentState = END_ROUND;
                connection.write(
                    Message {
                        .type = Message::type_message::END_ROUND,
                        .payload = {
                            .who_win = Message::payload_t::who_win_t::YOU
                        }
                    }
                );
            } else {
                currentMove = WHO_MOVE::ME;
            }
        } break;
        case Message::type_message::ANSWER_STEP: {
            _enemyField.addShip(msg->payload.pos.x, msg->payload.pos.y);
        } break;
        case Message::type_message::END_ROUND: {
            currentState = END_ROUND;
        } break;
    }

}

void END_ROUND_render(sf::RenderWindow& window)
{
    sf::Text _text_win(font, L"Конец раунда!\nТы выиграл!", 24);
    sf::Text _text_loose(font, L"Конец раунда!\nТы проиграл.", 24);
        
    window.draw(_mainField.isEnd() ? _text_loose : _text_win);
    window.draw(finishRoundButton);
}