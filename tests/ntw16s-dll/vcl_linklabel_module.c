#define UNICODE
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <wchar.h>
#include "isolated_process.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"VCL_LINK_FAIL line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
typedef BOOL (WINAPI *reg_fn)(int);
typedef void (WINAPI *foreign_set_fn)(void *);
typedef HWND (WINAPI *create_fn)(HWND,BOOL);
typedef BOOL (WINAPI *unreg_fn)(void);
typedef void (WINAPI *set_mode_fn)(int);
typedef BOOL (__cdecl *window_fn)(HWND);
__attribute__((noinline)) static LRESULT foreign_paint(WNDPROC proc,HWND w,UINT m,WPARAM p,LPARAM l)
{volatile LRESULT v=CallWindowProcW(proc,w,m,p,l);return v;}
static unsigned clicks,returns;static window_fn remove_style;static HWND observed;static BOOL remove_on_click,removed;
static LRESULT CALLBACK parent_proc(HWND w,UINT m,WPARAM p,LPARAM l)
{
 if(m==WM_NOTIFY){NMHDR *header=(NMHDR*)l;if(header->code==NM_CLICK){++clicks;if(remove_on_click && header->hwndFrom==observed){remove_on_click=FALSE;removed=remove_style(observed);}}if(header->code==NM_RETURN)++returns;}
 return DefWindowProcW(w,m,p,l);
}
static int run(int argc,WCHAR **argv)
{
 CHECK(argc==2);HMODULE fixture=LoadLibraryW(argv[1]);CHECK(fixture);
 reg_fn reg=(void*)GetProcAddress(fixture,"FixtureRegister");create_fn create=(void*)GetProcAddress(fixture,"FixtureCreate");
 unreg_fn unreg=(void*)GetProcAddress(fixture,"FixtureUnregister");foreign_set_fn set_foreign=(void*)GetProcAddress(fixture,"FixtureSetForeign");set_mode_fn set_mode=(void*)GetProcAddress(fixture,"FixtureSetMode");CHECK(reg&&create&&unreg&&set_foreign&&set_mode);set_foreign((void*)foreign_paint);
 HMODULE dll=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");CHECK(dll);
 window_fn attached=(void*)GetProcAddress(dll,"Ntw16sWindowAttached");remove_style=(void*)GetProcAddress(dll,"Ntw16sWindowRemove");CHECK(attached&&remove_style);
 WNDCLASSW cls={.hInstance=GetModuleHandleW(NULL),.lpfnWndProc=parent_proc,.lpszClassName=L"VclLinkParent"};CHECK(RegisterClassW(&cls));
 HWND root=CreateWindowW(cls.lpszClassName,L"Link model",WS_OVERLAPPEDWINDOW,20,20,400,200,NULL,NULL,cls.hInstance,NULL);CHECK(root);ShowWindow(root,SW_HIDE);ShowWindow(root,SW_SHOW);CHECK(IsWindowVisible(root));
 for(int mode=0;mode<3;++mode){
  CHECK(reg(mode==0?1:mode==1?2:0));HWND link=create(root,FALSE);CHECK(link&&IsWindowVisible(link));
  LITEM item={.mask=LIF_ITEMINDEX|LIF_ITEMID|LIF_STATE,.iLink=0,.stateMask=LIS_ENABLED|LIS_FOCUSED|LIS_VISITED};
  CHECK(SendMessageW(link,LM_GETITEM,0,(LPARAM)&item)&&!wcscmp(item.szID,L"probe"));
  CHECK(RedrawWindow(link,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW));
  if(mode<2)CHECK(!attached(link));
  else{
   CHECK(attached(link));LOGFONTW lf={0};HFONT font=(HFONT)SendMessageW(link,WM_GETFONT,0,0);CHECK(font&&GetObjectW(font,sizeof(lf),&lf)&&wcsstr(lf.lfFaceName,L"Win16 System"));
   item.mask=LIF_ITEMINDEX|LIF_STATE;item.stateMask=LIS_VISITED;item.state=LIS_VISITED;CHECK(SendMessageW(link,LM_SETITEM,0,(LPARAM)&item));item.state=0;CHECK(SendMessageW(link,LM_GETITEM,0,(LPARAM)&item)&&(item.state&LIS_VISITED));
   SetFocus(link);SendMessageW(link,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(5,8));SendMessageW(link,WM_LBUTTONUP,0,MAKELPARAM(5,8));
   SendMessageW(link,WM_KEYDOWN,VK_RETURN,0);SendMessageW(link,WM_KEYUP,VK_RETURN,0);
   printf("LINK_INPUT clicks=%u returns=%u focus=%u\n",clicks,returns,GetFocus()==link);
   CHECK(clicks==1 && returns==1);
   item.stateMask=LIS_ENABLED;item.state=0;CHECK(SendMessageW(link,LM_SETITEM,0,(LPARAM)&item));
   SendMessageW(link,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(5,8));SendMessageW(link,WM_LBUTTONUP,0,MAKELPARAM(5,8));SendMessageW(link,WM_KEYDOWN,VK_RETURN,0);
   CHECK(clicks==1 && returns==1);item.state=LIS_ENABLED;CHECK(SendMessageW(link,LM_SETITEM,0,(LPARAM)&item));
   CHECK(remove_style(link)&&!attached(link));CHECK((HFONT)SendMessageW(link,WM_GETFONT,0,0)==(HFONT)GetStockObject(DEFAULT_GUI_FONT));item.stateMask=LIS_VISITED;CHECK(SendMessageW(link,LM_GETITEM,0,(LPARAM)&item)&&(item.state&LIS_VISITED));
  }
  CHECK(DestroyWindow(link));CHECK(unreg());
 }
 CHECK(reg(0));HWND link=create(root,FALSE);CHECK(link);
 CHECK(RedrawWindow(link,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));CHECK(attached(link));
 observed=link;removed=FALSE;remove_on_click=TRUE;
 SendMessageW(link,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(5,8));SendMessageW(link,WM_LBUTTONUP,0,MAKELPARAM(5,8));
 CHECK(removed&&!attached(link));CHECK(DestroyWindow(link));CHECK(unreg());observed=NULL;
 CHECK(reg(0));HWND delayed=create(root,TRUE);CHECK(delayed);
 CHECK(RedrawWindow(delayed,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));CHECK(!attached(delayed));
 CHECK(SendMessageW(delayed,WM_SETTEXT,0,(LPARAM)L"<a id=\"probe\">Open</a>"));
 set_mode(2);CHECK(RedrawWindow(delayed,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));CHECK(!attached(delayed));
 set_mode(0);CHECK(RedrawWindow(delayed,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));CHECK(attached(delayed));
 CHECK(DestroyWindow(delayed));CHECK(unreg());
 CHECK(DestroyWindow(root));CHECK(UnregisterClassW(cls.lpszClassName,cls.hInstance));CHECK(FreeLibrary(fixture));
 printf("VCL_LINK_OK bits=%u fake=1 foreign=1 font=1 visited=1 mouse=1 keyboard=1 disabled=1 restore=1 reentrant=1 delayed=1 fresh_witness=1 cleanup=1\n",(unsigned)(sizeof(void*)*8));return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,run);}
