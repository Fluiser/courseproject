#include <SFML/Graphics.hpp>
#include <imgui.h>
#include "imgui-SFML.h"
#include <thread>
#include <chrono>
#include "./Graphics/WarField.hpp"
#include "./System/Network/Connection.hpp"
#include "./System/StateHandler.hpp"
#include "./System/Types.hpp"
#include "./Graphics/renderer.hpp"
#include "./System/events.hpp"

sf::Texture background_texture;
sf::Texture field_background_texture;

WarField _mainField(20, 20);
WarField _enemyField(700, 20);

sf::Font font;
Connection connection;

States_t currentState = MENU;
WHO_MOVE currentMove;

input_t inputAddr; 

StateHandler<States_t> render_handler;
StateHandler<States_t, std::function<void(sf::RenderWindow& window, const std::optional<sf::Event>& event)>> event_hanler;


void render(sf::RenderWindow& window)
{
    sf::Sprite background(background_texture);
    background.setPosition({0,0});

    window.clear(sf::Color::Black);
    window.draw(background);

    render_handler.emit(currentState);
   
    if(inputAddr.isOpened) ImGui::SFML::Render(window);
    window.display();
}

int main()
{
    if(!font.openFromFile("f.ttf")) {
        std::cout << "Err load font\n";
        return 1;
    }

    if(!background_texture.loadFromFile("background.png")) {
        std::cout << "Err load background.png\n";
        return 1;
    }

    if(!field_background_texture.loadFromFile("field.png")) {
        std::cout << "Err load field.png\n";
        return 1;
    }
    inputAddr.addr.resize(64, '\0');
    _mainField.spawnShips();

    sf::RenderWindow window(sf::VideoMode(v2u{1280, 720}), "mainwindow");
    window.setFramerateLimit(30);

    sf::Clock timer;

    if(!ImGui::SFML::Init(window)) {
        std::cout << "Err init imgui\n";
        return 1;
    }

    ImGui::SetNextWindowPos(ImVec2(500, 550), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(200, 50), ImGuiCond_FirstUseEver);

    render_handler.insert(States_t::MENU, [&window]() { MENU_render(window); });
    render_handler.insert(States_t::BUILDING_FIELD, [&window]() { BUILDING_render(window); });
    render_handler.insert(States_t::BUILDING_FIELD_COMPLETE, [&window]() { BUILDING_READY_render(window); });
    render_handler.insert(States_t::BUILDING_FIELD_SERVERWAITINGCLIENT, [&window]() { SERVERWAIT_CLIENT_render(window); });
    render_handler.insert(States_t::WAR, [&window]() { WAR_render(window); });
    render_handler.insert(States_t::END_ROUND, [&window]() { END_ROUND_render(window); });

    event_hanler.insert(States_t::MENU, MENU_event);
    event_hanler.insert(States_t::BUILDING_FIELD, BUILDING_event);
    event_hanler.insert(States_t::WAR, WAR_event);
    event_hanler.insert(States_t::END_ROUND, END_ROUND_event);

    while(window.isOpen())
    {
        while(const std::optional event = window.pollEvent())
        {
            event_hanler.emit(currentState, window, event);

            if(inputAddr.isOpened) ImGui::SFML::ProcessEvent(window, *event);

            if(event->getIf<sf::Event::Closed>()) {
                return 0; 
            }
        }

        if(inputAddr.isOpened)
        {
            ImGui::SFML::Update(window, timer.restart());
            ImGui::Begin("Remote addr for connecting");
            ImGui::InputText("Address", inputAddr.addr.data(), inputAddr.addr.size());
            ImGui::InputInt("port", &inputAddr.port);
            ImGui::End();
        }
        render(window);
    }

    return 0;
}