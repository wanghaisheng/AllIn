#ifndef HELPER_MINI_DUMPER_H_
#define HELPER_MINI_DUMPER_H_

#include <windows.h>

class MiniDumper {
public:
    static HRESULT  CreateInstance();
    static HRESULT  ReleaseInstance();

public:
    LONG WriteMiniDump(_EXCEPTION_POINTERS *pExceptionInfo);

private:
    void SetMiniDumpFileName(void);
    BOOL GetImpersonationToken(HANDLE* phToken);
    BOOL EnablePrivilege(LPCTSTR pszPriv, HANDLE hToken, TOKEN_PRIVILEGES* ptpOld);
    BOOL RestorePrivilege(HANDLE hToken, TOKEN_PRIVILEGES* ptpOld);

private:
    MiniDumper();
    virtual ~MiniDumper(void);

private:
    TCHAR	m_szMiniDumpPath[MAX_PATH];
    TCHAR	m_szAppPath[MAX_PATH];
};

#endif
