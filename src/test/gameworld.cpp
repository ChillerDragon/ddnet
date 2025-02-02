#include "base/math.h"
#include "engine/engine.h"
#include "engine/server/server.h"
#include "engine/server/server_logger.h"
#include "engine/shared/assertion_logger.h"
#include "engine/shared/config.h"
#include "game/server/gamecontext.h"
#include "test.h"
#include <gtest/gtest.h>

#include <base/system.h>
#include <game/version.h>
#include "base/logger.h"
#include <game/server/gameworld.h>
#include "game/server/entities/character.h"
#include "game/generated/protocol.h"


#include <mutex>
#include <optional>
#include <vector>
#include <csignal>
#include <thread>

bool IsInterrupted()
{
	return false;
}

TEST(GameWorld, basic)
{
	std::vector<std::shared_ptr<ILogger>> vpLoggers;
	std::shared_ptr<ILogger> pStdoutLogger;
	if(pStdoutLogger)
	{
		vpLoggers.push_back(pStdoutLogger);
	}
	std::shared_ptr<CFutureLogger> pFutureFileLogger = std::make_shared<CFutureLogger>();
	vpLoggers.push_back(pFutureFileLogger);
	std::shared_ptr<CFutureLogger> pFutureConsoleLogger = std::make_shared<CFutureLogger>();
	vpLoggers.push_back(pFutureConsoleLogger);
	std::shared_ptr<CFutureLogger> pFutureAssertionLogger = std::make_shared<CFutureLogger>();
	vpLoggers.push_back(pFutureAssertionLogger);


	CServer *pServer = CreateServer();
	pServer->SetLoggers(pFutureFileLogger, std::move(pStdoutLogger));

	IKernel *pKernel = IKernel::Create();
	pKernel->RegisterInterface(pServer);

	// create the components
	IEngine *pEngine = CreateEngine(GAME_NAME, pFutureConsoleLogger, 2 * std::thread::hardware_concurrency() + 2);
	pKernel->RegisterInterface(pEngine);

	const char *apArgs[] = {"DDNet"};
	IStorage *pStorage = CreateStorage(IStorage::EInitializationType::SERVER, 1, apArgs);
	EXPECT_NE(pStorage, nullptr);
	pKernel->RegisterInterface(pStorage);

	pFutureAssertionLogger->Set(CreateAssertionLogger(pStorage, GAME_NAME));

	IConsole *pConsole = CreateConsole(CFGFLAG_SERVER | CFGFLAG_ECON).release();
	pKernel->RegisterInterface(pConsole);

	IConfigManager *pConfigManager = CreateConfigManager();
	pKernel->RegisterInterface(pConfigManager);

	IEngineMap *pEngineMap = CreateEngineMap();
	pKernel->RegisterInterface(pEngineMap); // IEngineMap
	pKernel->RegisterInterface(static_cast<IMap *>(pEngineMap), false);

	IEngineAntibot *pEngineAntibot = CreateEngineAntibot();
	pKernel->RegisterInterface(pEngineAntibot); // IEngineAntibot
	pKernel->RegisterInterface(static_cast<IAntibot *>(pEngineAntibot), false);

	IGameServer *pGameServer = CreateGameServer();
	pKernel->RegisterInterface(pGameServer);

	pEngine->Init();
	pConsole->Init();
	pConfigManager->Init();

	// register all console commands
	pServer->RegisterCommands();

	pConfigManager->SetReadOnly("sv_max_clients", true);
	pConfigManager->SetReadOnly("sv_test_cmds", true);
	pConfigManager->SetReadOnly("sv_rescue", true);

	pFutureFileLogger->Set(log_logger_noop());

	auto pServerLogger = std::make_shared<CServerLogger>(pServer);
	pEngine->SetAdditionalLogger(pServerLogger);

	// int Ret = pServer->Run();
	EXPECT_NE(pServer->LoadMap("coverage"), 0);
	pGameServer->OnInit(nullptr);
	CGameContext *pGame = (CGameContext *)pGameServer;

	// test
	CNetObj_PlayerInput Input = {};

	CCharacter *pChr1 = new(0) CCharacter(&pGame->m_World, Input);
	pChr1->m_Pos = vec2(0, 0);
	pGame->m_World.InsertEntity(pChr1);


	CCharacter *pChr2 = new(1) CCharacter(&pGame->m_World, Input);
	pChr2->m_Pos = vec2(10, 10);
	pGame->m_World.InsertEntity(pChr2);

	CCharacter *pClosest = pGame->m_World.ClosestCharacter(vec2(1, 1), 20, nullptr);
	EXPECT_EQ(pClosest, pChr1);

	// cleanup
	pServerLogger->OnServerDeletion();
	delete pKernel;
}
