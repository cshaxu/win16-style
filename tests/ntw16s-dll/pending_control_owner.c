#include "ntw16s-dll/attachment.h"
#include <stdio.h>
#include <wchar.h>
#include "isolated_process.h"
static BOOL (__cdecl *attached)(HWND),(__cdecl *remove_window)(HWND),(__cdecl *remove_control)(HWND);
static BOOL (__cdecl *install_control)(HWND,const void*);
#define CHECK(x) do{if(!(x)){fprintf(stderr,"PENDING_OWNER_FAIL line=%u error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static HFONT source;
static WNDPROC native;static unsigned mode;static BOOL fired,removed,independent;
static LRESULT CALLBACK wrapper(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(!fired && ((mode==0 && m==WM_GETFONT) || (mode==1 && m==WM_SETFONT)) && attached(w)){
        fired=TRUE;removed=remove_window(w);
    }
    LRESULT result=CallWindowProcW(native,w,m,a,b);
    if(m==WM_CREATE){CallWindowProcW(native,w,WM_SETFONT,(WPARAM)source,FALSE);if(mode==2)independent=install_control(w,NULL);}
    return result;
}
static int worker(int argc,WCHAR **argv)
{
    if(argc!=2)return 2;
    HMODULE dll=LoadLibraryW(argv[1]);if(!dll)return 3;
    ntw16s_creation_scope *(__cdecl *begin)(BOOL)=(void*)GetProcAddress(dll,"Ntw16sCreationBegin");
    BOOL (__cdecl *end)(ntw16s_creation_scope*,ntw16s_creation_result*)=(void*)GetProcAddress(dll,"Ntw16sCreationEnd");
    attached=(void*)GetProcAddress(dll,"Ntw16sWindowAttached");remove_window=(void*)GetProcAddress(dll,"Ntw16sWindowRemove");
    remove_control=(void*)GetProcAddress(dll,"win16_style_control_remove");install_control=(void*)GetProcAddress(dll,"win16_style_control_install");
    if(!begin || !end || !attached || !remove_window || !remove_control || !install_control)return 4;
    HINSTANCE instance=GetModuleHandleW(NULL);WNDCLASSEXW cls={.cbSize=sizeof(cls)};if(!GetClassInfoExW(NULL,L"BUTTON",&cls))return 5;
    native=cls.lpfnWndProc;cls.lpfnWndProc=wrapper;cls.lpszClassName=L"PendingReproButton";cls.hInstance=instance;cls.style&=~CS_GLOBALCLASS;if(!RegisterClassExW(&cls))return 6;
    HWND root=CreateWindowW(L"STATIC",L"owner",WS_POPUP,0,0,300,200,NULL,NULL,instance,NULL);if(!root)return 7;
    source=CreateFontW(-13,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,0,0,0,0,L"Arial");if(!source)return 10;
    for(mode=0;mode<3;++mode){
        fired=removed=independent=FALSE;ntw16s_creation_scope *scope=begin(TRUE);if(!scope)return 8;
        HWND w=CreateWindowW(cls.lpszClassName,L"owned",WS_CHILD|WS_VISIBLE,0,0,120,40,root,NULL,instance,NULL);
        ntw16s_creation_result result={0};if(!end(scope,&result) || !w)return 9;
        BOOL coordinated=attached(w);HFONT current=(HFONT)SendMessageW(w,WM_GETFONT,0,0);LOGFONTW lf;BOOL valid=GetObjectW(current,sizeof(lf),&lf)!=0;BOOL leaf_present=remove_control(w);
        printf("PENDING_OWNER_OBSERVE mode=%u valid=%u application=%u fired=%u removed=%u leaf=%u scope_error=%lu\n",mode,valid,current==source,fired,removed,leaf_present,result.failure);
        CHECK(valid && !coordinated);
        if(mode<2)CHECK(fired && removed && !leaf_present && current==source && result.failure==ERROR_CANCELLED);
        else CHECK(independent && leaf_present && result.failure==ERROR_ALREADY_EXISTS);
        printf("PENDING_FONT mode=%u valid=%u is_application=%u\n",mode,valid,current==source);
        printf("PENDING_REPRO mode=%u fired=%u remove=%u independent=%u coordinator=%u leaf_present_after_cancel=%u scope_error=%lu\n",mode,fired,removed,independent,coordinated,leaf_present,result.failure);
        DestroyWindow(w);
    }
    CHECK(DestroyWindow(root) && DeleteObject(source) && FreeLibrary(dll));printf("PENDING_OWNER_OK bits=%u cancel_getfont=1 cancel_setfont=1 independent=1 application_font_valid=1\n",(unsigned)(sizeof(void*)*8));return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
