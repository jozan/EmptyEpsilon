#ifndef GAME_STATE_BROADCASTER_H
#define GAME_STATE_BROADCASTER_H

#include "Updatable.h"

class GameStateBroadcaster : public Updatable
{
public:
	GameStateBroadcaster();
	virtual ~GameStateBroadcaster();

	void start();
	void stop();

	virtual void update(float delta) override;

private:
	void broadcastGameState();
};

#endif // GAME_STATE_BROADCASTER_H
