#include "authmanager.h"
#include <algorithm>
#include <base/hash_ctxt.h>
#include <base/system.h>
#include <engine/shared/config.h>
#include <generated/protocol.h>

#define ADMIN_IDENT "default_admin"
#define MOD_IDENT "default_mod"
#define HELPER_IDENT "default_helper"

int rcon_rank_to_network(int Rank)
{
	switch (Rank) {
		case RANK_ADMIN:
			return AUTHED_ADMIN;
		case RANK_MODERATOR:
			return AUTHED_MOD;
		case RANK_HELPER:
			return AUTHED_HELPER;
		case RANK_NONE:
			return AUTHED_NO;
	}

	dbg_assert(Rank > RANK_NONE && Rank < RANK_ADMIN, "Rank %d not in the allowed range %d-%d.", Rank, RANK_NONE + 1, RANK_ADMIN - 1);
	return Rank;
}

bool CRconRole::CanUseRconCommand(const char *pCommand)
{
	return std::ranges::any_of(
		m_vRconCommands,
		[pCommand](const std::string &Command) {
			return str_comp_nocase(Command.c_str(), pCommand) == 0;
		});
}

bool CRconRole::AllowCommand(const char *pCommand)
{
	if(CanUseRconCommand(pCommand))
		return false;

	m_vRconCommands.emplace_back(pCommand);
	return true;
}

const char *CAuthManager::AuthLevelToRoleName(int AuthLevel)
{
	switch(AuthLevel)
	{
	case AUTHED_ADMIN:
		return ROLE_NAME_ADMIN;
	case AUTHED_MOD:
		return ROLE_NAME_MODERATOR;
	case AUTHED_HELPER:
		return ROLE_NAME_HELPER;
	}

	dbg_assert(false, "Invalid auth level: %d", AuthLevel);
	dbg_break();
}

static int RoleNameToAuthLevel(const char *pRoleName)
{
	if(!str_comp(pRoleName, ROLE_NAME_ADMIN))
		return AUTHED_ADMIN;
	if(!str_comp(pRoleName, ROLE_NAME_MODERATOR))
		return AUTHED_MOD;
	if(!str_comp(pRoleName, ROLE_NAME_HELPER))
		return AUTHED_HELPER;

	dbg_assert(false, "Invalid role name: %s", pRoleName);
	dbg_break();
}

static MD5_DIGEST HashPassword(const char *pPassword, const unsigned char aSalt[SALT_BYTES])
{
	// Hash the password and the salt
	MD5_CTX Md5;
	md5_init(&Md5);
	md5_update(&Md5, (unsigned char *)pPassword, str_length(pPassword));
	md5_update(&Md5, aSalt, SALT_BYTES);
	return md5_finish(&Md5);
}

CAuthManager::CAuthManager()
{
	m_aDefault[0] = -1;
	m_aDefault[1] = -1;
	m_aDefault[2] = -1;
	m_Generated = false;

	AddRole(ROLE_NAME_ADMIN, RANK_ADMIN);
	AddRole(ROLE_NAME_MODERATOR, RANK_MODERATOR);
	AddRole(ROLE_NAME_HELPER, RANK_HELPER);
}

void CAuthManager::Init()
{
	size_t NumDefaultKeys = 0;
	if(g_Config.m_SvRconPassword[0])
		NumDefaultKeys++;
	if(g_Config.m_SvRconModPassword[0])
		NumDefaultKeys++;
	if(g_Config.m_SvRconHelperPassword[0])
		NumDefaultKeys++;

	auto It = std::find_if(m_vKeys.begin(), m_vKeys.end(), [](CKey Key) { return str_comp(Key.m_aIdent, DEFAULT_SAVED_RCON_USER) == 0; });
	if(It != m_vKeys.end())
		NumDefaultKeys++;

	if(m_vKeys.size() == NumDefaultKeys && !g_Config.m_SvRconPassword[0])
	{
		secure_random_password(g_Config.m_SvRconPassword, sizeof(g_Config.m_SvRconPassword), 6);
		AddDefaultKey(ROLE_NAME_ADMIN, g_Config.m_SvRconPassword);
		m_Generated = true;
	}
}

int CAuthManager::AddKeyHash(const char *pIdent, MD5_DIGEST Hash, const unsigned char *pSalt, const char *pRoleName)
{
	if(FindKey(pIdent) >= 0)
		return -1;

	CKey Key;
	str_copy(Key.m_aIdent, pIdent);
	Key.m_Pw = Hash;
	mem_copy(Key.m_aSalt, pSalt, SALT_BYTES);
	Key.m_pRole = FindRole(pRoleName);
	dbg_assert(Key.m_pRole != nullptr, "invalid role name '%s' role=%p", pRoleName, Key.m_pRole);

	m_vKeys.push_back(Key);
	return m_vKeys.size() - 1;
}

int CAuthManager::AddKey(const char *pIdent, const char *pPw, const char *pRoleName)
{
	// Generate random salt
	unsigned char aSalt[SALT_BYTES];
	secure_random_fill(aSalt, SALT_BYTES);
	return AddKeyHash(pIdent, HashPassword(pPw, aSalt), aSalt, pRoleName);
}

void CAuthManager::RemoveKey(int Slot)
{
	m_vKeys.erase(m_vKeys.begin() + Slot);
	// Update indices of default keys
	for(int &Default : m_aDefault)
	{
		if(Default == Slot)
		{
			Default = -1;
		}
		else if(Default > Slot)
		{
			--Default;
		}
	}
}

int CAuthManager::FindKey(const char *pIdent) const
{
	for(size_t i = 0; i < m_vKeys.size(); i++)
		if(!str_comp(m_vKeys[i].m_aIdent, pIdent))
			return i;

	return -1;
}

bool CAuthManager::CheckKey(int Slot, const char *pPw) const
{
	if(Slot < 0 || Slot >= (int)m_vKeys.size())
		return false;
	return m_vKeys[Slot].m_Pw == HashPassword(pPw, m_vKeys[Slot].m_aSalt);
}

int CAuthManager::DefaultKey(const char *pRoleName) const
{
	if(!str_comp(pRoleName, ROLE_NAME_ADMIN))
		return m_aDefault[std::size(m_aDefault) - AUTHED_ADMIN];
	if(!str_comp(pRoleName, ROLE_NAME_MODERATOR))
		return m_aDefault[std::size(m_aDefault) - AUTHED_MOD];
	if(!str_comp(pRoleName, ROLE_NAME_HELPER))
		return m_aDefault[std::size(m_aDefault) - AUTHED_HELPER];
	return 0;
}

int CAuthManager::KeyLevel(int Slot) const
{
	if(Slot < 0 || Slot >= (int)m_vKeys.size())
		return false;
	return m_vKeys[Slot].m_pRole->Rank();
}

CRconRole *CAuthManager::KeyRole(int Slot) const
{
	if(Slot < 0 || Slot >= (int)m_vKeys.size())
		return nullptr;
	return m_vKeys[Slot].m_pRole;
}

const char *CAuthManager::KeyIdent(int Slot) const
{
	if(Slot < 0 || Slot >= (int)m_vKeys.size())
		return nullptr;
	return m_vKeys[Slot].m_aIdent;
}

bool CAuthManager::IsValidIdent(const char *pIdent) const
{
	return str_length(pIdent) < (int)sizeof(CKey().m_aIdent);
}

void CAuthManager::UpdateKeyHash(int Slot, MD5_DIGEST Hash, const unsigned char *pSalt, const char *pRoleName)
{
	if(Slot < 0 || Slot >= (int)m_vKeys.size())
		return;

	CKey *pKey = &m_vKeys[Slot];
	pKey->m_Pw = Hash;
	mem_copy(pKey->m_aSalt, pSalt, SALT_BYTES);
	pKey->m_pRole = FindRole(pRoleName);
}

void CAuthManager::UpdateKey(int Slot, const char *pPw, const char *pRoleName)
{
	if(Slot < 0 || Slot >= (int)m_vKeys.size())
		return;

	// Generate random salt
	unsigned char aSalt[SALT_BYTES];
	secure_random_fill(aSalt, SALT_BYTES);
	UpdateKeyHash(Slot, HashPassword(pPw, aSalt), aSalt, pRoleName);
}

void CAuthManager::ListKeys(FListCallback pfnListCallback, void *pUser)
{
	for(auto &Key : m_vKeys)
		pfnListCallback(Key.m_aIdent, Key.m_pRole->Rank(), pUser);
}

void CAuthManager::AddDefaultKey(const char *pRoleName, const char *pPw)
{
	int Level = RoleNameToAuthLevel(pRoleName);
	dbg_assert(Level >= AUTHED_HELPER && Level <= AUTHED_ADMIN, "default role with name '%s' not found.", pRoleName);

	static const char s_aaIdents[3][sizeof(HELPER_IDENT)] = {ADMIN_IDENT, MOD_IDENT, HELPER_IDENT};
	int Index = AUTHED_ADMIN - Level;
	if(m_aDefault[Index] >= 0)
		return; // already exists
	m_aDefault[Index] = AddKey(s_aaIdents[Index], pPw, AuthLevelToRoleName(Level));
}

bool CAuthManager::IsGenerated() const
{
	return m_Generated;
}

int CAuthManager::NumNonDefaultKeys() const
{
	int DefaultCount = std::count_if(std::begin(m_aDefault), std::end(m_aDefault), [](int Slot) {
		return Slot >= 0;
	});
	return m_vKeys.size() - DefaultCount;
}

CRconRole *CAuthManager::FindRole(const char *pName)
{
	auto It = m_Roles.find(pName);
	if(It == m_Roles.end())
		return nullptr;
	return &It->second;
}

bool CAuthManager::AddRole(const char *pName, int Rank)
{
	if(FindRole(pName))
		return false;

	m_Roles.insert({pName, CRconRole(pName, Rank)});
	return true;
}

bool CAuthManager::CanRoleUseCommand(const char *pRoleName, const char *pCommand)
{
	CRconRole *pRole = FindRole(pRoleName);
	if(!pRole)
		return false;

	return pRole->CanUseRconCommand(pCommand);
}

void CAuthManager::GetRoleNames(char *pBuf, size_t BufSize)
{
	size_t WriteLen = 1;
	pBuf[0] = '\0';

	bool First = true;
	for(const auto &It : m_Roles)
	{
		if(!First)
			WriteLen += str_length(It.second.Name());
		WriteLen += str_length(It.second.Name());
		if(WriteLen >= BufSize)
			break;

		if(!First)
			str_append(pBuf, ", ", BufSize - WriteLen);
		str_append(pBuf, It.second.Name(), BufSize - WriteLen);
		First = false;
	}
}

