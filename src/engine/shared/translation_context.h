#ifndef ENGINE_SHARED_TRANSLATION_CONTEXT_H
#define ENGINE_SHARED_TRANSLATION_CONTEXT_H

class CTranslationContext
{
public:
	CTranslationContext()
	{
		m_LocalClientID = -1;
		m_ShouldSendGameInfo = false;
		m_GameFlags = 0;
		m_ScoreLimit = 0;
		m_TimeLimit = 0;
		m_MatchNum = 0;
		m_MatchCurrent = 0;
	}
	int m_LocalClientID;

	bool m_ShouldSendGameInfo;
	int m_GameFlags;
	int m_ScoreLimit;
	int m_TimeLimit;
	int m_MatchNum;
	int m_MatchCurrent;
};

#endif
