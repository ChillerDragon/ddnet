#include "mod.h"

#include <game/server/entities/character.h>
#include <game/server/player.h>

#define GAME_TYPE_NAME "Mod"
#define TEST_TYPE_NAME "TestMod"

CGameControllerMod::CGameControllerMod(class CGameContext *pGameServer) :
	IGameController(pGameServer)
{
	m_pGameType = g_Config.m_SvTestingCommands ? TEST_TYPE_NAME : GAME_TYPE_NAME;
}

CGameControllerMod::~CGameControllerMod() = default;

void CGameControllerMod::OnReset()
{
	IGameController::OnReset();
	Teams().Reset();
}

void CGameControllerMod::OnCharacterSpawn(CCharacter *pChr)
{
	IGameController::OnCharacterSpawn(pChr);
	pChr->SetTeams(&Teams());
	pChr->SetTeleports(&m_TeleOuts, &m_TeleCheckOuts);
	Teams().OnCharacterSpawn(pChr->GetPlayer()->GetCID());
}
