#include <type_traits>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <iostream>
#include "dtoa/dtoa_milo.h"

#include "gameGlobalInfo.h"
#include "preferenceManager.h"
#include "gameStateBroadcaster.h"
#include "spaceObjects/spaceship.h"

namespace net = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
using net::ip::tcp;

class MessageWriter
{
public:
    MessageWriter(char *&ptr)
        : ptr(ptr), first(true)
    {
        *ptr++ = '[';
    }

    ~MessageWriter()
    {
    }

    void end()
    {
        *ptr++ = ']';
    }

    template <typename T>
    void write(const char *key, const T &value)
    {
        if (!first)
            *ptr++ = ',';
        *ptr++ = '"';
        while (*key)
            *ptr++ = *key++;
        *ptr++ = '"';
        *ptr++ = ',';
        first = false;
        writeValue(value);
    }

private:
    void writeValue(bool b)
    {
        const char *c = "0";
        if (b)
            c = "1";
        while (*c)
            *ptr++ = *c++;
    }
    void writeValue(int i) { ptr += sprintf(ptr, "%d", i); }
    void writeValue(float _f)
    {
        dtoa_milo(_f, ptr);
        ptr += strlen(ptr);
    }
    void writeValue(const char *value)
    {
        *ptr++ = '"';
        while (*value)
            *ptr++ = *value++;
        *ptr++ = '"';
    }
    void writeValue(const string &value)
    {
        const char *str = value.c_str();
        *ptr++ = '"';
        while (*str)
            *ptr++ = *str++;
        *ptr++ = '"';
    }

    char *&ptr;
    bool first;
};

class WebSocketSession : public std::enable_shared_from_this<WebSocketSession>
{
    tcp::resolver resolver_;
    websocket::stream<tcp::socket> ws_;
    beast::flat_buffer buffer_;
    std::string host_;

public:
    explicit WebSocketSession(boost::asio::io_context &ioc)
        : resolver_(ioc), ws_(ioc) {}

    void connect(std::string host, std::string port)
    {
        host_ = host;
        auto const results = resolver_.resolve(host, port);
        auto ep = net::connect(ws_.next_layer(), results);

        host_ += ':' + std::to_string(ep.port());

        ws_.handshake(host_, "/");
        std::cout << "ws: connected" << std::endl;
    }

    void read()
    {
        ws_.read(buffer_);

        auto out = beast::buffers_to_string(buffer_.data());

        std::cout << out << std::endl;
    }

    void write(const char *message)
    {
        ws_.write(net::buffer(message, strlen(message)));
    }

    void write(std::string message)
    {
        ws_.write(net::buffer(message));
        std::cout << "ws: writing msg" << std::endl;
    }

    void closeConnection()
    {
        if (ws_.is_open())
        {
            ws_.close(websocket::close_code::normal);
            std::cout << "ws: connection closed" << std::endl;
        }
    }

    bool isOpen()
    {
        return ws_.is_open();
    }
};

GameStateBroadcaster::GameStateBroadcaster()
{
    boost::asio::io_context ioc;
    ws = std::make_shared<WebSocketSession>(ioc);
}

GameStateBroadcaster::~GameStateBroadcaster()
{
    stop();
}

void GameStateBroadcaster::start()
{
    LOG(INFO) << "GameStateBroadcaster: start";

    auto host = PreferencesManager::get("game_state_broadcaster_address", "localhost");
    auto port = PreferencesManager::get("game_state_broadcaster_port", "8080");

    if (ws)
        ws->connect(host, port);
}

void GameStateBroadcaster::stop()
{
    LOG(INFO) << "GameStateBroadcaster: stop";
    if (ws)
        ws->closeConnection();
}

void GameStateBroadcaster::update(float delta)
{
    broadcastGameState();
}

void GameStateBroadcaster::broadcastGameState()
{
    try
    {
        if (engine->getGameSpeed() == 0.0f || !ws || !ws->isOpen())
            return;

        if (my_spaceship)
        {
            static char message_buffer[1024];
            char *ptr = message_buffer;
            MessageWriter message(ptr);

            message.write("callsign", my_spaceship->getCallSign());
            message.write("hull", my_spaceship->getHull());
            message.write("shieldsActive", my_spaceship->getShieldsActive());
            message.end();

            if (memcmp(&message_buffer, &last_sent_message, sizeof(message_buffer)) != 0)
            {
                LOG(INFO) << "GameStateBroadcaster: broadcastGameState";
                LOG(INFO) << "  - ship: " << my_spaceship->getCallSign() << " (id: " << my_spaceship->getMultiplayerId() << ")";
                LOG(INFO) << "  - hull: " << my_spaceship->getHull();
                string shieldsActive = my_spaceship->getShieldsActive() ? "on" : "off";
                LOG(INFO) << "  - shields: " << shieldsActive;

                if (ws && ws->isOpen())
                    ws->write(message_buffer);
                memcpy(last_sent_message, message_buffer, sizeof(message_buffer));
            }
        }
    }
    catch (std::exception &e)
    {
        LOG(WARNING) << "GameStateBroadcaster: error: " << e.what();
    }
}
