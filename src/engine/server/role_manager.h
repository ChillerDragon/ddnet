#ifndef ENGINE_SERVER_ROLE_MANAGER_H
#define ENGINE_SERVER_ROLE_MANAGER_H

#include <string>
#include <unordered_map>
#include <vector>

class CRoleManager
{
public:
	std::unordered_map<std::string, std::vector<std::string>> m_RconCommands;
	void GrantAccess(const char *pRoleName, const char *pCommand);
	bool IsAuthorizedToExecute(const char *pRoleName, const char *pCommand);
};

#endif
