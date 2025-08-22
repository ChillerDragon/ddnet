#include <base/system.h>

#include "access_level.h"

CAccessLevel::CAccessLevel(const char *pName)
{
	str_copy(m_aName, pName);
}

bool CAccessLevel::AllowCommand(const char *pCommand)
{
	if(CanUseRconCommand(pCommand))
		return false;

	m_vCommands.emplace_back(pCommand);
	return true;
}

bool CAccessLevel::CanUseRconCommand(const char *pCommand)
{
	return std::any_of(m_vCommands.begin(), m_vCommands.end(), [pCommand](const std::string& Command) {
		return str_comp(Command.c_str(), pCommand) == 0;
	});
}


CAccessLevel *CAccessLevel::FindAccessLevelByName(std::vector<CAccessLevel> &vAccessLevels, const char *pName)
{
	for(CAccessLevel &AccessLevel : vAccessLevels)
	{
		if(!str_comp(AccessLevel.Name(), pName))
		{
			return &AccessLevel;
		}
	}
	return nullptr;
}
