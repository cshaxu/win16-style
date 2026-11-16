#ifndef NTW16S_GLOBAL_POLICY_H
#define NTW16S_GLOBAL_POLICY_H
#include <windows.h>

#include "lib-win32/process-global/policy.h"

BOOL ntw16s_global_image_allowed(const WCHAR *image,const WCHAR *windows);
#endif
