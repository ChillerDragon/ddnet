#ifndef ENGINE_SERVER_AUTHMANAGER_H
#define ENGINE_SERVER_AUTHMANAGER_H

#include <vector>

#include <base/hash.h>

#define SALT_BYTES 8

class CAuthManager
{
private:
	struct CKey
	{
		char m_aIdent[64];
		MD5_DIGEST m_Pw;
		unsigned char m_aSalt[SALT_BYTES];
		char m_aLevel[512];
	};
	std::vector<CKey> m_vKeys;

	int m_aDefault[3];
	bool m_Generated;

public:
	typedef void (*FListCallback)(const char *pIdent, const char *pLevel, void *pUser);

	CAuthManager();

	void Init();
	int AddKeyHash(const char *pIdent, MD5_DIGEST Hash, const unsigned char *pSalt, const char *pAuthLevel);
	int AddKey(const char *pIdent, const char *pPw, const char *pAuthLevel);
	void RemoveKey(int Slot);
	int FindKey(const char *pIdent) const;
	bool CheckKey(int Slot, const char *pPw) const;
	int DefaultKey(int AuthLevel) const;
	const char *KeyLevel(int Slot) const;
	const char *KeyIdent(int Slot) const;
	bool IsValidIdent(const char *pIdent) const;
	void UpdateKeyHash(int Slot, MD5_DIGEST Hash, const unsigned char *pSalt, const char * pAuthLevel);
	void UpdateKey(int Slot, const char *pPw, const char *pAuthLevel);
	void ListKeys(FListCallback pfnListCallbac, void *pUser);
	void AddDefaultKey(const char *pLevel, const char *pPw);
	bool IsGenerated() const;
	int NumNonDefaultKeys() const;
};

#endif //ENGINE_SERVER_AUTHMANAGER_H
