#pragma once
#include <unordered_map>
#include <functional>

template<class T, class CBType = std::function<void()>>
class StateHandler {
    std::unordered_map<T, CBType> _handlers;
public:
    void insert(T key, CBType cb)
    {
        _handlers.insert({key, cb});
    }

    void emit(T key)
    {
        if(_handlers.count(key) > 0) {
            _handlers.at(key)();
        }
    }

    template<class... Args>
    void emit(T key, Args&&... args)
    {
        if(_handlers.count(key) > 0) {
            _handlers.at(key)( std::forward<Args>(args)... );
        }
    }
};