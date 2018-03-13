
#pragma once

#include <ncltech\Scene.h>
#include <ncltech\NetworkBase.h>

//Basic Network Example

class Maze_Client : public Scene
{
public:
	Maze_Client(const std::string& friendly_name);

	virtual void OnInitializeScene() override;
	virtual void OnCleanupScene() override;
	virtual void OnUpdateScene(float dt) override;


	void ProcessNetworkEvent(const ENetEvent& evnt);

protected:
	GameObject* box;

	NetworkBase network;
	ENetPeer*	serverConnection;
};