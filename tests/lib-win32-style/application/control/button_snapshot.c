#include "lib-win32-style/application/control/button_snapshot.h"
#include <stdio.h>
#include <wchar.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"BUTTON_SNAPSHOT line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
typedef struct fixture {style_instance instance;BOOL label,stop,stop_image;HIMAGELIST images;unsigned fonts,disposed;} fixture;
static void dispose(void *context){++((fixture *)context)->disposed;}
static LRESULT CALLBACK procedure(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;fixture *f=(fixture *)data;
    if(m==WM_GETFONT && f->stop)(void)style_instance_detach(&f->instance);
    if(m==BM_GETIMAGE && f->stop_image)(void)style_instance_detach(&f->instance);
    if(m==BCM_GETIMAGELIST && f->images){*(BUTTON_IMAGELIST *)b=(BUTTON_IMAGELIST){.himl=f->images,.margin={1,2,3,4},.uAlign=BUTTON_IMAGELIST_ALIGN_RIGHT};return TRUE;}
    return DefSubclassProc(w,m,a,b);
}
static style_control_button_role role(HWND w,void *context,DWORD style)
{(void)w;fixture *f=context;return (style_control_button_role){style&(f->label?SS_TYPEMASK:BS_TYPEMASK),f->label};}
static HFONT font(HWND w,void *context){++((fixture *)context)->fonts;return (HFONT)SendMessageW(w,WM_GETFONT,0,0);}
static BOOL attach(fixture *f,HWND window)
{style_instance_init(&f->instance,f,dispose);if(!style_instance_attach(&f->instance,window,procedure,NULL))return FALSE;style_instance_retain(&f->instance);return TRUE;}
int main(void)
{
    HINSTANCE module=GetModuleHandleW(NULL);HWND root=CreateWindowW(L"STATIC",L"",WS_OVERLAPPED,0,0,240,140,NULL,NULL,module,NULL);
    HWND button=CreateWindowW(L"BUTTON",L"&Pick",WS_CHILD|BS_AUTO3STATE,0,0,120,25,root,NULL,module,NULL);
    HWND label=CreateWindowW(L"STATIC",L"Label",WS_CHILD|SS_SIMPLE,0,30,120,25,root,NULL,module,NULL);
    HWND image=CreateWindowW(L"STATIC",L"",WS_CHILD|SS_ICON,0,60,32,32,root,NULL,module,NULL);CHECK(root && button && label && image);
    HWND picture=CreateWindowW(L"BUTTON",L"",WS_CHILD|BS_BITMAP,40,60,32,32,root,NULL,module,NULL);CHECK(picture);
    HFONT application=CreateFontW(-15,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,0,0,L"Arial");CHECK(application);
    SendMessageW(button,WM_SETFONT,(WPARAM)application,FALSE);SendMessageW(label,WM_SETFONT,(WPARAM)application,FALSE);
    SendMessageW(button,BM_SETCHECK,BST_INDETERMINATE,0);SendMessageW(button,BM_SETSTATE,TRUE,0);
    fixture first={0},second={.label=TRUE},third={.label=TRUE},fourth={0};CHECK(attach(&first,button) && attach(&second,label) && attach(&third,image) && attach(&fourth,picture));
    style_control_button_snapshot snapshot;const style_control_button_reader read={&first,role,font};
    CHECK(style_control_button_snapshot_read(button,&first.instance,&read,&snapshot));
    CHECK(!snapshot.role.label && snapshot.role.type==BS_AUTO3STATE && (snapshot.state&BST_PUSHED) && snapshot.check==BST_INDETERMINATE && snapshot.font==application && !wcscmp(snapshot.text,L"&Pick"));
    style_control_button_snapshot_clear(&snapshot);CHECK(!snapshot.text && !snapshot.font);
    LOGFONTW metrics;CHECK(GetObjectW(application,sizeof(metrics),&metrics));
    const style_control_button_reader static_read={&second,role,font};CHECK(style_control_button_snapshot_read(label,&second.instance,&static_read,&snapshot));
    CHECK(snapshot.role.label && snapshot.role.type==SS_SIMPLE && !snapshot.state && !snapshot.check && !wcscmp(snapshot.text,L"Label"));style_control_button_snapshot_clear(&snapshot);
    HICON icon=LoadIconW(NULL,MAKEINTRESOURCEW(32512));CHECK(icon);SendMessageW(image,STM_SETICON,(WPARAM)icon,0);
    const style_control_button_reader icon_read={&third,role,font};CHECK(style_control_button_snapshot_read(image,&third.instance,&icon_read,&snapshot));
    CHECK(snapshot.icon==icon && !snapshot.text && !third.fonts);style_control_button_snapshot_clear(&snapshot);
    CHECK((HICON)SendMessageW(image,STM_GETICON,0,0)==icon);
    HBITMAP bitmap=CreateBitmap(3,4,1,1,NULL);CHECK(bitmap);SendMessageW(picture,BM_SETIMAGE,IMAGE_BITMAP,(LPARAM)bitmap);
    style_control_button_image native_image;CHECK(style_control_button_image_read(picture,&fourth.instance,IMAGE_BITMAP,&native_image));
    CHECK(native_image.image==bitmap && native_image.size.cx==3 && native_image.size.cy==4 && !native_image.listed);
    SendMessageW(picture,BM_SETIMAGE,IMAGE_BITMAP,0);
    fourth.images=ImageList_Create(4,5,ILC_COLOR32|ILC_MASK,1,1);CHECK(fourth.images && ImageList_SetImageCount(fourth.images,1));
    CHECK(style_control_button_image_read(picture,&fourth.instance,IMAGE_BITMAP,&native_image) && native_image.listed && native_image.images.himl==fourth.images && native_image.size.cx==4 && native_image.size.cy==5 && native_image.images.margin.right==3);
    fourth.stop_image=TRUE;CHECK(!style_control_button_image_read(picture,&fourth.instance,IMAGE_BITMAP,&native_image) && fourth.instance.detached && !fourth.disposed);
    style_instance_release(&fourth.instance);CHECK(fourth.disposed==1);
    first.stop=TRUE;CHECK(!style_control_button_snapshot_read(button,&first.instance,&read,&snapshot) && first.instance.detached && !snapshot.text && !first.disposed);
    style_instance_release(&first.instance);CHECK(first.disposed==1);
    CHECK(style_instance_detach(&second.instance) && style_instance_detach(&third.instance));style_instance_release(&second.instance);style_instance_release(&third.instance);
    CHECK(second.disposed==1 && third.disposed==1 && DestroyWindow(root) && DeleteObject(application) && DeleteObject(bitmap) && ImageList_Destroy(fourth.images));
    puts("BUTTON_SNAPSHOT native state/check/text/role, borrowed font/icon and font-query retirement cleanup PASS");return 0;
}
