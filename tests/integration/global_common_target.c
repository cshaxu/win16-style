#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <commctrl.h>
#include <wchar.h>
#define QUERY (WM_APP+81)
#define CONTENT (WM_APP+82)
#define CREATE_LATE (WM_APP+83)
#define LATE_QUERY (WM_APP+84)
#define LATE_CLICKED (WM_APP+85)
#define VB_HASH (WM_APP+86)
#define VB_ATTACHED (WM_APP+87)
static HWND controls[5];
static HWND late[2];
static HWND vb_form;
static DWORD (WINAPI *vb_hash)(HWND);
static unsigned clicks;
typedef BOOL (__cdecl *attached_fn)(HWND);
static LRESULT CALLBACK proc(HWND w,UINT m,WPARAM a,LPARAM b)
{
 if(m==QUERY){HMODULE lib=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");attached_fn attached=lib?(attached_fn)(void *)GetProcAddress(lib,"Ntw16sWindowAttached"):NULL;unsigned mask=0;for(unsigned i=0;i<5;++i)if(attached && attached(controls[i]))mask|=1u<<i;return mask;}
 if(m==CREATE_LATE){for(unsigned i=0;i<2;++i){late[i]=CreateWindowW(L"BUTTON",L"Late",WS_CHILD|(i?0:WS_VISIBLE),315,10+40*(int)i,80,30,w,(HMENU)(UINT_PTR)(100+i),GetModuleHandleW(NULL),NULL);if(!late[i])return FALSE;if(i)ShowWindow(late[i],SW_SHOW);}return TRUE;}
 if(m==LATE_QUERY){HMODULE lib=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");attached_fn attached=lib?(attached_fn)(void *)GetProcAddress(lib,"Ntw16sWindowAttached"):NULL;unsigned mask=0;for(unsigned i=0;i<2;++i)if(late[i]&&attached&&attached(late[i]))mask|=1u<<i;return mask;}
 if(m==LATE_CLICKED)return clicks;
 if(m==VB_HASH)return vb_form && vb_hash?vb_hash(vb_form):0;
 if(m==VB_ATTACHED){HMODULE lib=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");attached_fn attached=lib?(attached_fn)(void *)GetProcAddress(lib,"Ntw16sWindowAttached"):NULL;return vb_form && attached && attached(vb_form);}
 if(m==WM_COMMAND && (LOWORD(a)==100||LOWORD(a)==101) && HIWORD(a)==BN_CLICKED){++clicks;return 0;}
 if(m==CONTENT){WCHAR text[32];return SendMessageW(controls[1],TCM_GETITEMCOUNT,0,0)==1 && SendMessageW(controls[2],HDM_GETITEMCOUNT,0,0)==1 && SendMessageW(controls[3],PBM_GETPOS,0,0)==42 && GetWindowTextW(controls[4],text,ARRAYSIZE(text)) && !lstrcmpW(text,L"Action");}
 if(m==WM_CLOSE){DestroyWindow(w);return 0;}if(m==WM_DESTROY){PostQuitMessage(0);return 0;}return DefWindowProcW(w,m,a,b);
}
int wmain(int argc,WCHAR **argv)
{
 if(argc!=2 && argc!=3)return 2;
 INITCOMMONCONTROLSEX cc={sizeof(cc),ICC_TAB_CLASSES|ICC_PROGRESS_CLASS|ICC_LISTVIEW_CLASSES|ICC_LINK_CLASS};
 if(!InitCommonControlsEx(&cc))return 3;
 WNDCLASSW c={0};c.hInstance=GetModuleHandleW(NULL);c.lpfnWndProc=proc;c.lpszClassName=L"Win16GlobalCommon";if(!RegisterClassW(&c))return 4;
 HWND w=CreateWindowW(c.lpszClassName,argv[1],WS_OVERLAPPEDWINDOW,40,40,440,340,NULL,NULL,c.hInstance,NULL);if(!w)return 5;
 HMODULE runtime=NULL;BOOL (WINAPI *release)(void)=NULL;
 if(argc==3){runtime=LoadLibraryW(argv[2]);if(!runtime)return 8;
  BOOL (WINAPI *reg)(void)=(void *)GetProcAddress(runtime,"VbGlobalRegister");release=(void *)GetProcAddress(runtime,"VbGlobalRelease");
  vb_hash=(void *)GetProcAddress(runtime,"VbGlobalHash");if(!reg||!release||!vb_hash||!reg())return 9;
  vb_form=CreateWindowW(L"ThunderRT6FormDC",L"Global VB fixture",WS_OVERLAPPEDWINDOW,40,40,200,100,w,NULL,runtime,NULL);if(!vb_form)return 10;}
 const WCHAR *names[]={L"SysLink",L"SysTabControl32",L"SysHeader32",L"msctls_progress32",L"BUTTON"};
 for(unsigned k=0;k<5;++k){controls[k]=CreateWindowW(names[k],k==0?L"<a href=\"https://example.com\">Link</a>":k==4?L"Action":L"",WS_CHILD|WS_VISIBLE,10,10+50*(int)k,300,35,w,(HMENU)(UINT_PTR)(10+k),c.hInstance,NULL);if(!controls[k])return 6;}
 TCITEMW tab={.mask=TCIF_TEXT,.pszText=L"One"};HDITEMW header={.mask=HDI_TEXT|HDI_WIDTH,.cxy=80,.pszText=L"Column"};
 if(SendMessageW(controls[1],TCM_INSERTITEMW,0,(LPARAM)&tab)<0 || SendMessageW(controls[2],HDM_INSERTITEMW,0,(LPARAM)&header)<0)return 7;
 SendMessageW(controls[3],PBM_SETPOS,42,0);ShowWindow(w,SW_SHOW);if(vb_form)ShowWindow(vb_form,SW_SHOW);
 MSG msg;while(GetMessageW(&msg,NULL,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}
 if(vb_form && IsWindow(vb_form))DestroyWindow(vb_form);
 if(release)release();
 if(runtime)FreeLibrary(runtime);
 return 0;
}
