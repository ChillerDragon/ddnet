#ifndef ENGINE_PERMISSIONS_MANAGER_H
#define ENGINE_PERMISSIONS_MANAGER_H

class IPermissionsManager
{
public:
	virtual ~IPermissionsManager() = default;

	virtual bool CanRoleUseCommand(const char *pRoleName, const char *pCommand) = 0;
};

#endif
