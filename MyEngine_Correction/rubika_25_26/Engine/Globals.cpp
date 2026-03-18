#include "Globals.h"

#include <Engine/Gameplay/GameMgr.h>
#include <Engine/Render/Ressource/TextureMgr.h>
#include <Engine/Debug/DebugMgr.h>
#include <Engine/Console/LogConsole.h>
#include <Engine/Random/RandomMgr.h>

#include "TaskMgr.h"

Globals gData;

Globals::Globals() : FrameCount(0)
{
	GameMgr = new ::GameMgr();
	TextureMgr = new ::TextureMgr();
	DebugMgr = new ::DebugMgr();
	Console = new ::Logger();
	RandomMgr = new ::RandomMgr();
	TaskMgr = new ::TaskMgr();
}

Globals::~Globals()
{
	Destroy();
}

void Globals::Init()
{
	//GameMgr->Init();
	TextureMgr->Init();
	//DebugMgr->Init();
	Console->Init();
	RandomMgr->Init();
	TaskMgr->Init();
}

void Globals::Shut()
{
	//GameMgr->Shut();
	TextureMgr->Shut();
	//DebugMgr->Shut();
	Console->Shut();
	RandomMgr->Shut();
	TaskMgr->Shut();
}

void Globals::Destroy()
{
	delete DebugMgr;
	DebugMgr = nullptr;

	delete GameMgr;
	GameMgr = nullptr;

	delete TextureMgr;
	TextureMgr = nullptr;

	delete Console;
	Console = nullptr;

	delete RandomMgr;
	RandomMgr = nullptr;
	
	delete TaskMgr;
	TaskMgr = nullptr;
}
