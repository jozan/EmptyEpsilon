#include "gameStateBroadcaster.h"
#include "gameGlobalInfo.h"
#include "spaceObjects/spaceship.h"

GameStateBroadcaster::GameStateBroadcaster()
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
	LOG(INFO) << "GameStateBroadcaster: broadcastGameState";
	foreach (SpaceObject, obj, space_object_list)
	{
		P<SpaceShip> ship = obj;

		if (ship)
		{
			LOG(INFO) << "GameStateBroadcaster: ship: " << ship->getCallSign() << ", id: " << ship->getMultiplayerId();
			string shieldsActive = ship->getShieldsActive() ? "on" : "off";
			LOG(INFO) << "GameStateBroadcaster: hull: " << ship->getHull() << ", shields: (" << shieldsActive << ")";
		}
	}
}
