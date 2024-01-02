#ifndef ENGINE_SHARED_PROTOCOLGLUE_H
#define ENGINE_SHARED_PROTOCOLGLUE_H

#include <engine/shared/packer.h>

class CTranslatedGameMessage
{
public:
	int m_MsgId;
	CUnpacker m_Unpacker;

	bool IsValid() { return m_MsgId; }

	CTranslatedGameMessage()
	{
		m_MsgId = 0;
	}

	CTranslatedGameMessage(int MsgId) :
		m_MsgId(MsgId)
	{
	}
};

int PlayerFlags_SevenToSix(int Flags);
int PlayerFlags_SixToSeven(int Flags);
void PickupType_SevenToSix(int Type7, int &Type6, int &SubType6);
int PickupType_SixToSeven(int Type6, int SubType6);

#endif
