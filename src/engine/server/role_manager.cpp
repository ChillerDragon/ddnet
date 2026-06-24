#include "role_manager.h"
#include <base/str.h>

void CRoleManager::GrantAccess(const char *pRoleName, const char *pCommand)
{
	m_RconCommands[pRoleName].emplace_back(pCommand);
}

bool CRoleManager::IsAuthorizedToExecute(const char *pRoleName, const char *pCommand)
{
	auto Role = m_RconCommands.find(pRoleName);
	if(Role == m_RconCommands.end())
		return false;

	for(auto Command : Role->second)
		if(!str_comp(Command.c_str(), pCommand))
			return true;
	return false;
}
