#define UNICODE
#include <windows.h>
#include "native_thunk.h"
/* Test-only DLL: its class is owned outside the EXE. The synthetic delayed
 * native initialization exercises creation-scope admission; it does not
 * claim to reproduce a Delphi runtime's actual creation order. */
#ifdef TEST_NUMBERBOX
#define EDIT_CLASS L"TNumberBox"
#else
#define EDIT_CLASS L"TLabeledEdit"
#endif
#include "vcl_edit_fixture.h"
__declspec(dllexport) BOOL WINAPI FixtureRegister(int impostor)
{
    WNDCLASSEXW n={.cbSize=sizeof(n)};
    if(!GetClassInfoExW(NULL,L"EDIT",&n))return FALSE;
    return register_native(&n,impostor);
}
#define edit_class EDIT_CLASS
#include "vcl_edit_create.h"
