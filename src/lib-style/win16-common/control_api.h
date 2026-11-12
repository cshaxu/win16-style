#ifndef WIN16_CONTROL_API_H
#define WIN16_CONTROL_API_H
#include <windows.h>
/* The containing DLL owns the stable export table. */
#ifndef NTW16S_API
#if defined(NTW16S_STATIC) || defined(NTW16S_BUILD)
#define NTW16S_API
#else
#define NTW16S_API __declspec(dllimport)
#endif
#endif
#endif
