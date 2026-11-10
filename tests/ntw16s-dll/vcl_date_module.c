#define UNICODE
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "isolated_process.h"
#include "../../src/lib-win32/process-hook/unwind.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"VCL_DATE_MODULE_FAIL line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
typedef BOOL (WINAPI *reg_fn)(BOOL);
typedef HWND (WINAPI *create_fn)(HWND,BOOL);
typedef BOOL (WINAPI *unreg_fn)(void);
typedef BOOL (__cdecl *window_fn)(HWND);
static HGDIOBJ tracked_region;static unsigned region_created,region_deleted;static BOOL track_active,simulate_leak,delete_result;
static HRGN (WINAPI *region_real)(int,int,int,int);static BOOL (WINAPI *delete_real)(HGDIOBJ);
static HRGN WINAPI region_observer(int l,int t,int r,int b){HRGN v=region_real(l,t,r,b);if(track_active && v){tracked_region=v;++region_created;}return v;}
static BOOL WINAPI delete_observer(HGDIOBJ object){if(track_active && object==tracked_region){++region_deleted;if(simulate_leak)return TRUE;}BOOL result=delete_real(object);if(track_active && object==tracked_region)delete_result=result;return result;}
#include "import_swap.h"
static HWND observed;
static window_fn remove_style;
static BOOL retire,destroy_item,removed,destroyed;
static unsigned drops,closes,changes;
typedef void (WINAPI *signal_fn)(void);
static signal_fn raise_print,reenter_info;
#define COLOR_TEST_EXCEPTION 0xe0425342u
#define NTW16S_TEST_EXCEPTION_CODE COLOR_TEST_EXCEPTION
#define test_unwind exception_probe_send
#include "exception_probe.h"
#undef test_unwind
static LRESULT CALLBACK parent_proc(HWND w,UINT m,WPARAM p,LPARAM l)
{
    if(m==WM_APP+42 && (HWND)l==observed){if(retire){retire=FALSE;removed=remove_style(observed);}if(destroy_item){destroy_item=FALSE;destroyed=DestroyWindow(observed);}return 0;}
    if(m==WM_NOTIFY && l && ((NMHDR*)l)->hwndFrom==observed){UINT code=((NMHDR*)l)->code;if(code==DTN_DROPDOWN)++drops;else if(code==DTN_CLOSEUP)++closes;else if(code==DTN_DATETIMECHANGE)++changes;}
    return DefWindowProcW(w,m,p,l);
}
static DWORD WINAPI watchdog(void *arg){Sleep(700);PostMessageW((HWND)arg,DTM_CLOSEMONTHCAL,0,0);return 0;}
static BOOL test_unwind(HWND w,HDC dc)
{
    raise_print();return exception_probe_send(w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT|PRF_ERASEBKGND);
}
/* Track the exact temporary region used by this DLL. A simulated failed
 * release is detected and then explicitly cleaned up in this owned process. */
static BOOL exact_region_unwind(HWND w,HDC dc,HMODULE module,BOOL negative,BOOL report)
{
    slot_swap region_slot={0},delete_slot={0};BOOL region_swapped=FALSE,delete_swapped=FALSE,call_ok=FALSE,restored=TRUE,cleaned=TRUE;
    DWORD kind=0;tracked_region=NULL;region_created=region_deleted=0;delete_result=FALSE;simulate_leak=negative;
    if(!(region_swapped=swap_import(module,"CreateRectRgn",(void*)region_observer,&region_slot)))goto done;
    region_real=(void*)region_slot.original;
    if(!(delete_swapped=swap_import(module,"DeleteObject",(void*)delete_observer,&delete_slot)))goto done;
    delete_real=(void*)delete_slot.original;track_active=TRUE;call_ok=test_unwind(w,dc);track_active=FALSE;
    GdiFlush();kind=tracked_region?GetObjectType(tracked_region):0;
done:
    track_active=FALSE;
    if(delete_swapped)restored=restore_import(&delete_slot)&&restored;
    if(region_swapped)restored=restore_import(&region_slot)&&restored;
    if(negative && kind && delete_real)cleaned=delete_real(tracked_region);
    if(report)printf("DATE_REGION negative=%u created=%u deletes=%u delete_result=%u live=%lu restored=%u cleaned=%u\n",
        negative,region_created,region_deleted,delete_result,kind,restored,cleaned);
    return call_ok && restored && cleaned && region_created==1 && region_deleted==1 &&
        (negative?(kind==OBJ_REGION && !delete_result):(!kind && delete_result));
}
#include "vcl_capture.h"
static int run(int argc,WCHAR **argv)
{
    CHECK(argc==2);HMODULE fixture=LoadLibraryW(argv[1]);CHECK(fixture);
    reg_fn reg=(void*)GetProcAddress(fixture,"FixtureRegister");create_fn create=(void*)GetProcAddress(fixture,"FixtureCreate");
    unreg_fn unreg=(void*)GetProcAddress(fixture,"FixtureUnregister");raise_print=(void*)GetProcAddress(fixture,"FixtureRaiseNextPrint");reenter_info=(void*)GetProcAddress(fixture,"FixtureReenterInfo");CHECK(reg && create && unreg && raise_print && reenter_info);
    HMODULE dll=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");CHECK(dll);
    window_fn attached=(void*)GetProcAddress(dll,"Ntw16sWindowAttached");remove_style=(void*)GetProcAddress(dll,"Ntw16sWindowRemove");CHECK(attached && remove_style);
    HWND startup=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,1,1,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(startup);CHECK(DestroyWindow(startup));
    WNDCLASSW cls={.hInstance=GetModuleHandleW(NULL),.lpfnWndProc=parent_proc,.lpszClassName=L"VclModuleDateParent"};CHECK(RegisterClassW(&cls));
    HWND root=CreateWindowW(cls.lpszClassName,L"Module Date",WS_OVERLAPPEDWINDOW,20,20,400,240,NULL,NULL,cls.hInstance,NULL);CHECK(root);
    ShowWindow(root,SW_HIDE);ShowWindow(root,SW_SHOW);CHECK(IsWindowVisible(root));
    for(unsigned mode=0;mode<3;++mode){
        CHECK(reg(mode==0));observed=create(root,mode==2);CHECK(observed && IsWindowVisible(observed));
        CHECK(RedrawWindow(observed,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));
        if(mode!=1)CHECK(!attached(observed));
        else {
            CHECK(attached(observed));SYSTEMTIME before={.wYear=2026,.wMonth=10,.wDay=27},after={0};CHECK(SendMessageW(observed,DTM_SETSYSTEMTIME,GDT_VALID,(LPARAM)&before));
            SetFocus(observed);CHECK(GetFocus()==observed);
            DATETIMEPICKERINFO info={.cbSize=sizeof(info)};RECT client;CHECK(SendMessageW(observed,DTM_GETDATETIMEPICKERINFO,0,(LPARAM)&info) && GetClientRect(observed,&client));
            CHECK(client.right>0 && client.right<=220 && client.bottom>0 && client.bottom<=64);
            int width=client.right,height=client.bottom;DWORD styled[220*64],native[220*64],clipped[220*64];RECT styled_clip,native_clip;
            CHECK(capture(observed,styled,width,height,NULL,&styled_clip));CHECK(capture(observed,clipped,width,height,&info.rcButton,NULL));
            unsigned inside=0,outside=0;for(int y=0;y<height;++y)for(int x=0;x<width;++x)if((clipped[y*width+x]&0xffffff)!=0x5a5a5a){if(x>=info.rcButton.left && x<info.rcButton.right && y>=info.rcButton.top && y<info.rcButton.bottom)++inside;else ++outside;}CHECK(inside && !outside);
            HDC screen=GetDC(observed),memory=CreateCompatibleDC(screen);HBITMAP bitmap=CreateCompatibleBitmap(screen,width,height);HGDIOBJ prior=memory && bitmap?SelectObject(memory,bitmap):NULL;CHECK(prior);
            DWORD gdi_before=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);fprintf(stderr,"DATE_STAGE unwind_begin\n");CHECK(exact_region_unwind(observed,memory,dll,FALSE,TRUE));CHECK(exact_region_unwind(observed,memory,dll,TRUE,TRUE));for(unsigned i=0;i<8;++i)CHECK(exact_region_unwind(observed,memory,dll,FALSE,FALSE));GdiFlush();DWORD warm_count=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);for(unsigned i=0;i<8;++i)CHECK(exact_region_unwind(observed,memory,dll,FALSE,FALSE));GdiFlush();DWORD end_count=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);fprintf(stderr,"DATE_GDI_REPEAT warm=%lu end=%lu\n",warm_count,end_count);CHECK(end_count<=warm_count);fprintf(stderr,"DATE_STAGE unwind_end\n");GdiFlush();DWORD gdi_after=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);fprintf(stderr,"DATE_GDI_UNWIND diagnostic_before=%lu after=%lu\n",gdi_before,gdi_after);CHECK(attached(observed));
            SelectObject(memory,prior);CHECK(DeleteObject(bitmap));CHECK(DeleteDC(memory));CHECK(ReleaseDC(observed,screen));
            fprintf(stderr,"DATE_STAGE popup_begin\n");drops=closes=changes=0;HANDLE guard=CreateThread(NULL,0,watchdog,observed,0,NULL);CHECK(guard);
            int x=(info.rcButton.left+info.rcButton.right)/2,y=(info.rcButton.top+info.rcButton.bottom)/2;SendMessageW(observed,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(x,y));SendMessageW(observed,WM_LBUTTONUP,0,MAKELPARAM(x,y));
            HWND popup=(HWND)SendMessageW(observed,DTM_GETMONTHCAL,0,0);CHECK(popup && IsWindowVisible(popup));SendMessageW(observed,DTM_CLOSEMONTHCAL,0,0);
            CHECK(WaitForSingleObject(guard,1500)==WAIT_OBJECT_0);CloseHandle(guard);CHECK(drops==1 && closes==1 && changes==0);fprintf(stderr,"DATE_STAGE popup_end\n");
            CHECK(SendMessageW(observed,DTM_GETSYSTEMTIME,0,(LPARAM)&after)==GDT_VALID && after.wYear==before.wYear && after.wMonth==before.wMonth && after.wDay==before.wDay);
            SetFocus(observed);CHECK(GetFocus()==observed);changes=0;SendMessageW(observed,WM_KEYDOWN,VK_UP,0);SendMessageW(observed,WM_KEYUP,VK_UP,0);
            CHECK(SendMessageW(observed,DTM_GETSYSTEMTIME,0,(LPARAM)&after)==GDT_VALID && (after.wMonth!=before.wMonth || after.wDay!=before.wDay || after.wYear!=before.wYear));CHECK(changes==1);
            CHECK(SendMessageW(observed,DTM_SETSYSTEMTIME,GDT_VALID,(LPARAM)&before));fprintf(stderr,"DATE_STAGE remove_begin\n");CHECK(remove_style(observed));fprintf(stderr,"DATE_STAGE removed\n");CHECK(capture(observed,native,width,height,NULL,&native_clip));CHECK(EqualRect(&styled_clip,&native_clip));fprintf(stderr,"DATE_STAGE remove_end\n");
            unsigned changed=0,beyond=0;for(int yy=0;yy<height;++yy)for(int xx=0;xx<width;++xx)if((styled[yy*width+xx]&0xffffff)!=(native[yy*width+xx]&0xffffff)){++changed;if(xx<info.rcButton.left || xx>=info.rcButton.right || yy<info.rcButton.top || yy>=info.rcButton.bottom)++beyond;}
            printf("DATE_PIXELS changed=%u outside_button=%u clip_outside=%u\n",changed,beyond,outside);CHECK(changed && !beyond);
            CHECK(DestroyWindow(observed));observed=create(root,FALSE);CHECK(observed);CHECK(RedrawWindow(observed,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));CHECK(attached(observed));
            fprintf(stderr,"DATE_STAGE reenter_begin\n");retire=TRUE;removed=FALSE;reenter_info();CHECK(capture(observed,clipped,width,height,NULL,NULL));CHECK(removed && !attached(observed));
            CHECK(DestroyWindow(observed));observed=create(root,FALSE);CHECK(observed);CHECK(RedrawWindow(observed,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW));CHECK(attached(observed));
            destroy_item=TRUE;destroyed=FALSE;reenter_info();CHECK(capture(observed,clipped,width,height,NULL,NULL));CHECK(destroyed && !IsWindow(observed));observed=NULL;fprintf(stderr,"DATE_STAGE reenter_end\n");
        }
        if(observed)CHECK(DestroyWindow(observed));
        observed=NULL;CHECK(unreg());
    }
    CHECK(DestroyWindow(root));CHECK(FreeLibrary(fixture));
    printf("VCL_DATE_MODULE_OK bits=%u fake=1 checked_rejected=1 date=1 keyboard=1 popup=1 pixels=1 clip=1 dc_state=1 unwind=1 exact_region=1 leak_negative=1 repeated_gdi=1 retire=1 destroy=1 cleanup=1\n",(unsigned)(sizeof(void*)*8));return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,run);}
