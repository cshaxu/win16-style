#define UNICODE
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "isolated_process.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"VCL_TOOLBAR_FAIL line=%d error=%lu expr=%s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
typedef BOOL (WINAPI *reg_fn)(BOOL);typedef BOOL (WINAPI *unreg_fn)(void);typedef HWND (WINAPI *group_fn)(HWND);typedef HWND (WINAPI *create_fn)(HWND,int);typedef void (WINAPI *skip_fn)(BOOL);typedef unsigned (WINAPI *count_fn)(void);typedef BOOL (__cdecl *window_fn)(HWND);
static BOOL capture(HWND w,DWORD *out,int width,int height){HDC dc=CreateCompatibleDC(NULL);BITMAPINFO bi={0};void *bits=NULL;bi.bmiHeader.biSize=sizeof(bi.bmiHeader);bi.bmiHeader.biWidth=width;bi.bmiHeader.biHeight=-height;bi.bmiHeader.biPlanes=1;bi.bmiHeader.biBitCount=32;HBITMAP b=CreateDIBSection(dc,&bi,DIB_RGB_COLORS,&bits,NULL,0);HGDIOBJ old=b?SelectObject(dc,b):NULL;if(!old)return FALSE;memset(bits,0x55,(size_t)width*height*4);SendMessageW(w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT|PRF_ERASEBKGND);GdiFlush();memcpy(out,bits,(size_t)width*height*4);SelectObject(dc,old);DeleteObject(b);DeleteDC(dc);return TRUE;}
static int run(int argc,WCHAR **argv){CHECK(argc==2);HMODULE dll=GetModuleHandleW(sizeof(void*)==8?L"ntwshk64.dll":L"ntwshk32.dll"),f=LoadLibraryW(argv[1]);CHECK(dll&&f);window_fn attached=(void*)GetProcAddress(dll,"Ntw16sWindowAttached"),remove=(void*)GetProcAddress(dll,"Ntw16sWindowRemove");reg_fn reg=(void*)GetProcAddress(f,"FixtureRegister");unreg_fn unreg=(void*)GetProcAddress(f,"FixtureUnregister");group_fn group=(void*)GetProcAddress(f,"FixtureGroup");create_fn create=(void*)GetProcAddress(f,"FixtureCreate");skip_fn skip=(void*)GetProcAddress(f,"FixtureSkip");count_fn pre=(void*)GetProcAddress(f,"FixturePrepaint"),commands=(void*)GetProcAddress(f,"FixtureCommands");CHECK(attached&&remove&&reg&&unreg&&group&&create&&skip&&pre&&commands);HWND root=CreateWindowW(L"STATIC",L"",WS_OVERLAPPEDWINDOW,10,10,400,200,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(root);ShowWindow(root,SW_SHOWNA);
for(int mode=0;mode<8;mode++){CHECK(reg(mode==0));HWND g=group(root),t=create(g,mode==0?0:mode-1);CHECK(g&&t);skip(mode==4);CHECK(RedrawWindow(t,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));if(!mode)CHECK(!attached(t));else{CHECK(attached(t));RECT r;CHECK(GetClientRect(t,&r));int w=r.right,h=r.bottom;CHECK(w>0&&w<=400&&h>0&&h<=120);DWORD styled[400*120],native[400*120];unsigned n=pre();CHECK(capture(t,styled,w,h));CHECK(pre()>n);unsigned before=commands();RECT item;CHECK(SendMessageW(t,TB_GETITEMRECT,0,(LPARAM)&item));fprintf(stderr,"TOOLBAR_GEOMETRY mode=%d client=%d,%d item=%ld,%ld,%ld,%ld\n",mode,w,h,item.left,item.top,item.right,item.bottom);int x=(item.left+item.right)/2,y=(item.top+item.bottom)/2;SendMessageW(t,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(x,y));SendMessageW(t,WM_LBUTTONUP,0,MAKELPARAM(x,y));if(mode!=5)CHECK(commands()==before+1);if(mode==1){CHECK(SetParent(t,root)==g);CHECK(RedrawWindow(t,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));CHECK(!attached(t));}else CHECK(remove(t)&&!attached(t));CHECK(capture(t,native,w,h));unsigned diff=0;for(int i=0;i<w*h;i++)if((styled[i]&0xffffff)!=(native[i]&0xffffff))diff++;fprintf(stderr,"TOOLBAR_MODEL mode=%d pixels=%u\n",mode,diff);if(mode==1 || mode==6)CHECK(diff>0);else CHECK(diff==0);}if(mode==1)CHECK(DestroyWindow(t));CHECK(DestroyWindow(g));CHECK(unreg());}CHECK(DestroyWindow(root));CHECK(FreeLibrary(f));printf("VCL_TOOLBAR_MODULE_OK bits=%u fake=1 zero=1 duplicate=1 image=1 click=1 callback=1 app_skip=1 reparent=1 clipped=1 cap64=1 cap65=1 removal=1 cleanup=1\n",(unsigned)sizeof(void*)*8);return 0;}
static int theme_edge(PCWSTR fixture,PCWSTR color)
{
    WCHAR *end;
    unsigned long expected=wcstoul(color,&end,16);
    CHECK(end!=color && !*end && expected<=0xffffff);
    HMODULE module=LoadLibraryW(fixture);
    CHECK(module);
    reg_fn reg=(void *)GetProcAddress(module,"FixtureRegister");
    unreg_fn unreg=(void *)GetProcAddress(module,"FixtureUnregister");
    group_fn group=(void *)GetProcAddress(module,"FixtureGroup");
    create_fn create=(void *)GetProcAddress(module,"FixtureCreate");
    count_fn commands=(void *)GetProcAddress(module,"FixtureCommands");
    CHECK(reg && unreg && group && create && commands && reg(FALSE));
    HWND root=CreateWindowExW(WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE,L"STATIC",L"",WS_OVERLAPPEDWINDOW|WS_VISIBLE,
        -10000,-10000,400,200,NULL,NULL,GetModuleHandleW(NULL),NULL);
    CHECK(root);
    HWND parent=group(root),bar=parent?create(parent,0):NULL;
    RECT client,item;
    CHECK(bar && RedrawWindow(bar,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW) &&
        GetClientRect(bar,&client) && SendMessageW(bar,TB_GETITEMRECT,0,(LPARAM)&item));
    DWORD pixels[400*120];
    CHECK(client.right>0 && client.right<=400 && client.bottom>0 && client.bottom<=120 &&
        item.left>=0 && item.left+3<client.right && item.top>=0 && item.top<client.bottom);
    CHECK(capture(bar,pixels,client.right,client.bottom));
    DWORD dib=pixels[item.top*client.right+item.left+3];
    COLORREF edge=RGB((dib>>16)&0xff,(dib>>8)&0xff,dib&0xff);
    printf("THEME_VCL_TOOLBAR_EDGE pixel=%06lx\n",edge);
    CHECK(edge==(COLORREF)expected);
    LPARAM point=MAKELPARAM((item.left+item.right)/2,(item.top+item.bottom)/2);
    SendMessageW(bar,WM_LBUTTONDOWN,MK_LBUTTON,point);
    SendMessageW(bar,WM_LBUTTONUP,0,point);
    CHECK(commands()==1 && DestroyWindow(root) && unreg() && FreeLibrary(module));
    puts("THEME_VCL_TOOLBAR_OK command=1");
    return 0;
}

int wmain(int argc,WCHAR **argv)
{
    if(argc==4 && !wcscmp(argv[1],L"--theme-edge"))return theme_edge(argv[2],argv[3]);
    return isolated_process(argc,argv,run);
}
