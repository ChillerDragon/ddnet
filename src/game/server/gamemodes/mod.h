/* (c) Shereef Marzouk. See "licence DDRace.txt" and the readme.txt in the root of the distribution for more information. */
#ifndef GAME_SERVER_GAMEMODES_MOD_H
#define GAME_SERVER_GAMEMODES_MOD_H

#include <game/server/gamecontroller.h>

class CGameControllerMod : public IGameController
{
public:
	CGameControllerMod(class CGameContext *pGameServer);
	~CGameControllerMod();
	void OnCharacterSpawn(class CCharacter *pChr) override;

	void OnReset() override;
};
#endif // GAME_SERVER_GAMEMODES_MOD_H
