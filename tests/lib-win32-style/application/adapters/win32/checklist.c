#include "lib-win32-style/application/adapters/win32/checklist.h"
#include <commctrl.h>
#include <stdio.h>
#define CHECK(x) do { if(!(x)){fprintf(stderr,"CHECKLIST CONTRACT FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;} } while(0)
static HDC destination,source;
static const void *caller;
static unsigned paints;
static UINT flags;
static BOOL nested_result;
static BOOL paint(HDC dc,const RECT *bounds,UINT state)
{
    ++paints;flags=state;
    nested_result=style_runtime_vb_checklist(dc,bounds->left,bounds->top,11,11,source,0,0,SRCCOPY,caller,paint);
    return TRUE;
}
static BOOL identity(HWND window){return window!=NULL;}
static BOOL observe(int sx,const void *address)
{return style_runtime_vb_checklist(destination,2,2,11,11,source,sx,0,SRCCOPY,address,paint);}
int wmain(int argc,WCHAR **argv)
{
    CHECK(argc==2);
    HMODULE runtime=LoadLibraryW(argv[1]);CHECK(runtime);
    BOOL (WINAPI *reg)(void)=(void *)GetProcAddress(runtime,"ChecklistRegister");
    BOOL (WINAPI *release)(void)=(void *)GetProcAddress(runtime,"ChecklistRelease");
    void (WINAPI *arm)(UINT,HWND)=(void *)GetProcAddress(runtime,"ChecklistArm");
    CHECK(reg && release && arm && reg());caller=(const void *)(UINT_PTR)reg;
    HINSTANCE instance=GetModuleHandleW(NULL);
    HWND root=CreateWindowW(L"STATIC",L"",WS_OVERLAPPED,0,0,240,160,NULL,NULL,instance,NULL);CHECK(root);
    HWND other=CreateWindowW(L"STATIC",L"",WS_OVERLAPPED,0,0,240,160,NULL,NULL,instance,NULL);CHECK(other);
    HWND list=CreateWindowW(L"ThunderRT6ListBox",L"",WS_CHILD|LBS_OWNERDRAWFIXED|LBS_HASSTRINGS|LBS_MULTIPLESEL,
        0,0,160,80,root,NULL,runtime,NULL);CHECK(list);
    CHECK(SendMessageW(list,LB_SETITEMHEIGHT,0,20)!=LB_ERR && SendMessageW(list,LB_ADDSTRING,0,(LPARAM)L"row")!=LB_ERR);
    destination=GetDC(list);source=CreateCompatibleDC(destination);CHECK(destination && source);
    HBITMAP bitmap=CreateCompatibleBitmap(destination,44,11);CHECK(bitmap);
    HGDIOBJ previous=SelectObject(source,bitmap);CHECK(previous);
    CHECK(GetDpiForWindow(list)==96);
    CHECK(observe(0,caller) && paints==1 && flags==0 && !nested_result);
    CHECK(observe(11,caller) && paints==2 && flags==DFCS_CHECKED && !nested_result);
    CHECK(!observe(22,caller) && paints==2);
    CHECK(!observe(0,(const void *)(UINT_PTR)wmain) && paints==2);
    CHECK(!style_runtime_vb_checklist(destination,2,2,10,11,source,0,0,SRCCOPY,caller,paint));
    EnableWindow(list,FALSE);CHECK(observe(33,caller) && paints==3 && flags==DFCS_CHECKED);
    CHECK(!observe(0,caller) && paints==3);EnableWindow(list,TRUE);
    arm(1,NULL);CHECK(!observe(0,caller) && paints==3 && !IsWindowEnabled(list));EnableWindow(list,TRUE);
    LONG_PTR style=GetWindowLongPtrW(list,GWL_STYLE);
    arm(2,NULL);CHECK(!observe(0,caller) && paints==3);SetWindowLongPtrW(list,GWL_STYLE,style);
    arm(3,other);CHECK(!observe(0,caller) && paints==3 && GetParent(list)==other);CHECK(SetParent(list,root)==other);
    CHECK(observe(0,caller) && paints==4 && !nested_result);
    CHECK(GetCurrentObject(source,OBJ_BITMAP)==bitmap && GetObjectType(bitmap)==OBJ_BITMAP);
    CHECK(ReleaseDC(list,destination));SelectObject(source,previous);CHECK(DeleteObject(bitmap) && DeleteDC(source));
    CHECK(DestroyWindow(list));
    WNDCLASSW native;CHECK(GetClassInfoW(NULL,L"LISTBOX",&native));
    native.hInstance=instance;native.lpszClassName=L"CommonChecklist";native.style&=~CS_GLOBALCLASS;CHECK(RegisterClassW(&native));
    HWND control=CreateWindowW(native.lpszClassName,L"",WS_CHILD,0,0,160,80,root,NULL,instance,NULL);CHECK(control);
    HDC dc=GetDC(control);CHECK(dc);RECT item={0,0,100,30},bounds={1,2,14,15};
    CHECK(style_runtime_checklist_target(dc,dc,control,root,&item,&bounds,(const void *)(UINT_PTR)wmain,identity,16));
    CHECK(!style_runtime_checklist_target(dc,dc,control,root,&item,&bounds,(const void *)(UINT_PTR)wmain,identity,12));
    CHECK(!style_runtime_checklist_target(dc,dc,control,other,&item,&bounds,(const void *)(UINT_PTR)wmain,identity,16));
    CHECK(!style_runtime_checklist_target(dc,dc,control,root,&item,&bounds,(const void *)(UINT_PTR)DefWindowProcW,identity,16));
    style_instance owner={0};style_control_runtime state={0};state.report.window=control;state.report.parent=root;
    const style_runtime_observation observation={.instance=&owner,.runtime=&state,.window=control};
    style_runtime_checklists manager={0};style_runtime_checklist_scope outer,inner;
    DRAWITEMSTRUCT draw={.CtlType=ODT_LISTBOX,.hwndItem=control,.hDC=dc,.itemID=0,.rcItem=item};
    CHECK(!style_runtime_checklist_begin(&manager,&outer,&observation,root,WM_PAINT,(LPARAM)&draw,identity) && !manager.current);
    CHECK(!style_runtime_checklist_begin(&manager,&outer,&observation,other,WM_DRAWITEM,(LPARAM)&draw,identity) && !manager.current);
    draw.itemID=(UINT)-1;
    CHECK(!style_runtime_checklist_begin(&manager,&outer,&observation,root,WM_DRAWITEM,(LPARAM)&draw,identity) && !manager.current);
    draw.itemID=0;draw.hDC=source;
    CHECK(!style_runtime_checklist_begin(&manager,&outer,&observation,root,WM_DRAWITEM,(LPARAM)&draw,identity) && !manager.current);
    draw.hDC=dc;
    CHECK(style_runtime_checklist_begin(&manager,&outer,&observation,root,WM_DRAWITEM,(LPARAM)&draw,identity) && manager.current==&outer);
    CHECK(style_runtime_checklist_draw_target(&manager,dc,&bounds,(const void *)(UINT_PTR)wmain,identity,16));
    CHECK(style_runtime_checklist_begin(&manager,&inner,&observation,root,WM_DRAWITEM,(LPARAM)&draw,identity) && manager.current==&inner && inner.previous==&outer);
    owner.detached=TRUE;
    CHECK(!style_runtime_checklist_draw_target(&manager,dc,&bounds,(const void *)(UINT_PTR)wmain,identity,16));
    style_runtime_checklist_scope rejected;
    CHECK(!style_runtime_checklist_begin(&manager,&rejected,&observation,root,WM_DRAWITEM,(LPARAM)&draw,identity) && manager.current==&inner);
    owner.detached=FALSE;state.report.parent=other;
    CHECK(!style_runtime_checklist_draw_target(&manager,dc,&bounds,(const void *)(UINT_PTR)wmain,identity,16));
    state.report.parent=root;style_runtime_checklist_end(&manager,&inner);CHECK(manager.current==&outer);
    CHECK(style_runtime_checklist_draw_target(&manager,dc,&bounds,(const void *)(UINT_PTR)wmain,identity,16));
    style_runtime_checklist_end(&manager,&outer);CHECK(!manager.current);
    CHECK(!style_runtime_checklist_draw_target(&manager,dc,&bounds,(const void *)(UINT_PTR)wmain,identity,16));
    CHECK(SetViewportOrgEx(dc,1,0,NULL));
    CHECK(!style_runtime_checklist_target(dc,dc,control,root,&item,&bounds,(const void *)(UINT_PTR)wmain,identity,16));
    CHECK(ReleaseDC(control,dc) && DestroyWindow(control) && UnregisterClassW(native.lpszClassName,instance));
    CHECK(DestroyWindow(root) && DestroyWindow(other) && release() && FreeLibrary(runtime));
    puts("CHECKLIST CONTRACT PASS protocol=1 nested=1 disable=1 style=1 reparent=1 borrowed=1 caller=1 region_policy=1 cleanup=1");return 0;
}
