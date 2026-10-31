#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <commctrl.h>
#include <wchar.h>
#define QUERY (WM_APP+81)
#define CONTENT (WM_APP+82)
static HWND controls[5];
typedef BOOL (__cdecl *attached_fn)(HWND);
static LRESULT CALLBACK proc(HWND w,UINT m,WPARAM a,LPARAM b)
{
 if(m==QUERY){HMODULE lib=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");attached_fn attached=lib?(attached_fn)(void *)GetProcAddress(lib,"Ntw16sWindowAttached"):NULL;unsigned mask=0;for(unsigned i=0;i<5;++i)if(attached && attached(controls[i]))mask|=1u<<i;return mask;}
 if(m==CONTENT){WCHAR text[32];return SendMessageW(controls[1],TCM_GETITEMCOUNT,0,0)==1 && SendMessageW(controls[2],HDM_GETITEMCOUNT,0,0)==1 && SendMessageW(controls[3],PBM_GETPOS,0,0)==42 && GetWindowTextW(controls[4],text,ARRAYSIZE(text)) && !lstrcmpW(text,L"Action");}
 if(m==WM_CLOSE){DestroyWindow(w);return 0;}if(m==WM_DESTROY){PostQuitMessage(0);return 0;}return DefWindowProcW(w,m,a,b);
}
int wmain(int argc,WCHAR **argv)
{
 if(argc!=2)return 2;
 INITCOMMONCONTROLSEX cc={sizeof(cc),ICC_TAB_CLASSES|ICC_PROGRESS_CLASS|ICC_LISTVIEW_CLASSES|ICC_LINK_CLASS};
 if(!InitCommonControlsEx(&cc))return 3;
 WNDCLASSW c={0};c.hInstance=GetModuleHandleW(NULL);c.lpfnWndProc=proc;c.lpszClassName=L"Win16GlobalCommon";if(!RegisterClassW(&c))return 4;
 HWND w=CreateWindowW(c.lpszClassName,argv[1],WS_OVERLAPPEDWINDOW,40,40,440,340,NULL,NULL,c.hInstance,NULL);if(!w)return 5;
 const WCHAR *names[]={L"SysLink",L"SysTabControl32",L"SysHeader32",L"msctls_progress32",L"BUTTON"};
 for(unsigned k=0;k<5;++k){controls[k]=CreateWindowW(names[k],k==0?L"<a href=\"https://example.com\">Link</a>":k==4?L"Action":L"",WS_CHILD|WS_VISIBLE,10,10+50*(int)k,300,35,w,(HMENU)(UINT_PTR)(10+k),c.hInstance,NULL);if(!controls[k])return 6;}
 TCITEMW tab={.mask=TCIF_TEXT,.pszText=L"One"};HDITEMW header={.mask=HDI_TEXT|HDI_WIDTH,.cxy=80,.pszText=L"Column"};
 if(SendMessageW(controls[1],TCM_INSERTITEMW,0,(LPARAM)&tab)<0 || SendMessageW(controls[2],HDM_INSERTITEMW,0,(LPARAM)&header)<0)return 7;
 SendMessageW(controls[3],PBM_SETPOS,42,0);ShowWindow(w,SW_SHOW);
 MSG msg;while(GetMessageW(&msg,NULL,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}return 0;
}
