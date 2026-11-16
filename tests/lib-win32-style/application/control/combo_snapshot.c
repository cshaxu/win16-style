#include "lib-win32-style/application/control/combo_snapshot.h"
#include <stdio.h>
#include <wchar.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"COMBO_SNAPSHOT line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
typedef struct fixture {style_instance instance;BOOL fail,stop,pressed;unsigned fonts,disposed;} fixture;
static fixture *current;
static void dispose(void *context){++((fixture *)context)->disposed;}
static LRESULT CALLBACK procedure(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;fixture *f=(fixture *)data;
    if(m==WM_GETFONT && f->stop)(void)style_instance_detach(&f->instance);
    if(m==CB_GETDROPPEDSTATE && f->pressed)return TRUE;
    return DefSubclassProc(w,m,a,b);
}
static BOOL get_info(HWND w,PCOMBOBOXINFO info){return current->fail?FALSE:GetComboBoxInfo(w,info);}
static HFONT font(HWND w,void *context){++((fixture *)context)->fonts;return (HFONT)SendMessageW(w,WM_GETFONT,0,0);}
int main(void)
{
    HINSTANCE module=GetModuleHandleW(NULL);HWND root=CreateWindowW(L"STATIC",L"",WS_OVERLAPPED,0,0,200,160,NULL,NULL,module,NULL);CHECK(root);
    const DWORD kinds[]={CBS_DROPDOWN,CBS_DROPDOWNLIST,CBS_SIMPLE};HFONT application=(HFONT)GetStockObject(DEFAULT_GUI_FONT);
    for(unsigned i=0;i<ARRAYSIZE(kinds);++i){
        HWND window=CreateWindowW(L"COMBOBOX",L"",WS_CHILD|kinds[i],0,0,160,120,root,NULL,module,NULL);CHECK(window);
        SendMessageW(window,CB_ADDSTRING,0,(LPARAM)L"Choice");SendMessageW(window,CB_SETCURSEL,0,0);SendMessageW(window,WM_SETFONT,(WPARAM)application,FALSE);
        BOOL native_pressed=SendMessageW(window,CB_GETDROPPEDSTATE,0,0)!=0;
        fixture f={0};current=&f;style_instance_init(&f.instance,&f,dispose);CHECK(style_instance_attach(&f.instance,window,procedure,NULL));style_instance_retain(&f.instance);
        const style_control_combo_reader reader={&f,get_info,font};style_control_combo_snapshot snapshot;
        CHECK(style_control_combo_snapshot_read(window,&f.instance,&reader,&snapshot));
        printf("COMBO_STATE kind=%lu combo=%d list=%d font=%d text=%ls simple=%d editable=%d pressed=%d native_pressed=%d\n",(unsigned long)kinds[i],snapshot.info.hwndCombo==window,snapshot.info.hwndList!=NULL,snapshot.font==application,snapshot.text,snapshot.simple,snapshot.editable,snapshot.pressed,native_pressed);
        CHECK(snapshot.info.hwndCombo==window && snapshot.info.hwndList && snapshot.font==application && !wcscmp(snapshot.text,L"Choice") && snapshot.simple==(kinds[i]==CBS_SIMPLE) && snapshot.editable==(kinds[i]!=CBS_DROPDOWNLIST) && snapshot.pressed==native_pressed);
        HWND list=snapshot.info.hwndList;style_control_combo_snapshot_clear(&snapshot);CHECK(IsWindow(list) && !snapshot.text);
        f.pressed=TRUE;CHECK(style_control_combo_snapshot_read(window,&f.instance,&reader,&snapshot) && snapshot.pressed);style_control_combo_snapshot_clear(&snapshot);
        f.fail=TRUE;unsigned before=f.fonts;CHECK(!style_control_combo_snapshot_read(window,&f.instance,&reader,&snapshot) && !snapshot.text && f.fonts==before);f.fail=FALSE;
        f.stop=TRUE;CHECK(!style_control_combo_snapshot_read(window,&f.instance,&reader,&snapshot) && !snapshot.text && f.instance.detached && !f.disposed);
        style_instance_release(&f.instance);CHECK(f.disposed==1 && DestroyWindow(window));
    }
    LOGFONTW metrics;CHECK(GetObjectW(application,sizeof(metrics),&metrics) && DestroyWindow(root));
    puts("COMBO_SNAPSHOT native dropdown/list/simple state/text/children/font, failed info and font-query retirement cleanup PASS");return 0;
}
