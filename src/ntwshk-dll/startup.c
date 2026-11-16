#include "injection.h"
#include "lib-win32-style/access/startup/install.h"

/* These two ABI entries supply product configuration and terminal behavior. */
DWORD ntwshk_install(HANDLE child,const ntwshk_context *context,ntwshk_create_w create)
{return style_startup_install(child,context,L"ntwshk",create);}
void ntwshk_run_helper(void)
{ExitProcess(style_startup_run_helper(L"ntwshk"));}
