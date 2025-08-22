#ifndef ENGINE_SHARED_ACCESS_LEVEL_H
#define ENGINE_SHARED_ACCESS_LEVEL_H

#include <string>
#include <vector>

class CAccessLevel
{
	char m_aName[512];
	std::vector<std::string> m_vCommands;
public:
	CAccessLevel(const char *pName);
	const char *Name() { return m_aName; }
	bool AllowCommand(const char *pCommand);
	bool CanUseRconCommand(const char *pCommand);

	static CAccessLevel *FindAccessLevelByName(std::vector<CAccessLevel> &vAccessLevels, const char *pName);
};

#endif
