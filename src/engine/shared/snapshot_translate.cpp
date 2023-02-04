/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "compression.h"
#include "snapshot.h"
#include "uuid_manager.h"

#include <climits>
#include <cstdlib>

#include <base/math.h>
#include <base/system.h>

#include <engine/shared/translation_context.h>

#include <game/generated/protocol.h>
#include <game/generated/protocol7.h>
#include <game/generated/protocolglue.h>

// CSnapshot

int CSnapshot::TranslateSevenToSix(CSnapshot *pSixSnapDest, CTranslationContext &TranslationContext)
{
	// dbg_msg("snapshot7", "-------------------------------------------------------");
	// dbg_msg("snapshot7", "data_size=%d num_items=%d", m_DataSize, m_NumItems);

	CSnapshotBuilder Builder;
	Builder.Init();

	// hack to put game info in the snap
	// even though in 0.7 we get it as a game message
	if(TranslationContext.m_ShouldSendGameInfo)
	{
		// TODO: should we ever stop putting game info in every snap?
		// dbg_msg("snapshot", "adding fake game info to snap");
		// id 0 should never conflict since
		// this is the only place this item is added
		void *pObj = Builder.NewItem(NETOBJTYPE_GAMEINFO, 0, sizeof(CNetObj_GameInfo));
		if(pObj) // TODO: abort and return error code here
		{
			CNetObj_GameInfo Info6 = {};
			Info6.m_GameFlags = TranslationContext.m_GameFlags;
			Info6.m_ScoreLimit = TranslationContext.m_ScoreLimit;
			Info6.m_TimeLimit = TranslationContext.m_TimeLimit;
			Info6.m_RoundNum = TranslationContext.m_MatchNum;
			Info6.m_RoundCurrent = TranslationContext.m_MatchCurrent;

			mem_copy(pObj, &Info6, sizeof(CNetObj_GameInfo));
		}
		else
			dbg_msg("snapshot", "failed to add game info to snap");
	}

	for(int i = 0; i < m_NumItems; i++)
	{
		const CSnapshotItem *pItem7 = GetItem(i);
		int Size = GetItemSize(i);
		// dbg_msg("snapshot7", "\ttype=%d id=%d size=%d", pItem7->Type(), pItem7->ID(), Size);
		// for(int b = 0; b < Size / 4; b++)
		// 	dbg_msg("snapshot7", "\t\t%3d %12d\t%08x", b, pItem7->Data()[b], pItem7->Data()[b]);

		// the first few items are a full match
		// no translation needed
		if(pItem7->Type() == protocol7::NETOBJTYPE_PLAYERINPUT ||
			pItem7->Type() == protocol7::NETOBJTYPE_PROJECTILE ||
			pItem7->Type() == protocol7::NETOBJTYPE_LASER ||
			pItem7->Type() == protocol7::NETOBJTYPE_FLAG)
		{
			void *pObj = Builder.NewItem(pItem7->Type(), pItem7->ID(), Size);
			if(!pObj)
				continue; // TODO: abort and return error code here

			mem_copy(pObj, pItem7->Data(), Size);
		}
		else if(pItem7->Type() == protocol7::NETOBJTYPE_PICKUP)
		{
			const protocol7::CNetObj_Pickup *pPickup7 = (const protocol7::CNetObj_Pickup *)pItem7->Data();
			CNetObj_Pickup Pickup6 = {};
			Pickup6.m_X = pPickup7->m_X;
			Pickup6.m_Y = pPickup7->m_Y;
			Pickup6.m_Type = pPickup7->m_Type;
			Pickup6.m_Subtype = 0; // TODO: 0.7 removed that field do we need to fill that?
		}
		else if(pItem7->Type() == protocol7::NETOBJTYPE_CHARACTER)
		{
			const protocol7::CNetObj_Character *pChar7 = (const protocol7::CNetObj_Character *)pItem7->Data();
			// dbg_msg("snapshot7", "\t\tchar->m_X = %d", pChar7->m_X);

			void *pObj = Builder.NewItem(NETOBJTYPE_CHARACTER, pItem7->ID(), Size);
			if(!pObj)
				continue; // TODO: abort and return error code here

			mem_copy(pObj, pItem7->Data(), Size);
		}
		else if(pItem7->Type() == protocol7::NETOBJTYPE_PLAYERINFO)
		{
			void *pObj = Builder.NewItem(NETOBJTYPE_PLAYERINFO, pItem7->ID(), sizeof(CNetObj_PlayerInfo));
			if(!pObj)
				continue; // TODO: abort and return error code here

			// got 0.7
			// int m_PlayerFlags;
			// int m_Score;
			// int m_Latency;

			// want 0.6
			// int m_Local;
			// int m_ClientID;
			// int m_Team;
			// int m_Score;
			// int m_Latency;

			const protocol7::CNetObj_PlayerInfo *pInfo7 = (const protocol7::CNetObj_PlayerInfo *)pItem7->Data();
			CNetObj_PlayerInfo Info6 = {};
			Info6.m_Local = TranslationContext.m_LocalClientID == pItem7->ID(); // TODO: is this too hacky?
			Info6.m_ClientID = pItem7->ID(); // TODO: not sure if this actually works
			Info6.m_Team = 0; // TODO: where to get this value from?
			Info6.m_Score = pInfo7->m_Score;
			Info6.m_Latency = pInfo7->m_Latency;

			mem_copy(pObj, &Info6, sizeof(CNetObj_PlayerInfo));
		}
	}

	// unsigned char aTmpBuffer3[CSnapshot::MAX_SIZE];
	// CSnapshot *pTmpBuffer3 = (CSnapshot *)aTmpBuffer3; // TODO: replace by pSixSnapDest if memory allocation is fixed
	int TranslatedSnapSize = Builder.Finish(pSixSnapDest);

	// dbg_msg("snap_trans", "<<<<< DUMP START >>>>>>>>>>>>");
	// pSixSnapDest->DebugDump();
	// dbg_msg("snap_trans", "<<<<< DUMP END >>>>>>>>>>>>");

	return TranslatedSnapSize;
}
