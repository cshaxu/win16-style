/* Project-owned native popup model; run in an isolated desktop and process. */
#include <windows.h>
#include <stdio.h>
#include <imm.h>
#include <wchar.h>
#include "isolated_process.h"
static HMENU active;static BOOL styled, destroy_owner, nested_owner;static unsigned init_seen,timer_seen,failed;
static void check(BOOL ok,const char *what){if(!ok){fprintf(stderr,"FAIL %s error=%lu\n",what,GetLastError());++failed;}}
static BOOL item(HMENU m,UINT pos,UINT *type,ULONG_PTR *data){MENUITEMINFOW i={.cbSize=sizeof(i),.fMask=MIIM_FTYPE|MIIM_DATA};if(!GetMenuItemInfoW(m,pos,TRUE,&i))return FALSE;*type=i.fType;*data=i.dwItemData;return TRUE;}
static LRESULT CALLBACK proc(HWND w,UINT msg,WPARAM wp,LPARAM lp){
 if(msg==WM_INITMENUPOPUP && (HMENU)wp==active){UINT t;ULONG_PTR d;check(item(active,0,&t,&d) && !(t&MFT_OWNERDRAW) && d==0x1234,"app-init-original");++init_seen;}
 if(msg==WM_MEASUREITEM){MEASUREITEMSTRUCT *m=(void*)lp;if(m->itemData==0x5678){m->itemWidth=50;m->itemHeight=20;return TRUE;}}
 if(msg==WM_DRAWITEM){DRAWITEMSTRUCT *d=(void*)lp;if(d->itemData==0x5678)return TRUE;}
 return DefWindowProcW(w,msg,wp,lp);
}
static VOID CALLBACK timer(HWND w,UINT msg,UINT_PTR id,DWORD time){UINT t,u;ULONG_PTR d,e;(void)msg;(void)time;KillTimer(w,id);++timer_seen;
 check(item(active,0,&t,&d) && ((t&MFT_OWNERDRAW)!=0)==styled,"ordinary-open");
 check(item(active,1,&u,&e) && (u&MFT_OWNERDRAW) && e==0x5678,"application-owner-draw-preserved");
 if(nested_owner && id==12){HMENU outer=active;active=CreatePopupMenu();MENUITEMINFOW mi={.cbSize=sizeof(mi),.fMask=MIIM_FTYPE|MIIM_ID|MIIM_DATA|MIIM_STRING,.fType=MFT_STRING,.wID=201,.dwItemData=0x1234,.dwTypeData=L"Nested"};check(InsertMenuItemW(active,0,TRUE,&mi),"nested-insert");mi.fType=MFT_OWNERDRAW;mi.wID=202;mi.dwItemData=0x5678;mi.fMask=MIIM_FTYPE|MIIM_ID|MIIM_DATA;check(InsertMenuItemW(active,1,TRUE,&mi),"nested-owner-insert");check(SetTimer(w,13,100,timer)!=0,"nested-timer");TrackPopupMenu(active,TPM_RETURNCMD|TPM_RECURSE,90,90,0,w,NULL);check(item(active,0,&t,&d)&&!(t&MFT_OWNERDRAW)&&d==0x1234,"nested-restore");DestroyMenu(active);active=outer;check(item(active,0,&t,&d)&&((t&MFT_OWNERDRAW)!=0)==styled,"outer-survives-nested");}
 if(destroy_owner){DestroyWindow(w);} EndMenu();}

static int worker(int argc,WCHAR **argv){
 if(argc<2 || argc>3){return 2;} if(!ImmDisableIME(0)){return 3;} BOOL expected=argc==2 || wcscmp(argv[2],L"--native")!=0;
 if(argc==3 && !wcscmp(argv[2],L"--aware"))check(SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)!=NULL,"dpi-context");
 WNDCLASSW a={.lpfnWndProc=proc,.hInstance=GetModuleHandleW(NULL),.lpszClassName=L"TPUtilWindow"};
 WNDCLASSW b=a;b.lpszClassName=L"OtherUtility";check(RegisterClassW(&a)!=0,"register-a");check(RegisterClassW(&b)!=0,"register-b");
 HMODULE library=LoadLibraryW(argv[1]);check(library!=NULL,"helper-load");if(!library)return 1;
 HWND (WINAPI *wrong_owner)(HINSTANCE)=(void*)GetProcAddress(library,"wrong_owner");check(wrong_owner!=NULL,"helper-export");if(!wrong_owner)return 1;
 for(unsigned n=0;n<6;n++){
 HWND w=n==5?wrong_owner(a.hInstance):CreateWindowExW(WS_EX_TOOLWINDOW,n==1?b.lpszClassName:a.lpszClassName,L"",WS_POPUP|(n==2?WS_VISIBLE:0),40,40,100,80,NULL,NULL,a.hInstance,NULL);check(w!=NULL,"create-window");if(n==0)printf("POPUP_DPI %u\n",GetDpiForWindow(w));
 RECT before,after;GetWindowRect(w,&before);LONG_PTR style=GetWindowLongPtrW(w,GWL_STYLE),ex=GetWindowLongPtrW(w,GWL_EXSTYLE);
 active=CreatePopupMenu();MENUITEMINFOW i={.cbSize=sizeof(i),.fMask=MIIM_FTYPE|MIIM_ID|MIIM_DATA|MIIM_STRING,.fType=MFT_STRING,.wID=101,.dwItemData=0x1234,.dwTypeData=L"Ordinary"};check(InsertMenuItemW(active,0,TRUE,&i),"insert-normal");
 i.fType=MFT_OWNERDRAW;i.wID=102;i.dwItemData=0x5678;i.dwTypeData=NULL;i.fMask=MIIM_FTYPE|MIIM_ID|MIIM_DATA;check(InsertMenuItemW(active,1,TRUE,&i),"insert-owner");
 styled=expected && (n==0 || n==3 || n==4);destroy_owner=n==3;nested_owner=n==4;init_seen=timer_seen=0;check(SetTimer(w,12,50,timer)!=0,"set-timer");TrackPopupMenu(active,TPM_RETURNCMD,80,80,0,w,NULL);
 if(n==0){DWORD before_gdi=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);for(unsigned repeat=0;repeat<2;++repeat){check(SetTimer(w,12,50,timer)!=0,"repeat-timer");TrackPopupMenu(active,TPM_RETURNCMD,80,80,0,w,NULL);}GdiFlush();DWORD after_gdi=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);printf("POPUP_GDI before=%lu after=%lu\n",before_gdi,after_gdi);check(after_gdi<=before_gdi+2,"repeat-gdi");}
 UINT t,u;ULONG_PTR d,e;check(init_seen==(n==0?3u:n==4?2u:1u) && timer_seen==(n==0?3u:n==4?2u:1u),"callbacks");check(item(active,0,&t,&d) && !(t&MFT_OWNERDRAW) && d==0x1234,"restore-normal");check(item(active,1,&u,&e) && (u&MFT_OWNERDRAW) && e==0x5678,"restore-owner");
 if(n!=3){GetWindowRect(w,&after);check(EqualRect(&before,&after)&&GetWindowLongPtrW(w,GWL_STYLE)==style&&GetWindowLongPtrW(w,GWL_EXSTYLE)==ex,"window-unchanged");}else check(!IsWindow(w),"destroy-owner");
 printf("CASE %u styled=%u init=%u timer=%u errors=%u\n",n,styled,init_seen,timer_seen,failed);DestroyMenu(active);DestroyWindow(w);
 }
 FreeLibrary(library);if(!failed)puts("VCL_POPUP_SCOPE_OK callbacks=1 ownerdraw=1 nested=1 destroy=1 wrong_owner=1 restored=1");return failed?1:0;
}

int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
