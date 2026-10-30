#include <windows.h>
#include <commdlg.h>
#include <stdio.h>
static int depth,opens,checks,attached,failures;static int results[3];
typedef BOOL (__cdecl *install_fn)(HWND,const void *);
static VOID CALLBACK tick(HWND w,UINT msg,UINT_PTR id,DWORD time){(void)msg;(void)time;KillTimer(w,id);++checks;HMODULE dll=GetModuleHandleW(sizeof(void*)==4?L"ntw16s32.dll":L"ntw16s64.dll");install_fn f=dll?(install_fn)(void*)GetProcAddress(dll,"win16_style_control_install"):NULL;HWND list=GetDlgItem(w,1121);if(f&&list){SetLastError(0);if(!f(list,NULL)&&GetLastError()==ERROR_ALREADY_EXISTS)++attached;else ++failures;}else ++failures;PostMessageW(w,WM_COMMAND,MAKEWPARAM(IDCANCEL,BN_CLICKED),(LPARAM)GetDlgItem(w,IDCANCEL));}
static BOOL show(HWND owner);
static UINT_PTR CALLBACK hook(HWND w,UINT m,WPARAM p,LPARAM l){(void)p;(void)l;if(m==WM_INITDIALOG){++opens;if(depth==0){++depth;results[1]=show(w);--depth;}if(!SetTimer(w,77,80,tick))++failures;}return 0;}
static BOOL show(HWND owner){WCHAR path[MAX_PATH]=L"";OPENFILENAMEW f={0};f.lStructSize=sizeof(f);f.hwndOwner=owner;f.lpstrFile=path;f.nMaxFile=MAX_PATH;f.lpstrFilter=L"All\0*.*\0\0";f.lpfnHook=hook;f.Flags=OFN_ENABLEHOOK|OFN_NOCHANGEDIR;BOOL result=GetOpenFileNameW(&f);if(CommDlgExtendedError()!=0)++failures;return result;}
int wmain(void){results[0]=show(NULL);depth=1;results[2]=show(NULL);printf("NESTED_OLD bits=%u opens=%d checks=%d attached=%d failures=%d cancelled=%d%d%d\n",(unsigned)(sizeof(void*)*8),opens,checks,attached,failures,!results[0],!results[1],!results[2]);return opens==3&&checks==3&&attached==3&&!failures&&!results[0]&&!results[1]&&!results[2]?0:1;}
