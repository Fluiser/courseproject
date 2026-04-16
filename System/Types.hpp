#pragma once
#include <string>

enum States_t {
    MENU,
    BUILDING_FIELD_SERVERWAITINGCLIENT,
    BUILDING_FIELD,
    BUILDING_FIELD_COMPLETE,
    WAR,
    END_ROUND
};

struct input_t {
    std::string addr;
    int port = 50041;

    bool isOpened = false;
    bool isCompleted = false;
};

enum class WHO_MOVE {
    ME,
    ENEMY
};