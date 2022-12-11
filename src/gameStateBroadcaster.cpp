#include <type_traits>
#include <iostream>
#include "dtoa/dtoa_milo.h"

#include "gameGlobalInfo.h"
#include "gameStateBroadcaster.h"
#include "spaceObjects/spaceship.h"

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
        *ptr = '\0';
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

GameStateBroadcaster::GameStateBroadcaster(EEHttpServer *server)
{
}

GameStateBroadcaster::~GameStateBroadcaster()
{
    stop();
}

void GameStateBroadcaster::start()
{
    LOG(INFO) << "GameStateBroadcaster: start";
}

void GameStateBroadcaster::stop()
{
    LOG(INFO) << "GameStateBroadcaster: stop";
}

void GameStateBroadcaster::update(float delta)
{
    broadcastGameState();
}

void GameStateBroadcaster::broadcastGameState()
{
    try
    {
        if (engine->getGameSpeed() == 0.0f)
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

                LOG(INFO) << "  - message size: " << sizeof(message_buffer);

                server->broadcastGameState(string(message_buffer));

                memcpy(last_sent_message, message_buffer, sizeof(message_buffer));
            }
        }
    }
    catch (std::exception &e)
    {
        LOG(WARNING) << "GameStateBroadcaster: error: " << e.what();
    }
}
