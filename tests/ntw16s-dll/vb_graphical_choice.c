#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "isolated_process.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"VB_GRAPHICAL_CHOICE_FAIL line=%d error=%lu %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static LRESULT CALLBACK parent(HWND w,UINT m,WPARAM a,LPARAM b)
{return DefWindowProcW(w,m,a,b);}
static DWORD image(HWND w)
{
    RECT r;BITMAPINFO info={0};DWORD *bits=NULL;CHECK(GetClientRect(w,&r));
    HDC screen=GetDC(w),dc=CreateCompatibleDC(screen);info.bmiHeader=(BITMAPINFOHEADER){.biSize=sizeof(info.bmiHeader),.biWidth=r.right,.biHeight=-r.bottom,.biPlanes=1,.biBitCount=32};
    HBITMAP bitmap=CreateDIBSection(screen,&info,DIB_RGB_COLORS,(void **)&bits,NULL,0);CHECK(screen&&dc&&bitmap);
    HGDIOBJ old=SelectObject(dc,bitmap);SendMessageW(w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);GdiFlush();DWORD hash=2166136261u;
    for(int i=0;i<r.right*r.bottom;++i){hash^=bits[i]&0xffffff;hash*=16777619u;}
    CHECK(SelectObject(dc,old)&&DeleteObject(bitmap)&&DeleteDC(dc)&&ReleaseDC(w,screen));return hash;
}
static int worker(int argc,WCHAR **argv)
{
    CHECK(argc==3);HMODULE runtime=LoadLibraryW(argv[2]);CHECK(runtime);
    BOOL (WINAPI *register_runtime)(void)=(void *)GetProcAddress(runtime,"VbRegister");void (WINAPI *release_runtime)(void)=(void *)GetProcAddress(runtime,"VbRelease");CHECK(register_runtime&&release_runtime&&register_runtime());
    HMODULE style=GetModuleHandleW(sizeof(void *)==8?L"ntw16s64.dll":L"ntw16s32.dll");BOOL (__cdecl *attached)(HWND)=style?(void *)GetProcAddress(style,"Ntw16sWindowAttached"):NULL;BOOL (__cdecl *remove)(HWND)=style?(void *)GetProcAddress(style,"Ntw16sWindowRemove"):NULL;
    WNDCLASSW cls={.hInstance=GetModuleHandleW(NULL),.lpfnWndProc=parent,.lpszClassName=L"VBGraphicalChoiceParent"};CHECK(RegisterClassW(&cls));
    HWND root=CreateWindowW(cls.lpszClassName,L"choice",WS_POPUP,0,0,260,100,NULL,NULL,cls.hInstance,NULL);CHECK(root);
    HWND choice=CreateWindowW(L"ThunderRT6CheckBox",L"Graphical",WS_CHILD|WS_VISIBLE|BS_AUTO3STATE|BS_PUSHLIKE,0,0,150,28,root,NULL,runtime,NULL);CHECK(choice);ShowWindow(root,SW_SHOWNOACTIVATE);
    HWND option_a=CreateWindowW(L"ThunderRT6OptionButton",L"A",WS_CHILD|WS_VISIBLE|WS_GROUP|BS_RADIOBUTTON|BS_PUSHLIKE,0,32,70,28,root,NULL,runtime,NULL);
    HWND option_b=CreateWindowW(L"ThunderRT6OptionButton",L"B",WS_CHILD|WS_VISIBLE|BS_RADIOBUTTON|BS_PUSHLIKE,75,32,70,28,root,NULL,runtime,NULL);CHECK(option_a&&option_b);
    if(style){CHECK(attached&&remove&&attached(choice));DWORD styled=image(choice);SendMessageW(choice,BM_SETCHECK,BST_CHECKED,0);CHECK(SendMessageW(choice,BM_GETCHECK,0,0)==BST_CHECKED);DWORD checked=image(choice);CHECK(styled!=checked);CHECK(remove(choice)&&!attached(choice)&&image(choice)!=checked);puts("VB_GRAPHICAL_CHOICE_OK styled=1 check_state=1 detach_native=1");}
    else {CHECK(!style);(void)image(choice);SendMessageW(choice,BM_SETCHECK,BST_CHECKED,0);CHECK(SendMessageW(choice,BM_GETCHECK,0,0)==BST_CHECKED);(void)image(choice);puts("VB_GRAPHICAL_CHOICE_OK styled=0 check_state=1 detach_native=1");}
    if(style){CHECK(attached(option_a)&&attached(option_b));SendMessageW(option_a,BM_CLICK,0,0);LRESULT a1=SendMessageW(option_a,BM_GETCHECK,0,0),b1=SendMessageW(option_b,BM_GETCHECK,0,0);DWORD first=image(option_a);SendMessageW(option_b,BM_CLICK,0,0);LRESULT a2=SendMessageW(option_a,BM_GETCHECK,0,0),b2=SendMessageW(option_b,BM_GETCHECK,0,0);DWORD second=image(option_b);printf("VB_GRAPHICAL_OPTION_TRACE a1=%lld b1=%lld a2=%lld b2=%lld first=%08lx second=%08lx\n",(long long)a1,(long long)b1,(long long)a2,(long long)b2,first,second);CHECK(a1==BST_CHECKED&&b1==BST_UNCHECKED&&b2==BST_CHECKED&&a2==BST_UNCHECKED);CHECK(remove(option_a)&&remove(option_b));}
    else {SendMessageW(option_a,BM_CLICK,0,0);CHECK(SendMessageW(option_a,BM_GETCHECK,0,0)==BST_CHECKED);SendMessageW(option_b,BM_CLICK,0,0);CHECK(SendMessageW(option_b,BM_GETCHECK,0,0)==BST_CHECKED&&SendMessageW(option_a,BM_GETCHECK,0,0)==BST_UNCHECKED);}
    puts("VB_GRAPHICAL_OPTION_OK runtime_grouping=1 native_retire=1");
    CHECK(DestroyWindow(root)&&UnregisterClassW(cls.lpszClassName,cls.hInstance));release_runtime();FreeLibrary(runtime);return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
