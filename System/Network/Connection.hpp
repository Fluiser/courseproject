#pragma once
#include <SFML/Network.hpp>
#include <memory>
#include <iostream>
#include "../../Graphics/Field.hpp"

#pragma pack(push, 1)
struct Message {
    enum type_message: uint8_t {
        READY_FOR_START = 0,
        MAKE_STEP = 1,
        ANSWER_STEP = 2,
        END_ROUND = 3
    } type;

    union payload_t {
        Position pos;
        enum class who_win_t: uint8_t {
            YOU,
            ME
        } who_win;
    } payload;
};
#pragma pack(pop)

class Connection {
    std::unique_ptr<sf::TcpListener> _listener;
    sf::TcpSocket _socket;

    unsigned _port = 0;
public:

    void connect(bool openServer)
    {
        if(_listener){
            std::cout << "Open server when is already startup\n";
            _listener->close();
        }

        _port = 50000 + (rand()%10000);

        _listener = std::make_unique<sf::TcpListener>();

        auto status = _listener->listen(_port);
        for(int i = 0; i < 4 && status != sf::TcpListener::Status::Done; ++i)
        {
            _port = 50000 + (rand()%10000);
            status = _listener->listen(_port);
        }

        if(status != sf::TcpListener::Status::Done)
        {
            std::cout << "Err open port " << _port << '\n';
            exit(2);
        }
        
        std::cout << "Create room: port is " << _port << '\n';
        _listener->setBlocking(false);

    }

    bool accept()
    {
        return (_listener->accept(_socket) == sf::Socket::Status::Done);
    }

    static bool parseAddr(const std::string& str, sf::IpAddress& addr)
    {
        auto parsedAddr = sf::IpAddress::resolve(str.data());
        if(parsedAddr) 
        {
            addr = *parsedAddr;
            std::cout << "parsed value: " << parsedAddr->toString() << "\n";
            return true;
        }

        return false;
    }

    bool isServer()
    {
        return (bool)_listener;
    }

    bool connect(sf::IpAddress addr, unsigned port)
    {
        if(_listener) {
            std::cout << "Use remote connect when server up\n";
            _listener->close();
            _listener.release();
        }

        _socket.disconnect();
        _socket.setBlocking(true);

        auto state = (_socket.connect(addr, port) == sf::Socket::Status::Done);
        std::cout << "res connect: " << state << "\n";
        return state;
    }

    void write(const Message& msg)
    {
        _socket.setBlocking(true);
        (void)_socket.send(&msg, sizeof(Message));
        _socket.setBlocking(false);
    }

    std::optional<Message> read()
    {
        _socket.setBlocking(false);
            
        Message msg;
        size_t size = 0;
        (void)_socket.receive(&msg, sizeof(msg), size);

        if(size > 0) {
            if(size == sizeof(msg)) {
                return msg;
            } else {
                std::cout << "received " << size << "bytes from " << sizeof(msg) << "\n";
            }
        }


        return {};
    }

    //only for server
    const unsigned& get_port() {return _port;}
};