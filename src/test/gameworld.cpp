#include "test.h"
#include <gtest/gtest.h>

#include <base/logger.h>
#include <base/system.h>
#include <engine/engine.h>
#include <engine/server/server.h>
#include <engine/server/server_logger.h>
#include <engine/shared/assertion_logger.h>
#include <engine/shared/config.h>
#include <game/generated/protocol.h>
#include <game/server/entities/character.h>
#include <game/server/gamecontext.h>
#include <game/server/gameworld.h>
#include <game/version.h>

#include <thread>

bool IsInterrupted()
{
	return false;
}

std::vector<std::string> FakeQueue;
std::vector<std::string> FetchAndroidServerCommandQueue()
{
	return FakeQueue;
}

class GameWorld : public ::testing::Test
{
public:
	IGameServer *m_pGameServer = nullptr;
	IKernel *m_pKernel;
	std::shared_ptr<CServerLogger> m_pServerLogger = nullptr;

	CGameContext *GameServer()
	{
		return (CGameContext *)m_pGameServer;
	}

	GameWorld()
	{
		std::shared_ptr<CFutureLogger> pFutureConsoleLogger = std::make_shared<CFutureLogger>();

		CServer *pServer = CreateServer();

		m_pKernel = IKernel::Create();
		m_pKernel->RegisterInterface(pServer);

		IEngine *pEngine = CreateEngine(GAME_NAME, pFutureConsoleLogger, (2 * std::thread::hardware_concurrency()) + 2);
		m_pKernel->RegisterInterface(pEngine);

		const char *apArgs[] = {"DDNet"};
		IStorage *pStorage = CreateStorage(IStorage::EInitializationType::SERVER, 1, apArgs);
		EXPECT_NE(pStorage, nullptr);
		m_pKernel->RegisterInterface(pStorage);

		IConsole *pConsole = CreateConsole(CFGFLAG_SERVER | CFGFLAG_ECON).release();
		m_pKernel->RegisterInterface(pConsole);

		IConfigManager *pConfigManager = CreateConfigManager();
		m_pKernel->RegisterInterface(pConfigManager);

		IEngineMap *pEngineMap = CreateEngineMap();
		m_pKernel->RegisterInterface(pEngineMap);
		m_pKernel->RegisterInterface(static_cast<IMap *>(pEngineMap), false);

		IEngineAntibot *pEngineAntibot = CreateEngineAntibot();
		m_pKernel->RegisterInterface(pEngineAntibot);
		m_pKernel->RegisterInterface(static_cast<IAntibot *>(pEngineAntibot), false);

		m_pGameServer = CreateGameServer();
		m_pKernel->RegisterInterface(m_pGameServer);

		pEngine->Init();
		pConsole->Init();
		pConfigManager->Init();

		pServer->RegisterCommands();

		m_pServerLogger = std::make_shared<CServerLogger>(pServer);
		pEngine->SetAdditionalLogger(m_pServerLogger);

		EXPECT_NE(pServer->LoadMap("coverage"), 0);
		m_pGameServer->OnInit(nullptr);
	};

	~GameWorld()
	{
		// m_pGameServer->OnShutdown(nullptr);
		// m_pServerLogger->OnServerDeletion();
		delete m_pKernel;
	};
};

TEST_F(GameWorld, ClosestCharacter)
{
}
