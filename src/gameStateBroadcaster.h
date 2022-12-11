#ifndef GAME_STATE_BROADCASTER_H
#define GAME_STATE_BROADCASTER_H

#include "io/http/server.h"
#include "httpScriptAccess.h"
#include "Updatable.h"

class MessageWriter;

class GameStateBroadcaster : public Updatable
{
public:
    GameStateBroadcaster(EEHttpServer *server);
    virtual ~GameStateBroadcaster();

    void start();
    void stop();

    virtual void update(float delta) override;

private:
    char last_sent_message[1024];
    void broadcastGameState();
    EEHttpServer *server;
};

#endif // GAME_STATE_BROADCASTER_H
