#ifndef DUMPER_API_H_
#define DUMPER_API_H_

#ifdef _DEBUG
#pragma comment(lib, "minidumperd.lib")
#else
#pragma comment(lib, "minidumper.lib")
#endif

#ifdef __cplusplus
extern "C" {
#endif

int InstallCrashReport();

#ifdef __cplusplus
}
#endif

#endif
