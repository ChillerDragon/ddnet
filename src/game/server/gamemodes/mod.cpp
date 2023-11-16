/* (c) Shereef Marzouk. See "licence DDRace.txt" and the readme.txt in the root of the distribution for more information. */
/* Based on Race mod stuff and tweaked by GreYFoX@GTi and others to fit our DDRace needs. */
#include "mod.h"

#include <engine/server.h>
#include <engine/shared/config.h>
#include <game/mapitems.h>
#include <game/server/entities/character.h>
#include <game/server/gamecontext.h>
#include <game/server/player.h>
#include <game/server/score.h>
#include <game/version.h>

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