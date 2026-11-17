#include "lib-win32-style/application/adapters/win32/runtime_observer.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"OBSERVER FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
static style_instance instance;
static style_control_runtime runtime;
static BOOL owned=TRUE;
static BOOL current(HWND w,void *owner){return IsWindow(w) && owner==&instance && owned;}
static LRESULT CALLBACK native(HWND w,UINT m,WPARAM a,LPARAM b){return DefWindowProcW(w,m,a,b);}
static WNDPROC procedure(PCWSTR name){WNDCLASSEXW cls={.cbSize=sizeof(cls)};return GetClassInfoExW(NULL,name,&cls)?cls.lpfnWndProc:NULL;}
int main(void)
{
    INITCOMMONCONTROLSEX init={sizeof(init),ICC_WIN95_CLASSES|ICC_DATE_CLASSES|ICC_LINK_CLASS};CHECK(InitCommonControlsEx(&init));
    HMODULE module=GetModuleHandleW(NULL);
    static const WCHAR *classes[]={L"TControlBar",L"NativeObserver",L"TButton",L"TToolBar",L"TDBEdit",L"TDBComboBox",L"TScrollBox",L"TRichEdit"};
    for(unsigned i=0;i<ARRAYSIZE(classes);++i){WNDCLASSW cls={.hInstance=module,.lpfnWndProc=native,.lpszClassName=classes[i]};CHECK(RegisterClassW(&cls));}
    HWND parent=CreateWindowW(classes[0],L"",WS_POPUP,0,0,300,200,NULL,NULL,module,NULL);CHECK(parent);
    const WCHAR *native_classes[]={STATUSCLASSNAMEW,DATETIMEPICK_CLASSW,L"COMBOBOX",TRACKBAR_CLASSW,L"SCROLLBAR",L"EDIT",WC_LINK,L"COMBOBOX",WC_TABCONTROLW,WC_LISTVIEWW,WC_TREEVIEWW,TOOLBARCLASSNAMEW,L"BUTTON"};
    for(unsigned role=0;role<ARRAYSIZE(native_classes);++role){
        HWND w=CreateWindowW(role==11?L"TToolBar":role==12?L"TButton":L"NativeObserver",L"",WS_CHILD,0,0,200,100,parent,NULL,module,NULL);CHECK(w);
        runtime=(style_control_runtime){0};instance=(style_instance){0};HMODULE *witness=NULL;BOOL *attempted=NULL;
        switch(role){
        case 0:runtime.status.candidate=TRUE;witness=&runtime.status.witness_owner;attempted=&runtime.status.attempted;break;
        case 1:runtime.date.candidate=TRUE;witness=&runtime.date.owner;attempted=&runtime.date.attempted;break;
        case 2:runtime.color.candidate=TRUE;witness=&runtime.color.owner;attempted=&runtime.color.attempted;break;
        case 3:runtime.track.candidate=TRUE;witness=&runtime.track.witness;attempted=&runtime.track.attempted;break;
        case 4:runtime.scroll.candidate=TRUE;runtime.scroll.parent=parent;witness=&runtime.scroll.witness;attempted=&runtime.scroll.attempted;break;
        case 5:case 6:case 7:runtime.edit.candidate=TRUE;runtime.link=role==6;runtime.combo=role==7;witness=&runtime.edit.witness;attempted=&runtime.edit.attempted;break;
        case 8:runtime.tab.candidate=TRUE;runtime.tab.parent=parent;witness=&runtime.tab.witness;attempted=&runtime.tab.attempted;break;
        case 9:case 10:runtime.report.binding=(style_control_runtime_binding){.candidate=TRUE,.parent=parent,.owner=module};runtime.report.tree_role=role==10;witness=&runtime.report.binding.witness;attempted=&runtime.report.binding.attempted;break;
        case 11:runtime.toolbar=(style_control_runtime_binding){.candidate=TRUE,.parent=parent,.owner=module};witness=&runtime.toolbar.witness;attempted=&runtime.toolbar.attempted;break;
        case 12:runtime.group.candidate=TRUE;witness=&runtime.group.witness;attempted=&runtime.group.attempted;break;
        }
        style_runtime_observation paint={.instance=&instance,.runtime=&runtime,.owner=&instance,.window=w,.current=current};
        WNDPROC proc=procedure(native_classes[role]);CHECK(proc);UINT message=role==11?WM_PRINTCLIENT:WM_PAINT;
        style_runtime_observe_native(w,message,native,native,&paint,NULL);CHECK(!*witness);
        owned=FALSE;style_runtime_observe_native(w,message,proc,native,&paint,NULL);CHECK(!*witness);owned=TRUE;
        instance.detached=TRUE;style_runtime_observe_native(w,message,proc,native,&paint,NULL);CHECK(!*witness);instance.detached=FALSE;
        *attempted=TRUE;style_runtime_observe_native(w,message,proc,native,&paint,NULL);CHECK(!*witness);*attempted=FALSE;
        style_runtime_observe_native(w,message,proc,NULL,&paint,NULL);CHECK(!*witness);
        style_runtime_observe_native(w,message,proc,native,&paint,NULL);CHECK(*witness==module);
        paint.window=parent;*witness=NULL;style_runtime_observe_native(w,message,proc,native,&paint,NULL);CHECK(!*witness);
        CHECK(DestroyWindow(w));
    }
    for(unsigned combo=0;combo<2;++combo){
        HWND w=CreateWindowW(combo?L"TDBComboBox":L"TDBEdit",L"",WS_CHILD,0,0,200,100,parent,NULL,module,NULL);CHECK(w);
        runtime=(style_control_runtime){.combo=combo};runtime.edit.candidate=TRUE;instance=(style_instance){0};
        style_runtime_observation paint={.instance=&instance,.runtime=&runtime,.owner=&instance,.window=w,.current=current};
        WNDPROC wide=procedure(combo?L"COMBOBOX":L"EDIT");WNDCLASSEXA ansi={.cbSize=sizeof(ansi)};CHECK(wide && GetClassInfoExA(NULL,combo?"COMBOBOX":"EDIT",&ansi));
        if(combo){style_runtime_observe_native(w,WM_PAINT,wide,native,&paint,NULL);CHECK(!runtime.edit.witness);}
        style_runtime_observe_ansi(&paint,ansi.lpfnWndProc,w,WM_PRINTCLIENT,native,combo);CHECK(!runtime.edit.witness);
        runtime.edit.attempted=TRUE;style_runtime_observe_ansi(&paint,ansi.lpfnWndProc,w,WM_PAINT,native,combo);CHECK(!runtime.edit.witness);runtime.edit.attempted=FALSE;
        style_runtime_observe_ansi(&paint,ansi.lpfnWndProc,w,WM_PAINT,native,combo);CHECK(runtime.edit.witness==module);CHECK(DestroyWindow(w));
    }
    HWND w=CreateWindowW(L"NativeObserver",L"",WS_CHILD,0,0,200,100,parent,NULL,module,NULL);CHECK(w);
    runtime=(style_control_runtime){0};runtime.cool.owner=module;instance=(style_instance){0};style_runtime_observation paint={.instance=&instance,.runtime=&runtime,.owner=&instance,.window=w,.current=current};
    WNDPROC rebar=procedure(REBARCLASSNAMEW);CHECK(rebar);CHECK(!style_runtime_observe_native(w,WM_PAINT,rebar,native,NULL,&paint) && paint.native==rebar);
    WNDPROC st=procedure(L"STATIC");CHECK(st);HMODULE returned=style_runtime_observe_native(w,WM_PAINT,st,native,&paint,NULL);CHECK(returned && returned!=module);
    owned=FALSE;CHECK(!style_runtime_observe_native(w,WM_PAINT,st,native,&paint,NULL));owned=TRUE;CHECK(DestroyWindow(w));
    HWND box=CreateWindowExW(WS_EX_CLIENTEDGE,L"TScrollBox",L"",WS_CHILD,0,0,200,100,parent,NULL,module,NULL);CHECK(box);
    runtime=(style_control_runtime){0};runtime.box=(style_runtime_frame_state){.parent=parent,.owner=module};paint.window=box;
    CHECK(!style_runtime_observe_box(box,WM_PAINT,native,&paint));CHECK(style_runtime_observe_box(box,WM_NCPAINT,native,&paint));instance.detached=TRUE;CHECK(!style_runtime_observe_box(box,WM_NCPAINT,native,&paint));instance.detached=FALSE;CHECK(DestroyWindow(box));
    HMODULE rich_module=LoadLibraryW(L"riched20.dll");CHECK(rich_module);WNDCLASSEXW rich_class={.cbSize=sizeof(rich_class)};CHECK(GetClassInfoExW(rich_module,L"RichEdit20W",&rich_class));
    HWND rich=CreateWindowExW(WS_EX_CLIENTEDGE,L"TRichEdit",L"",WS_CHILD,0,0,200,100,parent,NULL,module,NULL);CHECK(rich);
    runtime=(style_control_runtime){0};runtime.rich=(style_runtime_rich_state){.candidate=TRUE,.unicode=TRUE,.parent=parent,.owner=module};paint.window=rich;
    CHECK(!style_runtime_observe_rich(rich_class.lpfnWndProc,rich,WM_PAINT,native,FALSE,&paint,MCS_MULTISELECT));CHECK(!runtime.rich.witness);
    CHECK(style_runtime_observe_rich(rich_class.lpfnWndProc,rich,WM_PAINT,native,TRUE,&paint,MCS_MULTISELECT) && runtime.rich.witness);
    CHECK(DestroyWindow(rich));CHECK(FreeLibrary(rich_module));CHECK(DestroyWindow(parent));
    for(unsigned i=0;i<ARRAYSIZE(classes);++i)CHECK(UnregisterClassW(classes[i],module));
    puts("OBSERVER PASS native_roles=13 ansi_db=1 wrong_proc=1 wrong_caller=1 detached=1 owned=1 attempted=1 scope=1 cool=1 static=1 box=1 rich=1 no_style=1 cleanup=1");return 0;
}
