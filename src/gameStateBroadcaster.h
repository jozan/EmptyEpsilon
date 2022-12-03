#ifndef GAME_STATE_BROADCASTER_H
#define GAME_STATE_BROADCASTER_H

#include "Updatable.h"

class WebSocketSession;
class MessageWriter;

class GameStateBroadcaster : public Updatable
{
    std::shared_ptr<WebSocketSession> ws;

public:
    GameStateBroadcaster();
    virtual ~GameStateBroadcaster();

    void start();
    void stop();

    virtual void update(float delta) override;

private:
    char last_sent_message[1024];
    void wsHandler();
    void broadcastGameState();
};

#endif // GAME_STATE_BROADCASTER_H
