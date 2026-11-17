#include "lib-win32-style/application/control/native_overlay.h"
#include <stdio.h>
#include <oleacc.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"NATIVE OVERLAY FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
static style_instance instance;
static unsigned calls,failed;
static BOOL detach_query,pressed_query,paint_message;
static HDC borrowed;
static RECT expected;
static HRGN application_clip;
static LRESULT CALLBACK query(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;(void)data;LRESULT result=DefSubclassProc(w,m,a,b);
    if(m==CB_GETDROPPEDSTATE){if(detach_query)instance.detached=TRUE;if(pressed_query)return TRUE;}
    if(m==DTM_GETDATETIMEPICKERINFO && pressed_query)((DATETIMEPICKERINFO *)b)->stateButton|=STATE_SYSTEM_PRESSED;
    return result;
}
static void paint(HWND w,const style_instance *owner,HDC dc,const RECT *dirty,const style_control_native_button *button)
{
    (void)w;++calls;
    if(owner!=&instance || !EqualRect(&button->bounds,&expected) || button->pressed!=pressed_query)failed=1;
    if(paint_message){if(!dirty || dc==borrowed)failed=1;}
    else {HRGN clip=CreateRectRgn(0,0,0,0);if(!clip || GetClipRgn(dc,clip)!=1 || !EqualRgn(clip,application_clip))failed=1;if(clip)DeleteObject(clip);if(dirty)failed=1;}
    SetTextColor(dc,RGB(100,110,120));SetLastError(0x4321);
}
int main(void)
{
    INITCOMMONCONTROLSEX init={sizeof(init),ICC_DATE_CLASSES};CHECK(InitCommonControlsEx(&init));
    HWND parent=CreateWindowW(L"STATIC",L"",WS_POPUP|WS_VISIBLE,-10000,-10000,300,150,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(parent);
    HWND combo=CreateWindowW(L"COMBOBOX",L"",WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST|CBS_OWNERDRAWFIXED|CBS_HASSTRINGS,0,0,160,100,parent,NULL,GetModuleHandleW(NULL),NULL);CHECK(combo);
    style_control_native_combo_state color={.candidate=TRUE,.installed=TRUE,.parent=parent,.owner=(HMODULE)GetClassLongPtrW(combo,GCLP_HMODULE)},date={0};
    COMBOBOXINFO info={.cbSize=sizeof(info)};CHECK(GetComboBoxInfo(combo,&info));expected=info.rcButton;
    CHECK(SetWindowSubclass(combo,query,1,0));
    borrowed=CreateCompatibleDC(NULL);CHECK(borrowed);HBITMAP bitmap=CreateBitmap(300,100,1,32,NULL);CHECK(bitmap);
    HGDIOBJ old=SelectObject(borrowed,bitmap);CHECK(old);application_clip=CreateRectRgn(4,3,130,25);CHECK(application_clip);
    HRGN native_clip=CreateRectRgn(2,1,145,28),actual=CreateRectRgn(0,0,0,0);CHECK(native_clip && actual);
    style_control_native_overlay overlay;
    for(unsigned round=0;round<2;++round){pressed_query=round!=0;
        CHECK(SelectClipRgn(borrowed,application_clip)!=ERROR);
        style_control_native_overlay_begin(combo,WM_PRINTCLIENT,(WPARAM)borrowed,&instance,&color,&date,&overlay);
        HRGN owned=overlay.clip.region;CHECK(owned && overlay.clip.kind==1);
        CHECK(SelectClipRgn(borrowed,native_clip)!=ERROR);SetTextColor(borrowed,RGB(1,2,3));SetLastError(0x9876);
        style_control_native_overlay_finish(combo,WM_PRINTCLIENT,(WPARAM)borrowed,&instance,&color,&date,&overlay,paint);
        DWORD result_error=GetLastError();GdiFlush();
        CHECK(!overlay.clip.region && overlay.clip.kind==-1 && !GetObjectType(owned));
        CHECK(result_error==0x9876 && GetTextColor(borrowed)==RGB(1,2,3));
        CHECK(GetClipRgn(borrowed,actual)==1 && EqualRgn(actual,native_clip));
    }
    CHECK(calls==2 && !failed);pressed_query=FALSE;detach_query=TRUE;
    style_control_native_overlay_begin(combo,WM_PRINTCLIENT,(WPARAM)borrowed,&instance,&color,&date,&overlay);
    style_control_native_overlay_finish(combo,WM_PRINTCLIENT,(WPARAM)borrowed,&instance,&color,&date,&overlay,paint);
    CHECK(instance.detached && calls==2 && !overlay.clip.region);detach_query=FALSE;instance.detached=FALSE;
    CHECK(ValidateRect(combo,NULL));RECT dirty={1,2,10,12};CHECK(InvalidateRect(combo,&dirty,FALSE));paint_message=TRUE;
    style_control_native_overlay_begin(combo,WM_PAINT,0,&instance,&color,&date,&overlay);CHECK(overlay.update);
    SetLastError(0x6789);style_control_native_overlay_finish(combo,WM_PAINT,0,&instance,&color,&date,&overlay,paint);
    CHECK(calls==3 && !failed && GetLastError()==0x6789);paint_message=FALSE;
    style_control_native_overlay_begin(combo,WM_PRINTCLIENT,(WPARAM)borrowed,&instance,&color,&date,&overlay);
    HRGN owned=overlay.clip.region;CHECK(owned);style_control_native_overlay_dispose(&overlay);GdiFlush();CHECK(!overlay.clip.region && !GetObjectType(owned));
    HWND picker=CreateWindowW(DATETIMEPICK_CLASSW,L"",WS_CHILD,0,35,160,24,parent,NULL,GetModuleHandleW(NULL),NULL);CHECK(picker);
    color=(style_control_native_combo_state){0};date=(style_control_native_combo_state){.installed=TRUE,.parent=parent,.owner=(HMODULE)GetClassLongPtrW(picker,GCLP_HMODULE)};
    DATETIMEPICKERINFO di={.cbSize=sizeof(di)};CHECK(SendMessageW(picker,DTM_GETDATETIMEPICKERINFO,0,(LPARAM)&di));expected=di.rcButton;
    CHECK(SetWindowSubclass(picker,query,1,0));pressed_query=TRUE;
    CHECK(SelectClipRgn(borrowed,application_clip)!=ERROR);
    style_control_native_overlay_begin(picker,WM_PRINTCLIENT,(WPARAM)borrowed,&instance,&color,&date,&overlay);
    style_control_native_overlay_finish(picker,WM_PRINTCLIENT,(WPARAM)borrowed,&instance,&color,&date,&overlay,paint);CHECK(calls==4 && !failed);
    SelectObject(borrowed,old);CHECK(DeleteObject(bitmap));CHECK(DeleteDC(borrowed));CHECK(DeleteObject(application_clip));CHECK(DeleteObject(native_clip));CHECK(DeleteObject(actual));CHECK(DestroyWindow(parent));
    puts("NATIVE OVERLAY PASS combo_date_snapshot=1 pressed=1 borrowed_dc=1 native_clip_restore=1 paint_dc=1 retired_query=1 exact_region=1 dispose=1 last_error=1");return 0;
}
