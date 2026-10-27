/* Project-authored optional observer for original VB5/VB6 intrinsic samples.
 * Binaries/runtime paths are explicit; no copied external implementation. */
/* Optional real VB6 application observer; paths are explicit arguments.
 * Own private desktop/Job, native API observations, no existing-process access. */
#include <windows.h>
#include <tlhelp32.h>
#include <commctrl.h>
#include <stdio.h>
#include <wchar.h>
#include <stdint.h>
#include "../ntw16s-dll/isolated_process.h"
#include "../../src/ntw16s-dll/control_glyphs.h"
#include "../../src/ntw16s-dll/combo_glyph.h"
#include "../../src/ntw16s-dll/scroll_glyphs.h"
#include "vb_queue.h"
#include "vb_control_text.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"VB_SAMPLE_FAIL line=%d error=%lu %s\n",__LINE__,GetLastError(),#x);ExitProcess(1);}}while(0)
static void file_path(const WCHAR *path){
    HANDLE f=CreateFileW(path,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,0,NULL);
    CHECK(f!=INVALID_HANDLE_VALUE);
    WCHAR actual[32768];
    CHECK(GetFinalPathNameByHandleW(f,actual,32768,FILE_NAME_NORMALIZED));
    printf("VB_SAMPLE_MODULE_FILE actual=%ls\n",actual);
    CloseHandle(f);
}
static HANDLE job,ready,port;
static HWND form;
static BOOL graphical_pixel_check,close_vb5_squash;
static DWORD pid;
static HWND checks[80];
static unsigned count;
static unsigned about_closed;
static WCHAR about_capture_dir[MAX_PATH];
static uint64_t capture(HWND,const WCHAR *,BOOL);
static BOOL CALLBACK capture_print(HWND w,LPARAM unused)
{
    (void)unused;WCHAR cls[80];GetClassNameW(w,cls,80);
    if(!wcscmp(cls,L"ThunderRT5PictureBox") && !(GetWindowLongPtrW(w,GWL_STYLE)&WS_BORDER) && !(GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_CLIENTEDGE)){
        WCHAR path[MAX_PATH];CHECK(swprintf_s(path,MAX_PATH,L"%ls/about-print.bmp",about_capture_dir)>0);
        printf("VB_REAL_PRINT_IMAGE hash=%llu\n",(unsigned long long)capture(w,path,FALSE));
    }
    return TRUE;
}
static BOOL CALLBACK close_about(HWND w,LPARAM main)
{
    WCHAR cls[80],title[80];GetClassNameW(w,cls,80);GetWindowTextW(w,title,80);
    if(w!=(HWND)main && !wcscmp(cls,L"ThunderRT5Form") && !wcscmp(title,L"About...")){
        CHECK(EnumChildWindows(w,capture_print,0));
        SendMessageW(w,WM_CLOSE,0,0);if(!IsWindow(w))++about_closed;
    }
    return TRUE;
}
static HANDLE queue_mapping,queue_ready;
static vb_queue_state *queue_state;
static HMODULE queue_module;
static HHOOK queue_get,queue_sent;
static DWORD pump(HANDLE h,DWORD timeout){
    ULONGLONG end=GetTickCount64()+timeout;
    for(;;){
        ULONGLONG now=GetTickCount64();
        if(now>=end)return WAIT_TIMEOUT;
        DWORD r=MsgWaitForMultipleObjectsEx(1,&h,(DWORD)(end-now),QS_ALLINPUT,MWMO_INPUTAVAILABLE);
        if(r!=WAIT_OBJECT_0+1)return r;
        MSG m;
        while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){
            TranslateMessage(&m);
            DispatchMessageW(&m);
        }
    }
}
static void queue_prepare(void)
{
    WCHAR name[128],event_name[128];
    CHECK(swprintf_s(name,128,L"Local\\Win16VBQueue-%lu-%llu",GetCurrentProcessId(),(unsigned long long)GetTickCount64())>0);
    CHECK(swprintf_s(event_name,128,L"%ls-ready",name)>0);
    queue_mapping=CreateFileMappingW(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(*queue_state),name);
    CHECK(queue_mapping && GetLastError()!=ERROR_ALREADY_EXISTS);
    queue_state=MapViewOfFile(queue_mapping,FILE_MAP_READ|FILE_MAP_WRITE,0,0,sizeof(*queue_state));
    CHECK(queue_state);ZeroMemory(queue_state,sizeof(*queue_state));queue_state->size=sizeof(*queue_state);
    CHECK(!wcscpy_s(queue_state->event_name,128,event_name));
    queue_ready=CreateEventW(NULL,TRUE,FALSE,event_name);CHECK(queue_ready && GetLastError()!=ERROR_ALREADY_EXISTS);
    CHECK(SetEnvironmentVariableW(VB_QUEUE_ENV,name));
}
static void queue_install(const WCHAR *path)
{
    queue_state->pid=pid;queue_state->tid=GetWindowThreadProcessId(form,NULL);
    queue_module=LoadLibraryW(path);CHECK(queue_module);
    HOOKPROC get=(HOOKPROC)(UINT_PTR)GetProcAddress(queue_module,"VbQueueGet");
    HOOKPROC sent=(HOOKPROC)(UINT_PTR)GetProcAddress(queue_module,"VbQueueSent");
    CHECK(get && sent);
    queue_get=SetWindowsHookExW(WH_GETMESSAGE,get,queue_module,queue_state->tid);
    queue_sent=SetWindowsHookExW(WH_CALLWNDPROC,sent,queue_module,queue_state->tid);
    CHECK(queue_get && queue_sent);
}
static void queue_key(HWND window,WPARAM key)
{
    DWORD target_pid=0;CHECK(GetWindowThreadProcessId(window,&target_pid)==queue_state->tid && target_pid==pid);
    CHECK(IsChild(form,window) && AttachThreadInput(GetCurrentThreadId(),queue_state->tid,TRUE));
    SetActiveWindow(form);SetFocus(window);GUITHREADINFO gui={.cbSize=sizeof(gui)};
    CHECK(GetGUIThreadInfo(queue_state->tid,&gui) && (gui.hwndFocus==window || IsChild(window,gui.hwndFocus)));
    HWND destination=gui.hwndFocus;
    CHECK(ResetEvent(queue_ready));queue_state->root=GetParent(window);queue_state->window=window;
    queue_state->commands=queue_state->hscroll=queue_state->vscroll=0;
    queue_state->action=0;
    queue_state->acknowledgements=queue_state->rejected=0;
    queue_state->command_code=queue_state->scroll_code=0;
    ++queue_state->generation;MemoryBarrier();
    UINT barrier=RegisterWindowMessageW(VB_QUEUE_MESSAGE);CHECK(barrier);
    LPARAM key_data=1|((LPARAM)MapVirtualKeyW((UINT)key,MAPVK_VK_TO_VSC)<<16)|0x01000000;
    CHECK(PostMessageW(window,barrier,queue_state->generation+1,0));
    CHECK(PostMessageW(destination,WM_KEYDOWN,key,key_data) && PostMessageW(destination,WM_KEYUP,key,key_data|(LPARAM)0xc0000000u));
    CHECK(PostMessageW(window,barrier,queue_state->generation,0));
    CHECK(pump(queue_ready,3000)==WAIT_OBJECT_0);MemoryBarrier();
    CHECK(queue_state->completed==(LONG)queue_state->generation);
    CHECK(queue_state->acknowledgements==1 && queue_state->rejected==1);
    CHECK(AttachThreadInput(GetCurrentThreadId(),queue_state->tid,FALSE));
    printf("VB_SAMPLE_QUEUE_OK key=%u generation=%lu commands=%ld hscroll=%ld vscroll=%ld command_code=%lu scroll_code=%lu\n",
        (unsigned)key,queue_state->generation,queue_state->commands,queue_state->hscroll,queue_state->vscroll,queue_state->command_code,queue_state->scroll_code);
}
static void queue_survey(HWND window,LONG action)
{
    DWORD owner=0;CHECK(GetWindowThreadProcessId(window,&owner)==queue_state->tid && owner==pid);
    queue_state->window=window;queue_state->root=GetParent(window);
    queue_state->action=action?action:VB_QUEUE_SURVEY;
    ++queue_state->generation;CHECK(ResetEvent(queue_ready));MemoryBarrier();
    UINT marker=RegisterWindowMessageW(VB_QUEUE_MESSAGE);CHECK(marker);
    CHECK(PostMessageW(window,marker,queue_state->generation,0) && pump(queue_ready,3000)==WAIT_OBJECT_0);
    CHECK(queue_state->completed==(LONG)queue_state->generation);MemoryBarrier();
    WCHAR cls[100];GetClassNameW(window,cls,100);
    printf("VB_FILE_SURVEY class=%ls attached=%ld font_records=%ld system_fonts=%ld draw_items=%ld draw_type=%lu draw_dc_window=%p popup_dc=%ld system_text=%ld native_text=%ld\n",
        cls,queue_state->attached,queue_state->font_records,queue_state->system_fonts,queue_state->draw_items,
        queue_state->draw_type,(void *)queue_state->draw_dc_window,queue_state->popup_dc,queue_state->system_text,queue_state->native_text);
    if(action==VB_QUEUE_PAINT_SURVEY)printf("VB_NORMAL_PAINT class=%ls messages=%ld system_selected=%ld native_selected=%ld restored=%ld probe_error=%ld\n",cls,queue_state->paint_messages,queue_state->paint_system,queue_state->paint_native,queue_state->paint_restored,queue_state->paint_probe_error);
    if(action==VB_QUEUE_PAINT_SURVEY)CHECK(queue_state->paint_restored && !queue_state->paint_probe_error);
    if(action==VB_QUEUE_PAINT_SURVEY)printf("VB_NORMAL_BUFFER class=%ls system_selected=%ld native_selected=%ld\n",cls,queue_state->paint_system_memory,queue_state->paint_native_memory);
    if(action==VB_QUEUE_PAINT_SURVEY && queue_state->paint_system_callsite)printf("VB_PAINT_SYSTEM_CALLSITE address=%p\n",(void *)queue_state->paint_system_callsite);
    if(action==VB_QUEUE_PRINT_SAMPLE){printf("VB_REAL_PRINT_API calls=%ld api=%lu alignment=%lu restored=%ld error=%ld\n",queue_state->print_calls,queue_state->print_api,queue_state->print_alignment,queue_state->paint_restored,queue_state->paint_probe_error);CHECK(queue_state->paint_restored && !queue_state->paint_probe_error);}
    queue_state->action=0;
}
/* Only enumerate the UI thread of our own target, not arbitrary processes. */
static BOOL CALLBACK vb_disabled_game(HWND w,LPARAM data)
{
    WCHAR cls[100],title[120];GetClassNameW(w,cls,100);GetWindowTextW(w,title,120);
    if(!wcscmp(cls,L"ThunderRT6FormDC") && !wcscmp(title,L"MineSweeper") && IsWindowVisible(w)){*(HWND *)data=w;return FALSE;}
    return TRUE;
}
static void CALLBACK shown(HWINEVENTHOOK h,DWORD e,HWND w,LONG object,LONG child,DWORD thread,DWORD time){
    (void)h;
    (void)e;
    (void)thread;
    (void)time;
    WCHAR cls[80];
    if(object!=OBJID_WINDOW || child || !GetClassNameW(w,cls,80))return;
    DWORD candidate;
    GetWindowThreadProcessId(w,&candidate);
    HANDLE p=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,candidate);
    BOOL own=FALSE;
    if(p){
        IsProcessInJob(p,job,&own);
        CloseHandle(p);
    }
    if(own){
        WCHAR title[160];GetWindowTextW(w,title,160);printf("SAMPLE_SHOW class=%ls title=%ls hwnd=%p\n",cls,title,(void *)w);fflush(stdout);
        if(close_vb5_squash && !wcscmp(cls,L"#32770") && !wcscmp(title,L"Jongmin Baek")){PostMessageW(w,WM_CLOSE,0,0);return;}
        if(!wcsstr(cls,L"Form") || GetParent(w))return;
        form=w;
        pid=candidate;
        SetEvent(ready);
    }
}
static BOOL CALLBACK collect(HWND w,LPARAM ignored)
{
    (void)ignored;WCHAR cls[100],text[120];GetClassNameW(w,cls,100);CHECK(vb_control_text(w,text,120));
    HWND parent=GetParent(w);WCHAR parent_class[100];GetClassNameW(parent,parent_class,100);
    WCHAR real_class[100];CHECK(RealGetWindowClassW(w,real_class,100));
    printf("SAMPLE_REAL_CLASS hwnd=%p registered=%ls real=%ls\n",(void *)w,cls,real_class);
    printf("SAMPLE_WINDOW hwnd=%p parent=%p class=%ls parent_class=%ls owner=%p style=%08lx ex=%08lx text=%ls font=%p\n",(void *)w,(void *)parent,cls,parent_class,(void *)GetClassLongPtrW(w,GCLP_HMODULE),(DWORD)GetWindowLongPtrW(w,GWL_STYLE),(DWORD)GetWindowLongPtrW(w,GWL_EXSTYLE),text,(void *)SendMessageW(w,WM_GETFONT,0,0));
    if(wcsstr(cls,L"OptionButton") || wcsstr(cls,L"CheckBox"))printf("SAMPLE_BUTTON hwnd=%p dlg=%llx check=%lld state=%lld\n",(void *)w,(unsigned long long)SendMessageW(w,WM_GETDLGCODE,0,0),(long long)SendMessageW(w,BM_GETCHECK,0,0),(long long)SendMessageW(w,BM_GETSTATE,0,0));
    if(wcsstr(cls,L"ComboBox")) {COMBOBOXINFO i={.cbSize=sizeof(i)};BOOL ok=GetComboBoxInfo(w,&i);printf("SAMPLE_COMBO hwnd=%p count=%lld selected=%lld info=%d edit=%p list=%p error=%lu\n",(void *)w,(long long)SendMessageW(w,CB_GETCOUNT,0,0),(long long)SendMessageW(w,CB_GETCURSEL,0,0),ok,(void *)i.hwndItem,(void *)i.hwndList,GetLastError());}
    if(wcsstr(cls,L"ListBox"))printf("SAMPLE_LIST hwnd=%p count=%lld selected=%lld\n",(void *)w,(long long)SendMessageW(w,LB_GETCOUNT,0,0),(long long)SendMessageW(w,LB_GETCURSEL,0,0));
    if(wcsstr(cls,L"DriveListBox")){
        COMBOBOXINFO combo={.cbSize=sizeof(combo)};BOOL valid=GetComboBoxInfo(w,&combo);
        printf("VB_FILE_DRIVE_PROTOCOL info=%d count=%lld selected=%lld item=%p list=%p owner_draw=%u has_strings=%u\n",
            valid,(long long)SendMessageW(w,CB_GETCOUNT,0,0),(long long)SendMessageW(w,CB_GETCURSEL,0,0),
            (void *)combo.hwndItem,(void *)combo.hwndList,((DWORD)GetWindowLongPtrW(w,GWL_STYLE)&(CBS_OWNERDRAWFIXED|CBS_OWNERDRAWVARIABLE))!=0,((DWORD)GetWindowLongPtrW(w,GWL_STYLE)&CBS_HASSTRINGS)!=0);
    }
    if(count<80)checks[count++]=w;
    return TRUE;
}
static uint64_t capture(HWND w,const WCHAR *file,BOOL verify){
    RECT r;
    CHECK(GetWindowRect(w,&r));
    int cx=r.right-r.left,cy=r.bottom-r.top;
    CHECK(cx>0 && cy>0);
    BITMAPINFO info= {0};
    info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth=cx;
    info.bmiHeader.biHeight=-cy;
    info.bmiHeader.biPlanes=1;
    info.bmiHeader.biBitCount=32;
    HDC screen=GetDC(w),dc=CreateCompatibleDC(screen);
    void *data;
    HBITMAP bitmap=CreateDIBSection(screen,&info,DIB_RGB_COLORS,&data,NULL,0);
    CHECK(screen && dc && bitmap);
    HGDIOBJ old=SelectObject(dc,bitmap);
    memset(data,255,(size_t)cx*cy*4);
    CHECK(PrintWindow(w,dc,0));
    uint64_t hash=14695981039346656037ull;
    BYTE *b=data;
    for(size_t i=0; i<(size_t)cx*cy*4;
    ++i)hash=(hash^b[i])*1099511628211ull;
    WCHAR role[100];GetClassNameW(w,role,100);
    if(graphical_pixel_check && wcsstr(role,L"CommandButton") && (GetWindowLongPtrW(w,GWL_STYLE)&BS_TYPEMASK)==BS_OWNERDRAW){
        int u=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));DWORD *pixels=data;
        CHECK(cx>u && cy>u && (pixels[0]&0xffffff)==0xffffff && (pixels[u]&0xffffff)==0);
        printf("VB_GRAPHICAL_FRAME_PIXELS_OK scale=%d unobserved_print=1\n",u);
    }
    if(verify && wcsstr(role,L"ComboBox")){
        COMBOBOXINFO combo={.cbSize=sizeof(combo)};CHECK(GetComboBoxInfo(w,&combo));
        POINT origin={0,0};CHECK(ClientToScreen(w,&origin));int u=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));
        RECT arrow=combo.rcButton;InflateRect(&arrow,-u,-u);
        int left=(arrow.left+arrow.right-7*u)/2+origin.x-r.left,top=(arrow.top+arrow.bottom-9*u)/2+origin.y-r.top;
        CHECK(left>=0 && top>=0 && left+7*u<=cx && top+9*u<=cy);
        unsigned checked=0;DWORD *pixels=data;
        for(int y=0;y<9;++y)for(int x=0;x<7;++x)if(combo_arrow[y][x]=='0'){
            DWORD expected=IsWindowEnabled(w)?0:0x808080;
            CHECK((pixels[(top+y*u)*cx+left+x*u]&0xffffff)==expected);++checked;
        }
        printf("VB_SAMPLE_COMBO_GLYPH_OK black_source_pixels=%u scale=%d\n",checked,u);
    }else if(verify && wcsstr(role,L"ScrollBar")){
        BOOL vertical=(GetWindowLongPtrW(w,GWL_STYLE)&SBS_VERT)!=0;int cross=vertical?cx:cy;
        SCROLLBARINFO bar={.cbSize=sizeof(bar)};CHECK(GetScrollBarInfo(w,OBJID_CLIENT,&bar));
        const char (*glyph)[18]=(bar.rgstate[1]&STATE_SYSTEM_UNAVAILABLE)?(vertical?scroll_uparrowi:scroll_lfarrowi):(vertical?scroll_uparrow:scroll_lfarrow);
        const DWORD colors[4]={0,0xffffff,0x808080,0xc0c0c0};DWORD *pixels=data;unsigned checked=0;
        for(int y=1;y<16;++y)for(int x=1;x<16;++x){int px=MulDiv(x,cross,17),py=MulDiv(y,cross,17);
            if(px==MulDiv(x+1,cross,17) || py==MulDiv(y+1,cross,17))continue;
            CHECK(px<cx && py<cy && (pixels[py*cx+px]&0xffffff)==colors[glyph[y][x]-'0']);++checked;
        }
        CHECK(checked);printf("VB_SAMPLE_SCROLL_GLYPH_OK vertical=%d pixels=%u square=%d\n",vertical,checked,cross);
    }else if(verify){RECT client;CHECK(GetClientRect(w,&client));int dpi=(int)GetDpiForWindow(w);int top=(client.bottom-MulDiv(13,dpi,96))/2;CHECK(top>=0 && cx>=MulDiv(14,dpi,96) && cy>=top+MulDiv(13,dpi,96));
        LRESULT check=SendMessageW(w,BM_GETCHECK,0,0);CHECK(check==0 || check==1);DWORD *pixels=data;WCHAR cls[100];GetClassNameW(w,cls,100);unsigned row=wcsstr(cls,L"OptionButton")?13:0;
        for(int y=0;y<13;++y)for(int x=0;x<14;++x){DWORD expected=control_checks[row+y][x+(check?14:0)]=='0'?0:0xffffff;CHECK((pixels[(top+MulDiv(y,dpi,96))*cx+MulDiv(x,dpi,96)]&0xffffff)==expected);}
        printf("VB_SAMPLE_GLYPH_OK source_pixels=182 check=%lld\n",(long long)check);
    }
    BITMAPFILEHEADER head= {0};
    head.bfType=0x4d42;
    head.bfOffBits=sizeof(head)+sizeof(info.bmiHeader);
    head.bfSize=head.bfOffBits+(DWORD)((size_t)cx*cy*4);
    DWORD n;
    HANDLE out=CreateFileW(file,GENERIC_WRITE,0,NULL,CREATE_NEW,0,NULL);
    CHECK(out!=INVALID_HANDLE_VALUE && WriteFile(out,&head,sizeof(head),&n,NULL) && n==sizeof(head) && WriteFile(out,&info.bmiHeader,sizeof(info.bmiHeader),&n,NULL) && n==sizeof(info.bmiHeader) && WriteFile(out,data,head.bfSize-head.bfOffBits,&n,NULL) && n==head.bfSize-head.bfOffBits && CloseHandle(out));
    SelectObject(dc,old);
    DeleteObject(bitmap);
    DeleteDC(dc);
    ReleaseDC(w,screen);
    return hash;
}
static HWND picture_about;static WCHAR picture_dir[MAX_PATH];static unsigned picture_count;
static BOOL CALLBACK locate_about(HWND w,LPARAM main){WCHAR cls[80];GetClassNameW(w,cls,80);if(w!=(HWND)main && IsWindowVisible(w) && !wcscmp(cls,L"ThunderRT5Form")){picture_about=w;return FALSE;}return TRUE;}
static UINT about_menu(HMENU m){int count=GetMenuItemCount(m);for(int i=0;i<count;i++){WCHAR text[128]=L"";GetMenuStringW(m,i,text,128,MF_BYPOSITION);if(!wcscmp(text,L"&About.."))return GetMenuItemID(m,i);HMENU child=GetSubMenu(m,i);if(child){UINT id=about_menu(child);if(id)return id;}}return 0;}
static BOOL CALLBACK picture_buttons(HWND w,LPARAM unused){(void)unused;WCHAR cls[80];RECT rect;GetClassNameW(w,cls,80);GetWindowRect(w,&rect);DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);if(wcscmp(cls,L"ThunderRT5CommandButton") || (style&BS_TYPEMASK)!=BS_OWNERDRAW || rect.right-rect.left!=129 || rect.bottom-rect.top!=33)return TRUE;WCHAR path[MAX_PATH];unsigned i=picture_count++;CHECK(swprintf_s(path,MAX_PATH,L"%ls/button-%u-normal.bmp",picture_dir,i)>0);unsigned long long a=capture(w,path,FALSE);SendMessageW(w,BM_SETSTATE,TRUE,0);CHECK(SendMessageW(w,BM_GETSTATE,0,0)&BST_PUSHED);CHECK(swprintf_s(path,MAX_PATH,L"%ls/button-%u-pressed.bmp",picture_dir,i)>0);unsigned long long pressed=capture(w,path,FALSE);SendMessageW(w,BM_SETSTATE,FALSE,0);CHECK(!(SendMessageW(w,BM_GETSTATE,0,0)&BST_PUSHED));printf("VB5_PICTURE_PRESSED index=%u hash=%llu restored=1 no_click=1\n",i,pressed);EnableWindow(w,FALSE);CHECK(!IsWindowEnabled(w));CHECK(swprintf_s(path,MAX_PATH,L"%ls/button-%u-disabled.bmp",picture_dir,i)>0);unsigned long long b=capture(w,path,FALSE);EnableWindow(w,TRUE);CHECK(IsWindowEnabled(w));CHECK(swprintf_s(path,MAX_PATH,L"%ls/button-%u-restored.bmp",picture_dir,i)>0);unsigned long long c=capture(w,path,FALSE);printf("VB5_PICTURE_BUTTON index=%u style=%08lx dimensions=129x33 normal=%llu disabled=%llu restored=%llu\n",i,style,a,b,c);return TRUE;}
static HWND sorted_form,sorted_list,sorted_cancel;static HMODULE sorted_runtime;static LRESULT sorted_max=-1;
static BOOL CALLBACK sorted_dialog(HWND w,LPARAM main){WCHAR cls[80],title[100];GetClassNameW(w,cls,80);GetWindowTextW(w,title,100);if(w!=(HWND)main && IsWindowVisible(w) && !wcscmp(cls,L"ThunderRT5Form") && !wcscmp(title,L"Select Icon Device Images")){sorted_form=w;return FALSE;}return TRUE;}
static BOOL CALLBACK sorted_children(HWND w,LPARAM ignored){(void)ignored;WCHAR cls[80],title[80];GetClassNameW(w,cls,80);CHECK(vb_control_text(w,title,80));DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);if(!wcscmp(cls,L"ThunderRT5ListBox") && GetParent(w)==sorted_form && (style&(LBS_SORT|LBS_MULTIPLESEL))==(LBS_SORT|LBS_MULTIPLESEL) && (HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==sorted_runtime){LRESULT n=SendMessageW(w,LB_GETCOUNT,0,0);if(n>sorted_max){sorted_max=n;sorted_list=w;}}if(!wcscmp(cls,L"ThunderRT5CommandButton") && !wcscmp(title,L"Cancel"))sorted_cancel=w;return TRUE;}
/* Optional real VB6 UserControl sample: validate the frame pixel produced by
 * the previously captured 32-bit top-down BMP, without using screen pixels. */
static BOOL vb6_edge_black(const WCHAR *path){HANDLE f=CreateFileW(path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);if(f==INVALID_HANDLE_VALUE)return FALSE;BITMAPFILEHEADER h;DWORD n=0;BYTE pixel[4];BOOL ok=ReadFile(f,&h,sizeof(h),&n,NULL) && n==sizeof(h) && h.bfType==0x4d42 && SetFilePointer(f,(LONG)h.bfOffBits,NULL,FILE_BEGIN)!=INVALID_SET_FILE_POINTER && ReadFile(f,pixel,sizeof(pixel),&n,NULL) && n==sizeof(pixel) && !pixel[0] && !pixel[1] && !pixel[2];CloseHandle(f);return ok;}
static int worker(int argc,WCHAR **argv){
    CHECK(argc==4 || argc==5);
    if(argc==5)queue_prepare();
    close_vb5_squash=!wcscmp(argv[3],L"real-graphical-vb5-option");
    BOOL styled=wcscmp(argv[1],L"bare")!=0;
    WCHAR command[2048],path[MAX_PATH],scratch[MAX_PATH];
    CHECK(GetEnvironmentVariableW(L"WIN16STYLE_CAPTURE_DIR",scratch,MAX_PATH));
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION limit= {0};
    limit.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    job=CreateJobObjectW(NULL,NULL);
    CHECK(job && SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limit,sizeof(limit)));
    port=CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,1);
    JOBOBJECT_ASSOCIATE_COMPLETION_PORT association= {.CompletionKey=job,.CompletionPort=port};
    CHECK(port && SetInformationJobObject(job,JobObjectAssociateCompletionPortInformation,&association,sizeof(association)));
    ready=CreateEventW(NULL,TRUE,FALSE,NULL);
    HWINEVENTHOOK hook=SetWinEventHook(EVENT_OBJECT_SHOW,EVENT_OBJECT_SHOW,NULL,shown,0,0,WINEVENT_OUTOFCONTEXT|WINEVENT_SKIPOWNPROCESS);
    CHECK(ready && hook);
    STARTUPINFOW si= {.cb=sizeof(si)};
    PROCESS_INFORMATION p= {0};
    if(styled)CHECK(swprintf_s(command,2048,L"\"%ls\" --wait \"%ls\"",argv[1],argv[2])>0);
    else CHECK(swprintf_s(command,2048,L"\"%ls\"",argv[2])>0);
    CHECK(CreateProcessW(styled?argv[1]:argv[2],command,NULL,NULL,FALSE,CREATE_SUSPENDED|CREATE_NO_WINDOW,NULL,NULL,&si,&p));
    CHECK(AssignProcessToJobObject(job,p.hProcess) && ResumeThread(p.hThread)==1 && pump(ready,12000)==WAIT_OBJECT_0);
    HANDLE target=OpenProcess(PROCESS_QUERY_INFORMATION|SYNCHRONIZE,FALSE,pid);
    CHECK(target && WaitForInputIdle(target,5000)==0 && IsWindowVisible(form) && !IsIconic(form));
    if(queue_state)queue_install(argv[4]);
    DWORD size=MAX_PATH;
    CHECK(QueryFullProcessImageNameW(target,0,path,&size));
    BOOL wow=FALSE;CHECK(IsWow64Process(target,&wow) && wow);
    HANDLE process_snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);CHECK(process_snapshot!=INVALID_HANDLE_VALUE);
    PROCESSENTRY32W entry= {.dwSize=sizeof(entry)};DWORD parent=0;CHECK(Process32FirstW(process_snapshot,&entry));
    do{if(entry.th32ProcessID==pid){parent=entry.th32ParentProcessID;break;}}while(Process32NextW(process_snapshot,&entry));CloseHandle(process_snapshot);
    CHECK(parent==(styled?p.dwProcessId:GetCurrentProcessId()));
    printf("VB_SAMPLE_TARGET pid=%lu creator=%lu initial_pid=%lu bits=32 path=%ls dpi=%u\n",pid,parent,p.dwProcessId,path,GetDpiForWindow(form));
    HANDLE snap=INVALID_HANDLE_VALUE;unsigned retries=0;
    do{snap=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,pid);
        if(snap!=INVALID_HANDLE_VALUE || GetLastError()!=ERROR_BAD_LENGTH)break;
    }while(++retries<8);
    printf("VB_SAMPLE_MODULE_SNAPSHOT bad_length_retries=%u error=%lu\n",retries,snap==INVALID_HANDLE_VALUE?GetLastError():0);
    CHECK(snap!=INVALID_HANDLE_VALUE);
    MODULEENTRY32W mod= {.dwSize=sizeof(mod)};
    HMODULE runtime=NULL;
    BOOL dll=FALSE;
    CHECK(Module32FirstW(snap,&mod));
    do{
        if(!_wcsicmp(mod.szModule,L"CompressZItLib6.dll") || !_wcsicmp(mod.szModule,L"vbalHook6.dll") || !_wcsicmp(mod.szModule,L"vbalHook.dll") ||
                !_wcsicmp(mod.szModule,L"msscript.ocx") || !_wcsicmp(mod.szModule,L"vbscript.dll")){
            printf("VB_SAMPLE_PRIVATE_COM base=%p path=%ls\n",(void *)mod.hModule,mod.szExePath);file_path(mod.szExePath);
        }
        if((!_wcsicmp(mod.szModule,L"MSVBVM60.DLL") || !_wcsicmp(mod.szModule,L"MSVBVM50.DLL"))){
            runtime=mod.hModule;
            printf("VB_SAMPLE_RUNTIME base=%p path=%ls\n",(void *)runtime,mod.szExePath);
            file_path(mod.szExePath);
        }
        if(!_wcsicmp(mod.szModule,L"ntw16s32.dll")){
            dll=TRUE;
            printf("VB_SAMPLE_STYLE_MODULE base=%p path=%ls\n",(void *)mod.hModule,mod.szExePath);
            file_path(mod.szExePath);
        }
    }
    while(Module32NextW(snap,&mod));
    CloseHandle(snap);
    printf("VB_GRAPHICAL_OBSERVER_DIAGNOSTICS available=1\n");
    BOOL about_print=!wcscmp(argv[3],L"about-print");
    BOOL container_verify=!wcscmp(argv[3],L"containers-verify");
    BOOL containers=container_verify || !wcscmp(argv[3],L"containers");
    BOOL filedir_verify=!wcscmp(argv[3],L"filedir-verify");
    BOOL file_verify=filedir_verify || !wcscmp(argv[3],L"files-verify");
    BOOL graphical_verify=!wcscmp(argv[3],L"graphical-verify");
    BOOL graphical_pictures=!wcscmp(argv[3],L"graphical-pictures");
    BOOL graphical=graphical_verify || graphical_pictures || !wcscmp(argv[3],L"graphical-observe");
    BOOL real_graphical=!wcscmp(argv[3],L"real-graphical-option");
    BOOL real_checkbox=!wcscmp(argv[3],L"real-graphical-checkbox");
    BOOL real_picture=!wcscmp(argv[3],L"real-graphical-picture");
    BOOL real_vb5_check=!wcscmp(argv[3],L"real-graphical-vb5-check");
    BOOL real_vb5_custom=!wcscmp(argv[3],L"real-vb5-custom");
    BOOL real_disabled_picture=!wcscmp(argv[3],L"real-vb6-disabled-picture");
    BOOL real_vb5_picture=!wcscmp(argv[3],L"real-vb5-picture-buttons");
    BOOL real_vb5_checklist=!wcscmp(argv[3],L"real-vb5-checklist");
    BOOL real_vb5_sorted=!wcscmp(argv[3],L"real-vb5-sorted-list");
    BOOL real_vb6_sorted_combo=!wcscmp(argv[3],L"real-vb6-sorted-combo");
    BOOL usercontrol_verify=!wcscmp(argv[3],L"real-vb5-usercontrol");
    BOOL vb6_unedged=!wcscmp(argv[3],L"vb6-user-edge-late");
    BOOL mdi_verify=!wcscmp(argv[3],L"real-vb-mdi");
    BOOL inventory=vb6_unedged || usercontrol_verify || mdi_verify || real_vb6_sorted_combo || real_vb5_sorted || real_vb5_checklist || real_vb5_picture || graphical || about_print || containers || file_verify || real_graphical || real_checkbox || real_picture || real_vb5_check || real_vb5_custom || close_vb5_squash || real_disabled_picture || !wcscmp(argv[3],L"inventory");
    CHECK(runtime && dll==styled && EnumChildWindows(form,collect,0) && (inventory?count>0:count>10));
    if(vb6_unedged){
        HWND uc=NULL,edit=NULL,check=NULL;for(unsigned i=0;i<count;++i){WCHAR cls[80],title[80];GetClassNameW(checks[i],cls,80);GetWindowTextW(checks[i],title,80);if(!wcscmp(cls,L"ThunderRT6UserControlDC") && (HMODULE)GetClassLongPtrW(checks[i],GCLP_HMODULE)==runtime)uc=checks[i];if(!wcscmp(cls,L"ThunderRT6TextBox") && GetParent(checks[i])==uc)edit=checks[i];if(!wcscmp(cls,L"ThunderRT6CheckBox") && !wcscmp(title,L"A&llow Browsing"))check=checks[i];}
        CHECK(uc && edit && check && GetDpiForWindow(uc)==96 && !(GetWindowLongPtrW(uc,GWL_EXSTYLE)&WS_EX_CLIENTEDGE));CHECK(SetWindowPos(form,NULL,8,8,400,280,SWP_NOZORDER|SWP_NOACTIVATE));CHECK(SetWindowPos(uc,NULL,5,5,220,180,SWP_NOZORDER|SWP_NOACTIVATE));RECT outer;CHECK(GetWindowRect(uc,&outer));printf("VB6_UNEDGED_FIT rect=%ld,%ld,%ld,%ld screen=%d,%d\n",outer.left,outer.top,outer.right,outer.bottom,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN));CHECK(outer.left>=0 && outer.top>=0 && outer.right<=GetSystemMetrics(SM_CXSCREEN) && outer.bottom<=GetSystemMetrics(SM_CYSCREEN));RECT cr;CHECK(GetClientRect(uc,&cr));HFONT f=(HFONT)SendMessageW(uc,WM_GETFONT,0,0);WCHAR text_before[256],text_after[256];CHECK(vb_control_text(edit,text_before,256));
        CHECK(swprintf_s(path,MAX_PATH,L"%ls/unframed.bmp",scratch)>0);printf("VB6_UNEDGED_INITIAL hash=%llu\n",(unsigned long long)capture(uc,path,FALSE));LONG_PTR ex=GetWindowLongPtrW(uc,GWL_EXSTYLE);SetLastError(0);LONG_PTR prev=SetWindowLongPtrW(uc,GWL_EXSTYLE,ex|WS_EX_CLIENTEDGE);CHECK(prev || !GetLastError());CHECK(SetWindowPos(uc,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED));RedrawWindow(uc,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW);CHECK(swprintf_s(path,MAX_PATH,L"%ls/added.bmp",scratch)>0);printf("VB6_UNEDGED_ADDED hash=%llu\n",(unsigned long long)capture(uc,path,FALSE));if(styled)CHECK(vb6_edge_black(path));SetLastError(0);prev=SetWindowLongPtrW(uc,GWL_EXSTYLE,ex);CHECK(prev || !GetLastError());CHECK(SetWindowPos(uc,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED));RECT after;CHECK(GetClientRect(uc,&after) && EqualRect(&cr,&after) && (HFONT)SendMessageW(uc,WM_GETFONT,0,0)==f && vb_control_text(edit,text_after,256) && !wcscmp(text_before,text_after));LRESULT prior=SendMessageW(check,BM_GETCHECK,0,0);SendMessageW(check,BM_CLICK,0,0);CHECK(SendMessageW(check,BM_GETCHECK,0,0)!=prior);SendMessageW(check,BM_CLICK,0,0);CHECK(SendMessageW(check,BM_GETCHECK,0,0)==prior);puts("VB6_UNEDGED_RESTORED client_font_text_check=1");
    }
    if(usercontrol_verify){
        HWND container=NULL,list=NULL;
        for(unsigned i=0;i<count;++i){WCHAR name[80];GetClassNameW(checks[i],name,80);
            if(!wcscmp(name,L"ThunderRT5UserControl") && (HMODULE)GetClassLongPtrW(checks[i],GCLP_HMODULE)==runtime)container=checks[i];
            if(!wcscmp(name,L"ThunderRT5ListBox") && (HMODULE)GetClassLongPtrW(checks[i],GCLP_HMODULE)==runtime)list=checks[i];
        }
        CHECK(container && list && GetParent(list)==container && GetParent(container)==form);
        LRESULT count_before=SendMessageW(list,LB_GETCOUNT,0,0),original=SendMessageW(list,LB_GETCURSEL,0,0);
        CHECK(count_before==2 && original>=0 && original<count_before);
        LRESULT other=original?0:1;CHECK(SendMessageW(list,LB_SETCURSEL,(WPARAM)other,0)==other && SendMessageW(list,LB_GETCURSEL,0,0)==other);
        CHECK(SendMessageW(list,LB_SETCURSEL,(WPARAM)original,0)==original && SendMessageW(list,LB_GETCURSEL,0,0)==original && SendMessageW(list,LB_GETCOUNT,0,0)==count_before);
        printf("VB_REAL_USERCONTROL_OK styled=%u runtime5=1 nested_list=1 count=2 selection_restored=1\n",styled);
    }
    if(real_vb5_picture){
      CHECK(!wcscpy_s(picture_dir,MAX_PATH,scratch));HMENU menu=GetMenu(form);CHECK(menu);UINT id=about_menu(menu);CHECK(id && id!=(UINT)-1);CHECK(PostMessageW(form,WM_COMMAND,MAKEWPARAM(id,0),0));DWORD tid=GetWindowThreadProcessId(form,NULL);for(unsigned tries=0;tries<200 && !picture_about;++tries){EnumThreadWindows(tid,locate_about,(LPARAM)form);if(!picture_about)Sleep(10);}CHECK(picture_about);CHECK(EnumChildWindows(picture_about,picture_buttons,0));CHECK(picture_count==2);CHECK(SendMessageW(picture_about,WM_CLOSE,0,0)==0);printf("VB5_PICTURE_SAMPLE_OK buttons=%u pressed_restore=1 disabled_restore=1 no_click=1\n",picture_count);
    }
    if(real_vb5_checklist){
      HWND target=NULL;for(unsigned i=0;i<count;++i){WCHAR cls[100];GetClassNameW(checks[i],cls,100);DWORD style=(DWORD)GetWindowLongPtrW(checks[i],GWL_STYLE);if(!wcscmp(cls,L"ThunderRT5ListBox") && (style&LBS_OWNERDRAWFIXED) && (style&LBS_MULTIPLESEL) && (HMODULE)GetClassLongPtrW(checks[i],GCLP_HMODULE)==runtime)target=checks[i];}
      CHECK(target && GetParent(target)==form && GetDpiForWindow(target)==96 && SendMessageW(target,LB_GETCOUNT,0,0)>1);RECT item;CHECK(SendMessageW(target,LB_GETITEMRECT,0,(LPARAM)&item)!=LB_ERR);LRESULT before=SendMessageW(target,LB_GETSEL,0,0);CHECK(before==0 || before==1);
      WCHAR file[MAX_PATH];CHECK(swprintf_s(file,MAX_PATH,L"%ls/vb5-before.bmp",scratch)>0);unsigned long long h1=capture(target,file,FALSE);
      LPARAM point=MAKELPARAM(item.left+5,item.top+5);SendMessageW(target,WM_LBUTTONDOWN,MK_LBUTTON,point);SendMessageW(target,WM_LBUTTONUP,0,point);LRESULT after=SendMessageW(target,LB_GETSEL,0,0);CHECK(after!=before && (after==0 || after==1));
      CHECK(swprintf_s(file,MAX_PATH,L"%ls/vb5-after.bmp",scratch)>0);unsigned long long h2=capture(target,file,FALSE);SendMessageW(target,WM_LBUTTONDOWN,MK_LBUTTON,point);SendMessageW(target,WM_LBUTTONUP,0,point);CHECK(SendMessageW(target,LB_GETSEL,0,0)==before);
      CHECK(swprintf_s(file,MAX_PATH,L"%ls/vb5-restored.bmp",scratch)>0);unsigned long long h3=capture(target,file,FALSE);EnableWindow(target,FALSE);RedrawWindow(target,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW);CHECK(swprintf_s(file,MAX_PATH,L"%ls/vb5-disabled.bmp",scratch)>0);capture(target,file,FALSE);SendMessageW(target,LB_SETSEL,TRUE,0);RedrawWindow(target,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW);CHECK(swprintf_s(file,MAX_PATH,L"%ls/vb5-disabled-checked.bmp",scratch)>0);capture(target,file,FALSE);SendMessageW(target,LB_SETSEL,before,0);EnableWindow(target,TRUE);CHECK(SendMessageW(target,LB_SETCARETINDEX,0,TRUE)!=LB_ERR);SendMessageW(target,WM_KEYDOWN,VK_SPACE,1);SendMessageW(target,WM_KEYUP,VK_SPACE,0xc0000001);LRESULT key1=SendMessageW(target,LB_GETSEL,0,0);SendMessageW(target,WM_KEYDOWN,VK_SPACE,1);SendMessageW(target,WM_KEYUP,VK_SPACE,0xc0000001);LRESULT key2=SendMessageW(target,LB_GETSEL,0,0);printf("VB5_CHECKLIST_KEY before=%lld after=%lld restored=%lld\n",(long long)before,(long long)key1,(long long)key2);CHECK(key1!=before && key2==before);printf("VB5_CHECKLIST_RESULT count=%lld before=%lld after=%lld restored=%lld rect=%ld,%ld,%ld,%ld hashes=%llu,%llu,%llu\n",(long long)SendMessageW(target,LB_GETCOUNT,0,0),(long long)before,(long long)after,(long long)SendMessageW(target,LB_GETSEL,0,0),item.left,item.top,item.right,item.bottom,h1,h2,h3);
    }
    if(real_vb5_sorted){
      HWND main=form,button=NULL;sorted_runtime=runtime;CHECK(GetDpiForWindow(main)==96);for(unsigned i=0;i<count;i++){WCHAR cls[80],title[80];GetClassNameW(checks[i],cls,80);CHECK(vb_control_text(checks[i],title,80));if(!wcscmp(cls,L"ThunderRT5CommandButton") && !wcscmp(title,L"...")){CHECK(!button);button=checks[i];}}CHECK(button && PostMessageW(button,BM_CLICK,0,0));DWORD tid=GetWindowThreadProcessId(main,NULL);for(unsigned i=0;i<200 && !sorted_form;i++){EnumThreadWindows(tid,sorted_dialog,(LPARAM)main);if(!sorted_form)Sleep(10);}CHECK(sorted_form);for(unsigned i=0;i<200 && (!sorted_list || !sorted_cancel);i++){CHECK(EnumChildWindows(sorted_form,sorted_children,0));if(!sorted_list || !sorted_cancel)Sleep(10);}CHECK(sorted_list && sorted_cancel);
      LRESULT before=SendMessageW(sorted_list,LB_GETCOUNT,0,0);CHECK(before>0 && before<=32);WCHAR prior_text[32][256];LRESULT prior_data[32],prior_sel[32];for(LRESULT i=0;i<before;i++){LRESULT n=SendMessageW(sorted_list,LB_GETTEXTLEN,i,0);CHECK(n>=0 && n<256 && SendMessageW(sorted_list,LB_GETTEXT,i,(LPARAM)prior_text[i])==n);prior_data[i]=SendMessageW(sorted_list,LB_GETITEMDATA,i,0);prior_sel[i]=SendMessageW(sorted_list,LB_GETSEL,i,0);CHECK(prior_sel[i]==0 || prior_sel[i]==1);}const WCHAR *names[]={L"zz-p27-fixture",L"aa-p27-fixture",L"mm-p27-fixture"};for(int i=0;i<3;i++)CHECK(SendMessageW(sorted_list,LB_ADDSTRING,0,(LPARAM)names[i])!=LB_ERR);LRESULT a=SendMessageW(sorted_list,LB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)names[1]),b=SendMessageW(sorted_list,LB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)names[2]),c=SendMessageW(sorted_list,LB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)names[0]);CHECK(a>=0 && a<b && b<c);CHECK(SendMessageW(sorted_list,LB_SETSEL,TRUE,a)!=LB_ERR && SendMessageW(sorted_list,LB_SETSEL,TRUE,c)!=LB_ERR && SendMessageW(sorted_list,LB_GETSEL,a,0)==1 && SendMessageW(sorted_list,LB_GETSEL,c,0)==1);WCHAR file[MAX_PATH];CHECK(swprintf_s(file,MAX_PATH,L"%ls/sorted-selected.bmp",scratch)>0);unsigned long long h=capture(sorted_list,file,FALSE);CHECK(SendMessageW(sorted_list,LB_DELETESTRING,c,0)!=LB_ERR && SendMessageW(sorted_list,LB_DELETESTRING,b,0)!=LB_ERR && SendMessageW(sorted_list,LB_DELETESTRING,a,0)!=LB_ERR && SendMessageW(sorted_list,LB_GETCOUNT,0,0)==before);for(LRESULT i=0;i<before;i++){WCHAR text[256];CHECK(SendMessageW(sorted_list,LB_GETTEXT,i,(LPARAM)text)!=LB_ERR && !wcscmp(text,prior_text[i]) && SendMessageW(sorted_list,LB_GETITEMDATA,i,0)==prior_data[i] && SendMessageW(sorted_list,LB_GETSEL,i,0)==prior_sel[i]);}printf("VB5_SORTED_RESULT count=%lld indices=%lld,%lld,%lld selected=2 sorted=1 restored=1 hash=%llu\n",(long long)before,(long long)a,(long long)b,(long long)c,h);CHECK(PostMessageW(sorted_cancel,BM_CLICK,0,0));for(unsigned i=0;i<200 && IsWindow(sorted_form);i++)Sleep(10);CHECK(!IsWindow(sorted_form));form=main;
    }
    if(real_vb6_sorted_combo){
      HWND combo=NULL;for(unsigned i=0;i<count;i++){WCHAR cls[80];GetClassNameW(checks[i],cls,80);DWORD style=(DWORD)GetWindowLongPtrW(checks[i],GWL_STYLE);if(!combo && !wcscmp(cls,L"ThunderRT6ComboBox") && (style&CBS_SORT) && (style&3)==CBS_DROPDOWN && IsWindowVisible(checks[i]) && (HMODULE)GetClassLongPtrW(checks[i],GCLP_HMODULE)==runtime && SendMessageW(checks[i],CB_GETCOUNT,0,0)==0)combo=checks[i];}
      CHECK(combo && GetDpiForWindow(combo)==96);WCHAR text[512],after[512];CHECK(vb_control_text(combo,text,512));HFONT original=(HFONT)SendMessageW(combo,WM_GETFONT,0,0);LRESULT selected=SendMessageW(combo,CB_GETCURSEL,0,0);CHECK(selected==CB_ERR);const WCHAR *names[]={L"zz-probe-fixture",L"aa-probe-fixture",L"mm-probe-fixture"};for(int i=0;i<3;i++)CHECK(SendMessageW(combo,CB_ADDSTRING,0,(LPARAM)names[i])!=CB_ERR);LRESULT a=SendMessageW(combo,CB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)names[1]),b=SendMessageW(combo,CB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)names[2]),c=SendMessageW(combo,CB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)names[0]);CHECK(a==0 && b==1 && c==2);COMBOBOXINFO info={.cbSize=sizeof(info)};CHECK(GetComboBoxInfo(combo,&info) && info.hwndList && info.hwndItem);CHECK(SendMessageW(combo,CB_SHOWDROPDOWN,TRUE,0) && SendMessageW(combo,CB_GETDROPPEDSTATE,0,0));WCHAR file[MAX_PATH];CHECK(swprintf_s(file,MAX_PATH,L"%ls/combo-list.bmp",scratch)>0);unsigned long long hash=capture(info.hwndList,file,FALSE);CHECK(SendMessageW(combo,CB_SHOWDROPDOWN,FALSE,0) && !SendMessageW(combo,CB_GETDROPPEDSTATE,0,0));CHECK(SendMessageW(combo,CB_DELETESTRING,c,0)!=CB_ERR && SendMessageW(combo,CB_DELETESTRING,b,0)!=CB_ERR && SendMessageW(combo,CB_DELETESTRING,a,0)!=CB_ERR && SendMessageW(combo,CB_GETCOUNT,0,0)==0);CHECK(vb_control_text(combo,after,512) && !wcscmp(text,after) && SendMessageW(combo,CB_GETCURSEL,0,0)==selected && (HFONT)SendMessageW(combo,WM_GETFONT,0,0)==original);printf("VB6_SORTED_COMBO_OK indices=%lld,%lld,%lld drop=1 cancel=1 text_selection_font_restore=1 hash=%llu\n",(long long)a,(long long)b,(long long)c,hash);
    }
    HWND original_form=form;
    if(real_disabled_picture){
        HWND easy=NULL,tile=NULL,game=NULL;WCHAR text[120],cls[100],file[MAX_PATH];
        for(unsigned i=0;i<count;++i){CHECK(vb_control_text(checks[i],text,120));if(!wcscmp(text,L"Easy"))easy=checks[i];}
        CHECK(easy && GetDpiForWindow(form)==96);DWORD tid=GetWindowThreadProcessId(form,NULL);
        SendMessageW(easy,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(5,5));SendMessageW(easy,WM_LBUTTONUP,0,MAKELPARAM(5,5));
        /* The show event can arrive during SendMessage and be missed. Restrict
         * the bounded fallback to the already-owned target UI thread. */
        for(unsigned retry=0;retry<100 && !game;++retry){EnumThreadWindows(tid,vb_disabled_game,(LPARAM)&game);if(!game)Sleep(10);}
        CHECK(game && WaitForInputIdle(target,5000)==0);form=game;count=0;CHECK(EnumChildWindows(form,collect,0));
        for(unsigned i=0;i<count;++i){GetClassNameW(checks[i],cls,100);CHECK(vb_control_text(checks[i],text,120));
            if(!wcscmp(cls,L"ThunderRT6CommandButton") && !text[0] && IsWindowVisible(checks[i]) &&
                ((DWORD)GetWindowLongPtrW(checks[i],GWL_STYLE)&BS_TYPEMASK)==BS_OWNERDRAW){tile=checks[i];break;}}
        CHECK(tile && IsWindowEnabled(tile));HFONT font=(HFONT)SendMessageW(tile,WM_GETFONT,0,0);LONG_PTR style=GetWindowLongPtrW(tile,GWL_STYLE);
        CHECK(swprintf_s(file,MAX_PATH,L"%ls/tile-normal.bmp",scratch)>0);uint64_t normal=capture(tile,file,FALSE);
        EnableWindow(tile,FALSE);CHECK(!IsWindowEnabled(tile));CHECK(swprintf_s(file,MAX_PATH,L"%ls/tile-disabled.bmp",scratch)>0);
        uint64_t disabled=capture(tile,file,FALSE);EnableWindow(tile,TRUE);
        CHECK(IsWindowEnabled(tile) && GetWindowLongPtrW(tile,GWL_STYLE)==style && (HFONT)SendMessageW(tile,WM_GETFONT,0,0)==font);
        CHECK(swprintf_s(file,MAX_PATH,L"%ls/tile-restored.bmp",scratch)>0);uint64_t restored=capture(tile,file,FALSE);
        CHECK(normal==restored && disabled!=normal);puts("VB_REAL_DISABLED_PICTURE_OK disabled=1 restored=1 no_tile_click=1");
    }
    if(real_vb5_custom){
        HWND style_button=NULL,check=NULL;WCHAR text[120],cls[100],file[MAX_PATH];
        for(unsigned i=0;i<count;++i){GetClassNameW(checks[i],cls,100);CHECK(vb_control_text(checks[i],text,120));
            if(!wcscmp(cls,L"ThunderRT5OptionButton") && !wcscmp(text,L"&Owner Draw"))style_button=checks[i];
            if(!check && !wcscmp(cls,L"ThunderRT5CheckBox") && !wcscmp(text,L"Check1"))check=checks[i];}
        CHECK(style_button && check);SendMessageW(style_button,BM_CLICK,0,0);CHECK(SendMessageW(style_button,BM_GETCHECK,0,0)==BST_CHECKED);
        LRESULT initial=SendMessageW(check,BM_GETCHECK,0,0);HFONT font=(HFONT)SendMessageW(check,WM_GETFONT,0,0);LONG_PTR style=GetWindowLongPtrW(check,GWL_STYLE);
        CHECK(swprintf_s(file,MAX_PATH,L"%ls/custom-normal.bmp",scratch)>0);(void)capture(check,file,FALSE);
        for(unsigned pass=0;pass<2;++pass){LRESULT before=SendMessageW(check,BM_GETCHECK,0,0);
            SendMessageW(check,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(5,5));SendMessageW(check,WM_LBUTTONUP,0,MAKELPARAM(5,5));
            for(unsigned wait=0;wait<50 && SendMessageW(check,BM_GETCHECK,0,0)==before;++wait)Sleep(10);
            CHECK(SendMessageW(check,BM_GETCHECK,0,0)!=before);
            if(!pass){CHECK(swprintf_s(file,MAX_PATH,L"%ls/custom-checked.bmp",scratch)>0);(void)capture(check,file,FALSE);}}
        CHECK(SendMessageW(check,BM_GETCHECK,0,0)==initial && GetWindowLongPtrW(check,GWL_STYLE)==style && (HFONT)SendMessageW(check,WM_GETFONT,0,0)==font);
        CHECK(swprintf_s(file,MAX_PATH,L"%ls/custom-restored.bmp",scratch)>0);(void)capture(check,file,FALSE);
        puts("VB5_CUSTOM_DRAW_OK toggles=2 state_font_style_restored=1");
    }
    if(close_vb5_squash){
        CHECK(ResetEvent(ready));(void)pump(ready,500);
        HWND mouse=NULL,keyboard=NULL;WCHAR text[120],cls[100];
        for(unsigned i=0;i<count;++i){GetClassNameW(checks[i],cls,100);CHECK(vb_control_text(checks[i],text,120));
            if(!wcscmp(cls,L"ThunderRT5OptionButton") && !wcscmp(text,L"Mouse"))mouse=checks[i];
            if(!wcscmp(cls,L"ThunderRT5OptionButton") && !wcscmp(text,L"Keyboard"))keyboard=checks[i];}
        CHECK(mouse && keyboard && !IsWindowEnabled(mouse));
        CHECK(((DWORD)GetWindowLongPtrW(mouse,GWL_STYLE)&BS_TYPEMASK)==BS_OWNERDRAW && ((DWORD)GetWindowLongPtrW(keyboard,GWL_STYLE)&BS_TYPEMASK)==BS_OWNERDRAW);
        LRESULT mouse_initial=SendMessageW(mouse,BM_GETCHECK,0,0),keyboard_initial=SendMessageW(keyboard,BM_GETCHECK,0,0);
        HFONT mouse_font=(HFONT)SendMessageW(mouse,WM_GETFONT,0,0),keyboard_font=(HFONT)SendMessageW(keyboard,WM_GETFONT,0,0);
        LONG_PTR mouse_style=GetWindowLongPtrW(mouse,GWL_STYLE),keyboard_style=GetWindowLongPtrW(keyboard,GWL_STYLE);
        EnableWindow(mouse,TRUE);CHECK(IsWindowEnabled(mouse));
        for(unsigned pass=0;pass<2;++pass){HWND choice=pass?keyboard:mouse;
            SendMessageW(choice,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(5,5));SendMessageW(choice,WM_LBUTTONUP,0,MAKELPARAM(5,5));
            for(unsigned wait=0;wait<50 && SendMessageW(choice,BM_GETCHECK,0,0)!=BST_CHECKED;++wait)Sleep(10);
            CHECK(SendMessageW(choice,BM_GETCHECK,0,0)==BST_CHECKED);}
        CHECK(SendMessageW(mouse,BM_GETCHECK,0,0)==mouse_initial && SendMessageW(keyboard,BM_GETCHECK,0,0)==keyboard_initial);
        EnableWindow(mouse,FALSE);CHECK(!IsWindowEnabled(mouse));
        CHECK(GetWindowLongPtrW(mouse,GWL_STYLE)==mouse_style && GetWindowLongPtrW(keyboard,GWL_STYLE)==keyboard_style &&
            (HFONT)SendMessageW(mouse,WM_GETFONT,0,0)==mouse_font && (HFONT)SendMessageW(keyboard,WM_GETFONT,0,0)==keyboard_font);
        CHECK(swprintf_s(path,MAX_PATH,L"%ls/vb5-option.bmp",scratch)>0);(void)capture(keyboard,path,FALSE);
        puts("VB5_REAL_GRAPHICAL_OPTION_OK selection=1 group_restore=1 disabled_restore=1");
    }
    if(real_vb5_check){
        HWND check=NULL;WCHAR text[120],cls[100];
        for(unsigned i=0;i<count;++i){GetClassNameW(checks[i],cls,100);CHECK(vb_control_text(checks[i],text,120));
            if(!wcscmp(cls,L"ThunderRT5CheckBox") && !wcscmp(text,L"Show Next Piece"))check=checks[i];}
        CHECK(check && IsWindowVisible(check) && !IsWindowEnabled(check));CHECK(((DWORD)GetWindowLongPtrW(check,GWL_STYLE)&BS_TYPEMASK)==BS_OWNERDRAW);
        LRESULT initial=SendMessageW(check,BM_GETCHECK,0,0);HFONT font=(HFONT)SendMessageW(check,WM_GETFONT,0,0);LONG_PTR style=GetWindowLongPtrW(check,GWL_STYLE);
        EnableWindow(check,TRUE);CHECK(IsWindowEnabled(check));
        for(unsigned pass=0;pass<2;++pass){LRESULT before=SendMessageW(check,BM_GETCHECK,0,0);
            SendMessageW(check,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(5,5));SendMessageW(check,WM_LBUTTONUP,0,MAKELPARAM(5,5));
            for(unsigned wait=0;wait<50 && SendMessageW(check,BM_GETCHECK,0,0)==before;++wait)Sleep(10);
            CHECK(SendMessageW(check,BM_GETCHECK,0,0)!=before);}
        CHECK(SendMessageW(check,BM_GETCHECK,0,0)==initial);EnableWindow(check,FALSE);
        CHECK(!IsWindowEnabled(check) && GetWindowLongPtrW(check,GWL_STYLE)==style && (HFONT)SendMessageW(check,WM_GETFONT,0,0)==font);
        CHECK(swprintf_s(path,MAX_PATH,L"%ls/vb5-checkbox.bmp",scratch)>0);(void)capture(check,path,FALSE);
        puts("VB5_REAL_GRAPHICAL_CHECK_OK toggles=2 disabled_restored=1");
    }
    if(real_checkbox){
        HWND check=NULL;WCHAR text[120],cls[100];
        for(unsigned i=0;i<count;++i){CHECK(vb_control_text(checks[i],text,120));GetClassNameW(checks[i],cls,100);
            if(!wcscmp(cls,L"ThunderRT6CheckBox") && !wcscmp(text,L"seam concealment"))check=checks[i];}
        CHECK(check && IsWindowVisible(check));CHECK(((DWORD)GetWindowLongPtrW(check,GWL_STYLE)&BS_TYPEMASK)==BS_OWNERDRAW);
        LRESULT initial=SendMessageW(check,BM_GETCHECK,0,0);
        for(unsigned pass=0;pass<2;++pass){LRESULT before=SendMessageW(check,BM_GETCHECK,0,0);
            SendMessageW(check,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(5,5));SendMessageW(check,WM_LBUTTONUP,0,MAKELPARAM(5,5));
            for(unsigned wait=0;wait<50 && SendMessageW(check,BM_GETCHECK,0,0)==before;++wait)Sleep(10);
            CHECK(SendMessageW(check,BM_GETCHECK,0,0)!=before);}
        CHECK(SendMessageW(check,BM_GETCHECK,0,0)==initial);
        puts("VB_REAL_GRAPHICAL_CHECK_OK runtime6=1 ownerdraw=1 toggles=2 restored=1");
    }
    if(real_picture){
        unsigned options=0,checkboxes=0,commands=0;
        for(unsigned i=0;i<count;++i){
            WCHAR cls[100],text[120],file[MAX_PATH];HWND w=checks[i];GetClassNameW(w,cls,100);CHECK(vb_control_text(w,text,120));
            BOOL check=!wcscmp(cls,L"ThunderRT6CheckBox") && !wcscmp(text,L"Check1");
            BOOL option=!wcscmp(cls,L"ThunderRT6OptionButton") && !wcscmp(text,L"Option1");
            BOOL command=!wcscmp(cls,L"ThunderRT6CommandButton") && !wcscmp(text,L"Command1");
            if(!check && !option && !command)continue;
            LONG_PTR style=GetWindowLongPtrW(w,GWL_STYLE);CHECK((style&BS_TYPEMASK)==BS_OWNERDRAW);
            LRESULT initial=SendMessageW(w,BM_GETCHECK,0,0);HFONT font=(HFONT)SendMessageW(w,WM_GETFONT,0,0);
            CHECK(swprintf_s(file,MAX_PATH,L"%ls/control-%u-normal.bmp",scratch,i)>0);uint64_t normal=capture(w,file,FALSE);
            if(command){
                ++commands;CHECK(!(SendMessageW(w,BM_GETSTATE,0,0)&BST_PUSHED));
                SendMessageW(w,BM_SETSTATE,TRUE,0);CHECK(SendMessageW(w,BM_GETSTATE,0,0)&BST_PUSHED);
                CHECK(swprintf_s(file,MAX_PATH,L"%ls/control-%u-pressed.bmp",scratch,i)>0);(void)capture(w,file,FALSE);
                SendMessageW(w,BM_SETSTATE,FALSE,0);CHECK(!(SendMessageW(w,BM_GETSTATE,0,0)&BST_PUSHED));
                CHECK(swprintf_s(file,MAX_PATH,L"%ls/control-%u-restored.bmp",scratch,i)>0);CHECK(capture(w,file,FALSE)==normal);
            }else if(check){
                ++checkboxes;CHECK(initial==BST_UNCHECKED);
                for(unsigned pass=0;pass<2;++pass){LRESULT before=SendMessageW(w,BM_GETCHECK,0,0);
                    SendMessageW(w,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(5,5));SendMessageW(w,WM_LBUTTONUP,0,MAKELPARAM(5,5));
                    for(unsigned wait=0;wait<50 && SendMessageW(w,BM_GETCHECK,0,0)==before;++wait)Sleep(10);
                    CHECK(SendMessageW(w,BM_GETCHECK,0,0)!=before);
                    if(!pass){CHECK(swprintf_s(file,MAX_PATH,L"%ls/control-%u-checked.bmp",scratch,i)>0);(void)capture(w,file,FALSE);}}
                CHECK(SendMessageW(w,BM_GETCHECK,0,0)==initial);
            }else ++options;
            EnableWindow(w,FALSE);CHECK(!IsWindowEnabled(w));CHECK(swprintf_s(file,MAX_PATH,L"%ls/control-%u-disabled.bmp",scratch,i)>0);
            (void)capture(w,file,FALSE);EnableWindow(w,TRUE);
            CHECK((HFONT)SendMessageW(w,WM_GETFONT,0,0)==font && GetWindowLongPtrW(w,GWL_STYLE)==style && SendMessageW(w,BM_GETCHECK,0,0)==initial);
            printf("VB_REAL_PICTURE_CONTROL index=%u class=%ls normal_disabled_restored=1\n",i,cls);
        }
        CHECK(options==2 && checkboxes==1 && commands==1);puts("VB_REAL_PICTURE_STATES_OK options=2 checkbox=1 command=1 selected=1 pressed=1 disabled=1 restored=1");
    }
    if(real_graphical){
        HWND start=NULL;WCHAR text[120];
        for(unsigned i=0;i<count;++i){CHECK(vb_control_text(checks[i],text,120));if(!wcscmp(text,L"Test Yourself"))start=checks[i];}
        CHECK(start && ResetEvent(ready));SendMessageW(start,BM_CLICK,0,0);
        CHECK(pump(ready,5000)==WAIT_OBJECT_0 && IsWindowVisible(form));
        count=0;CHECK(EnumChildWindows(form,collect,0));HWND go=NULL;
        for(unsigned i=0;i<count;++i){CHECK(vb_control_text(checks[i],text,120));if(!wcscmp(text,L"Ready"))go=checks[i];}
        CHECK(go);SendMessageW(go,BM_CLICK,0,0);count=0;CHECK(EnumChildWindows(form,collect,0));
        HWND choice=NULL,next=NULL;WCHAR cls[100],parent_text[120];
        for(unsigned i=0;i<count;++i){
            GetClassNameW(checks[i],cls,100);CHECK(vb_control_text(checks[i],text,120));
            if(!wcscmp(cls,L"ThunderRT6Frame") && !wcscmp(text,L"Frame2"))next=checks[i];
            if(!wcscmp(cls,L"ThunderRT6OptionButton") && !wcscmp(text,L"Light")){
                CHECK(vb_control_text(GetParent(checks[i]),parent_text,120));
                if(!wcscmp(parent_text,L"Frame1"))choice=checks[i];
            }
        }
        CHECK(choice && next && IsWindowVisible(choice) && !IsWindowVisible(next));
        CHECK(((DWORD)GetWindowLongPtrW(choice,GWL_STYLE)&BS_TYPEMASK)==BS_OWNERDRAW);
        SendMessageW(choice,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(5,5));
        SendMessageW(choice,WM_LBUTTONUP,0,MAKELPARAM(5,5));
        for(unsigned i=0;i<50 && !IsWindowVisible(next);++i)Sleep(10);
        CHECK(IsWindowVisible(next) && SendMessageW(choice,BM_GETCHECK,0,0)==BST_CHECKED);
        puts("VB_REAL_GRAPHICAL_OPTION_OK runtime6=1 ownerdraw=1 selected=1 event=1");
    }
    if(mdi_verify){
        HWND mdi=NULL,child=NULL;
        for(unsigned i=0;i<count;i++){
            WCHAR cls[80];GetClassNameW(checks[i],cls,80);
            if(!wcscmp(cls,L"MDIClient"))mdi=checks[i];
            else if((GetWindowLongPtrW(checks[i],GWL_EXSTYLE)&WS_EX_MDICHILD) && IsWindowVisible(checks[i]))child=checks[i];
        }
        CHECK(mdi && child && GetParent(child)==mdi);DWORD_PTR result=0;
        if(styled){
            RECT r;CHECK(GetWindowRect(child,&r));POINT point={r.right-10,r.top+10};
            CHECK(SendMessageW(child,WM_NCHITTEST,0,MAKELPARAM(point.x,point.y))==HTMAXBUTTON);
            CHECK(SendMessageTimeoutW(child,WM_NCLBUTTONDOWN,HTMAXBUTTON,MAKELPARAM(point.x,point.y),SMTO_ABORTIFHUNG|SMTO_BLOCK,3000,&result));
            CHECK(ScreenToClient(child,&point));
            CHECK(SendMessageTimeoutW(child,WM_LBUTTONUP,0,MAKELPARAM(point.x,point.y),SMTO_ABORTIFHUNG|SMTO_BLOCK,3000,&result));
            for(unsigned i=0;i<100 && !IsZoomed(child);++i)Sleep(10);
            CHECK(IsZoomed(child));
        }else CHECK(SendMessageTimeoutW(mdi,WM_MDIMAXIMIZE,(WPARAM)child,0,SMTO_ABORTIFHUNG|SMTO_BLOCK,3000,&result) && IsZoomed(child));
        CHECK(swprintf_s(path,MAX_PATH,L"%ls/mdi-max.bmp",scratch)>0);(void)capture(form,path,FALSE);
        CHECK(SendMessageTimeoutW(mdi,WM_MDIRESTORE,(WPARAM)child,0,SMTO_ABORTIFHUNG|SMTO_BLOCK,3000,&result) && !IsZoomed(child));
        CHECK(swprintf_s(path,MAX_PATH,L"%ls/mdi-restored.bmp",scratch)>0);(void)capture(form,path,FALSE);
        printf("VB_REAL_MDI_OK styled=%u button=%u maximized=1 restored=1\n",styled,styled);
    }
    CHECK(swprintf_s(path,MAX_PATH,L"%ls/actor-%ls.bmp",scratch,styled?L"styled":L"bare")>0);
    printf("VB_SAMPLE_IMAGE hash=%llu\n",(unsigned long long)capture(form,path,FALSE));
    printf("SAMPLE_ROOT class_owner=%p windows=%u\n",(void *)GetClassLongPtrW(form,GCLP_HMODULE),count);
    if(!inventory){
    unsigned options=0,checkboxes=0,combos=0,lists=0,edits=0,bars=0;
    for(unsigned i=0;i<count;++i){HWND w=checks[i];WCHAR cls[100];GetClassNameW(w,cls,100);DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);HMODULE owner=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE);HFONT original_font=(HFONT)SendMessageW(w,WM_GETFONT,0,0);RECT original_rect,after_rect;CHECK(GetWindowRect(w,&original_rect));
        if(owner!=runtime)continue;
        CHECK(swprintf_s(path,MAX_PATH,L"%ls/control-%u.bmp",scratch,i)>0);uint64_t hash=capture(w,path,!wcscmp(argv[3],L"verify") && (wcsstr(cls,L"OptionButton") || wcsstr(cls,L"CheckBox") || wcsstr(cls,L"ComboBox") || wcsstr(cls,L"ScrollBar")));printf("SAMPLE_PIXELS index=%u class=%ls hash=%llu\n",i,cls,(unsigned long long)hash);
        if(owner!=runtime)continue;
        if(wcsstr(cls,L"OptionButton")){++options;if(!IsWindowEnabled(w))continue;SendMessageW(w,BM_CLICK,0,0);CHECK(SendMessageW(w,BM_GETCHECK,0,0)==BST_CHECKED);for(unsigned j=0;j<count;++j){WCHAR other[100];GetClassNameW(checks[j],other,100);if(checks[j]!=w && wcsstr(other,L"OptionButton") && GetParent(checks[j])==GetParent(w))CHECK(SendMessageW(checks[j],BM_GETCHECK,0,0)==BST_UNCHECKED);}CHECK(swprintf_s(path,MAX_PATH,L"%ls/selected-%u.bmp",scratch,i)>0);(void)capture(w,path,!wcscmp(argv[3],L"verify"));printf("SAMPLE_OPTION_GROUP_OK index=%u checked=1 siblings_cleared=1\n",i);}
        else if(wcsstr(cls,L"CheckBox")){++checkboxes;if(!IsWindowEnabled(w))continue;LRESULT original=SendMessageW(w,BM_GETCHECK,0,0);SendMessageW(w,BM_CLICK,0,0);CHECK(swprintf_s(path,MAX_PATH,L"%ls/selected-%u.bmp",scratch,i)>0);(void)capture(w,path,!wcscmp(argv[3],L"verify"));SendMessageW(w,BM_CLICK,0,0);CHECK(SendMessageW(w,BM_GETCHECK,0,0)==original);printf("SAMPLE_CHECK_CYCLE_OK index=%u round2=initial\n",i);}
        else if(wcsstr(cls,L"ComboBox")){++combos;LRESULT n=SendMessageW(w,CB_GETCOUNT,0,0);CHECK(n>1);SendMessageW(w,CB_SETCURSEL,1,0);CHECK(SendMessageW(w,CB_GETCURSEL,0,0)==1);SendMessageW(w,CB_SHOWDROPDOWN,TRUE,0);CHECK(SendMessageW(w,CB_GETDROPPEDSTATE,0,0));SendMessageW(w,CB_SHOWDROPDOWN,FALSE,0);CHECK(!SendMessageW(w,CB_GETDROPPEDSTATE,0,0));printf("SAMPLE_COMBO_SELECT_OK index=%u count=%lld select1=1 drop_cancel=1\n",i,(long long)n);}
        else if(wcsstr(cls,L"ListBox")){++lists;LRESULT n=SendMessageW(w,LB_GETCOUNT,0,0);CHECK(n>1 && SendMessageW(w,LB_SETCURSEL,1,0)!=LB_ERR && SendMessageW(w,LB_GETCURSEL,0,0)==1);printf("SAMPLE_LIST_SELECT_OK index=%u count=%lld select1=1\n",i,(long long)n);}
        else if(wcsstr(cls,L"TextBox")){++edits;WCHAR original[512],actual[512];CHECK(SendMessageW(w,WM_GETTEXT,512,(LPARAM)original));CHECK(SendMessageW(w,WM_SETTEXT,0,(LPARAM)L"Native sample probe"));SendMessageW(w,WM_GETTEXT,512,(LPARAM)actual);CHECK(!wcscmp(actual,L"Native sample probe"));CHECK(SendMessageW(w,WM_SETTEXT,0,(LPARAM)original));SendMessageW(w,WM_GETTEXT,512,(LPARAM)actual);CHECK(!wcscmp(original,actual));printf("SAMPLE_EDIT_OK index=%u text_restore=1 multiline=%u cross_process_replace_undo=not_covered\n",i,(style&ES_MULTILINE)!=0);}
        else if(wcsstr(cls,L"ScrollBar")){++bars;SCROLLINFO info={.cbSize=sizeof(info),.fMask=SIF_ALL};CHECK(GetScrollInfo(w,SB_CTL,&info));printf("SAMPLE_SCROLL_OK index=%u min=%d max=%d pos=%d page=%u\n",i,info.nMin,info.nMax,info.nPos,info.nPage);}
        if(queue_state && wcsstr(cls,L"ComboBox")){
            CHECK(SendMessageW(w,CB_GETCURSEL,0,0)==1);queue_key(w,VK_DOWN);
            CHECK(SendMessageW(w,CB_GETCURSEL,0,0)==2);
            CHECK(queue_state->commands==2 && queue_state->command_code==CBN_SELCHANGE);
            printf("VB_SAMPLE_COMBO_INPUT_OK index=%u selected=2 commands=%ld\n",i,queue_state->commands);
        }else if(queue_state && wcsstr(cls,L"ListBox")){
            LRESULT n=SendMessageW(w,LB_GETCOUNT,0,0);queue_key(w,VK_END);
            CHECK(SendMessageW(w,LB_GETCURSEL,0,0)==n-1);
            CHECK(queue_state->commands==1 && queue_state->command_code==LBN_SELCHANGE);
            printf("VB_SAMPLE_LIST_INPUT_OK index=%u selected=%lld commands=%ld\n",i,(long long)n-1,queue_state->commands);
        }else if(queue_state && wcsstr(cls,L"ScrollBar")){
            SCROLLINFO state={.cbSize=sizeof(state),.fMask=SIF_ALL};CHECK(GetScrollInfo(w,SB_CTL,&state));
            int original=state.nPos;queue_key(w,VK_END);CHECK(GetScrollInfo(w,SB_CTL,&state));
            CHECK(state.nPos>original && state.nPos<=state.nMax);int end=state.nPos;
            queue_key(w,VK_HOME);CHECK(GetScrollInfo(w,SB_CTL,&state) && state.nPos==state.nMin);
            CHECK(queue_state->scroll_code==SB_ENDSCROLL &&
                ((style&SBS_VERT)?queue_state->vscroll==2 && !queue_state->hscroll:queue_state->hscroll==2 && !queue_state->vscroll));
            printf("VB_SAMPLE_SCROLL_INPUT_OK index=%u end=%d home=%d hscroll=%ld vscroll=%ld\n",i,end,state.nPos,queue_state->hscroll,queue_state->vscroll);
        }
        CHECK((HFONT)SendMessageW(w,WM_GETFONT,0,0)==original_font && GetWindowRect(w,&after_rect) && EqualRect(&original_rect,&after_rect));
    }
    CHECK(options==4 && checkboxes==4 && combos==2 && lists==1 && edits==2 && bars==2);
    printf("SAMPLE_PROTOCOL_PASS option=%u checkbox=%u combo=%u list=%u edit=%u scrollbar=%u\n",options,checkboxes,combos,lists,edits,bars);
    if(!wcscmp(argv[3],L"disabled")){
        unsigned tested=0;
        for(unsigned i=0;i<count;++i){
            HWND w=checks[i];WCHAR cls[100];GetClassNameW(w,cls,100);
            BOOL choice=wcsstr(cls,L"CheckBox") || wcsstr(cls,L"OptionButton");
            BOOL combo=wcsstr(cls,L"ComboBox")!=NULL;
            if((!choice && !combo) || !IsWindowEnabled(w))continue;
            LONG_PTR style=GetWindowLongPtrW(w,GWL_STYLE);
            HFONT font=(HFONT)SendMessageW(w,WM_GETFONT,0,0);
            LRESULT prior=choice?SendMessageW(w,BM_GETCHECK,0,0):SendMessageW(w,CB_GETCURSEL,0,0);
            CHECK(swprintf_s(path,MAX_PATH,L"%ls/normal-%u.bmp",scratch,i)>0);
            uint64_t normal=capture(w,path,FALSE);
            EnableWindow(w,FALSE);CHECK(!IsWindowEnabled(w));
            /* Do not send BM_CLICK: VB may process an explicitly sent click
             * even though a disabled control cannot receive a user click. */
            CHECK(swprintf_s(path,MAX_PATH,L"%ls/disabled-%u.bmp",scratch,i)>0);
            uint64_t disabled=capture(w,path,styled);
            if(styled){CHECK(normal!=disabled);}
            EnableWindow(w,TRUE);CHECK(IsWindowEnabled(w));
            CHECK((choice?SendMessageW(w,BM_GETCHECK,0,0):SendMessageW(w,CB_GETCURSEL,0,0))==prior &&
                GetWindowLongPtrW(w,GWL_STYLE)==style && (HFONT)SendMessageW(w,WM_GETFONT,0,0)==font);
            printf("VB_DISABLED_OK class=%ls index=%u state_preserved=1 pixels_changed=%d restored=1\n",cls,i,normal!=disabled);
            ++tested;
        }
        CHECK(tested==10);printf("VB_DISABLED_SAMPLE_OK count=%u styled=%d\n",tested,styled);
    }
    }else {
        if(graphical){
            CHECK(queue_state);unsigned found=0;
            for(unsigned i=0;i<count;++i){HWND w=checks[i];WCHAR cls[100];GetClassNameW(w,cls,100);
                if((wcsstr(cls,L"CommandButton") || wcsstr(cls,L"CheckBox") || wcsstr(cls,L"OptionButton")) &&
                        (GetWindowLongPtrW(w,GWL_STYLE)&BS_TYPEMASK)==BS_OWNERDRAW){
                    HFONT source=(HFONT)SendMessageW(w,WM_GETFONT,0,0);RECT before,after,client_before,client_after;
                    LRESULT button_state=SendMessageW(w,BM_GETSTATE,0,0);
                    CHECK(GetWindowRect(w,&before) && GetClientRect(w,&client_before));
                    queue_survey(w,VB_QUEUE_GRAPHICAL_SURVEY);++found;
                    CHECK(queue_state->paint_restored && !queue_state->paint_probe_error);
                    if(graphical_verify)CHECK(styled?(queue_state->attached && queue_state->system_text>0):!queue_state->system_text);
                    /* Optional source-reviewed 3Ddefline: ten image-only
                     * commands. Never use this mode to click business code. */
                    if(graphical_pictures)CHECK(!wcscmp(cls,L"ThunderRT6CommandButton") && !GetWindowTextLengthW(w) &&
                        queue_state->graphical_frames==1 && queue_state->graphical_type==DFC_BUTTON &&
                        queue_state->graphical_state==DFCS_BUTTONPUSH && (styled?queue_state->attached:queue_state->attached==-1));
                    printf("VB_GRAPHICAL_API index=%u class=%ls attached=%ld frames=%ld type=%lu state=%lu rect=%ld,%ld,%ld,%ld dc_window=%p blits=%ld text=%ld restored=%ld\n",i,cls,queue_state->attached,queue_state->graphical_frames,queue_state->graphical_type,queue_state->graphical_state,queue_state->graphical_rect.left,queue_state->graphical_rect.top,queue_state->graphical_rect.right,queue_state->graphical_rect.bottom,(void *)queue_state->graphical_dc_window,queue_state->graphical_blits,queue_state->graphical_text,queue_state->paint_restored);
                    /* IAT wrappers change the direct caller; inspect actual
                     * style pixels after restoring imports, without wrappers. */
                    graphical_pixel_check=(graphical_verify || graphical_pictures) && styled;
                    CHECK(swprintf_s(path,MAX_PATH,L"%ls/graphical-%u.bmp",scratch,i)>0);uint64_t original_image=capture(w,path,FALSE);graphical_pixel_check=FALSE;
                    if(graphical_pictures){
                        LONG commands=queue_state->commands;CHECK(IsWindowEnabled(w) && !(button_state&BST_PUSHED));
                        EnableWindow(w,FALSE);CHECK(!IsWindowEnabled(w));queue_survey(w,VB_QUEUE_GRAPHICAL_SURVEY);
                        CHECK(queue_state->paint_restored && !queue_state->paint_probe_error && (queue_state->graphical_state&DFCS_INACTIVE));
                        CHECK(swprintf_s(path,MAX_PATH,L"%ls/graphical-%u-disabled.bmp",scratch,i)>0);(void)capture(w,path,FALSE);
                        EnableWindow(w,TRUE);SendMessageW(w,BM_SETSTATE,TRUE,0);CHECK(SendMessageW(w,BM_GETSTATE,0,0)&BST_PUSHED);
                        queue_survey(w,VB_QUEUE_GRAPHICAL_SURVEY);
                        CHECK(queue_state->paint_restored && !queue_state->paint_probe_error && (queue_state->graphical_state&DFCS_PUSHED));
                        CHECK(swprintf_s(path,MAX_PATH,L"%ls/graphical-%u-pressed.bmp",scratch,i)>0);(void)capture(w,path,FALSE);
                        SendMessageW(w,BM_SETSTATE,FALSE,0);
                        CHECK(swprintf_s(path,MAX_PATH,L"%ls/graphical-%u-restored.bmp",scratch,i)>0);
                        CHECK(capture(w,path,FALSE)==original_image);queue_survey(w,0);
                        CHECK(queue_state->commands==commands && !GetCapture());
                        RECT root_rect;CHECK(GetWindowRect(form,&root_rect));
                        printf("VB_GRAPHICAL_PICTURE_STATES_OK index=%u disabled=1 pressed=1 restored_pixels=1 no_click=1 root_offset=%ld,%ld size=%ld,%ld\n",
                            i,before.left-root_rect.left,before.top-root_rect.top,before.right-before.left,before.bottom-before.top);
                    }
                    queue_survey(w,VB_QUEUE_PAINT_SURVEY);
                    if(graphical_verify)CHECK(styled?queue_state->paint_system>0:!queue_state->paint_system);
                    CHECK((HFONT)SendMessageW(w,WM_GETFONT,0,0)==source && SendMessageW(w,BM_GETSTATE,0,0)==button_state &&
                        GetWindowRect(w,&after) && GetClientRect(w,&client_after) && EqualRect(&before,&after) && EqualRect(&client_before,&client_after));
                    printf("VB_GRAPHICAL_PROPERTIES_OK index=%u source_font=%p geometry_state_preserved=1 dpi=%u\n",i,(void *)source,GetDpiForWindow(w));
                    /* These optional Mouse sample buttons have no Click
                     * handler; their native notification is safe to observe. */
                    if(graphical_verify && IsWindowEnabled(w)){
                        LONG commands=queue_state->commands;SendMessageW(w,BM_CLICK,0,0);queue_survey(w,0);
                        CHECK(queue_state->commands==commands+1 && queue_state->command_code==BN_CLICKED);
                        printf("VB_GRAPHICAL_CLICK_OK index=%u native_parent_notification=1\n",i);
                    }
                }
            }
            CHECK(found>0 && (!graphical_pictures || found==10));printf("VB_GRAPHICAL_INVENTORY_OK controls=%u\n",found);
        }
        if(about_print){
            CHECK(queue_state);HWND main=form,button=NULL;
            CHECK(!wcscpy_s(about_capture_dir,MAX_PATH,scratch));
            for(unsigned i=0;i<count;++i){WCHAR text[80];CHECK(vb_control_text(checks[i],text,80));if(!wcscmp(text,L"About..."))button=checks[i];}
            CHECK(button);queue_survey(button,VB_QUEUE_PRINT_SAMPLE);
            CHECK(queue_state->print_calls>0 && queue_state->print_api==3);
            CHECK(EnumThreadWindows(queue_state->tid,close_about,(LPARAM)main) && about_closed==1);form=main;
        }
        if(containers){
            CHECK(queue_state);queue_survey(form,VB_QUEUE_PAINT_SURVEY);
            unsigned pictures=0,label_hosts=0;
            for(unsigned i=0;i<count;++i){WCHAR cls[100];GetClassNameW(checks[i],cls,100);
                if(wcsstr(cls,L"PictureBox") || wcsstr(cls,L"Frame")){
                    HFONT font=(HFONT)SendMessageW(checks[i],WM_GETFONT,0,0);RECT before,after,client_before,client_after;
                    CHECK(GetWindowRect(checks[i],&before) && GetClientRect(checks[i],&client_before));
                    queue_survey(checks[i],VB_QUEUE_PAINT_SURVEY);
                    if(wcsstr(cls,L"PictureBox")){
                        ++pictures;if(queue_state->paint_system>0)++label_hosts;
                        if(container_verify)CHECK(styled?queue_state->attached==1:queue_state->paint_system==0);
                    }
                    CHECK((HFONT)SendMessageW(checks[i],WM_GETFONT,0,0)==font && GetWindowRect(checks[i],&after) && GetClientRect(checks[i],&client_after) && EqualRect(&before,&after) && EqualRect(&client_before,&client_after));
                    CHECK(swprintf_s(path,MAX_PATH,L"%ls/container-%u.bmp",scratch,i)>0);(void)capture(checks[i],path,FALSE);
                }
            }
            if(container_verify){CHECK(pictures>0 && (styled?label_hosts>0:label_hosts==0));printf("VB_REAL_CONTAINERS_OK pictures=%u system_label_hosts=%u font_geometry_preserved=1\n",pictures,label_hosts);}
        }
        unsigned file_controls=0;CHECK(!file_verify || queue_state);
        if(queue_state)for(unsigned i=0;i<count;++i){
            WCHAR cls[100];GetClassNameW(checks[i],cls,100);
            if(wcsstr(cls,L"FileListBox") || wcsstr(cls,L"DirListBox") || wcsstr(cls,L"DriveListBox")){
                HWND w=checks[i];BOOL drive=wcsstr(cls,L"DriveListBox")!=NULL;
                HFONT source=(HFONT)SendMessageW(w,WM_GETFONT,0,0);RECT before,after;CHECK(GetWindowRect(w,&before));
                LRESULT selection=SendMessageW(w,drive?CB_GETCURSEL:LB_GETCURSEL,0,0);
                queue_survey(w,drive?VB_QUEUE_POPUP_SURVEY:VB_QUEUE_SURVEY);
                if(file_verify){
                    CHECK((HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)==runtime);
                    CHECK(styled?(queue_state->attached==1 && queue_state->system_text>0):queue_state->system_text==0);
                    CHECK((HFONT)SendMessageW(w,WM_GETFONT,0,0)==source && GetWindowRect(w,&after) && EqualRect(&before,&after));
                    CHECK(SendMessageW(w,drive?CB_GETCURSEL:LB_GETCURSEL,0,0)==selection);
                    if(drive){SendMessageW(w,CB_SHOWDROPDOWN,TRUE,0);queue_key(w,VK_ESCAPE);
                        CHECK(!SendMessageW(w,CB_GETDROPPEDSTATE,0,0) && SendMessageW(w,CB_GETCURSEL,0,0)==selection);}
                    if(filedir_verify && wcsstr(cls,L"FileListBox")){
                        LRESULT n=SendMessageW(w,LB_GETCOUNT,0,0);CHECK(n>1);
                        queue_key(w,VK_END);CHECK(SendMessageW(w,LB_GETCURSEL,0,0)==n-1 && queue_state->commands>0);
                        queue_key(w,VK_HOME);CHECK(SendMessageW(w,LB_GETCURSEL,0,0)==0 && queue_state->commands>0);
                        SendMessageW(w,LB_SETCURSEL,(WPARAM)selection,0);CHECK(SendMessageW(w,LB_GETCURSEL,0,0)==selection);
                        puts("VB_REAL_FILE_INPUT_OK queued_end_home=1 parent_notifications=1 selection_restored=1");
                    }
                    ++file_controls;
                }
            }
        }
        if(file_verify){CHECK(file_controls==(filedir_verify?2u:3u));printf("VB_REAL_FILES_OK runtime_owned=%u styled=%u actual_system_controls=%u font_geometry_selection_preserved=1 drive_escape=%u\n",file_controls,styled,styled?file_controls:0,!filedir_verify);}
        printf("VB_SAMPLE_INVENTORY_OK windows=%u selection_unchanged=1\n",count);
    }

    if(queue_state){CHECK(UnhookWindowsHookEx(queue_get) && UnhookWindowsHookEx(queue_sent));queue_get=queue_sent=NULL;}
    if(real_vb5_sorted){CHECK(TerminateJobObject(job,0));puts("VB5_SORTED_FORCED_JOB_CLEANUP to_avoid_registry_write=1");}
    else if(real_vb6_sorted_combo)CHECK(PostMessageW(form,WM_CLOSE,0,0));
    else SendMessageW(form,WM_CLOSE,0,0);
    if((real_graphical || real_disabled_picture) && original_form!=form && IsWindow(original_form))SendMessageW(original_form,WM_CLOSE,0,0);
    CHECK(WaitForSingleObject(target,5000)==WAIT_OBJECT_0 && WaitForSingleObject(p.hProcess,5000)==WAIT_OBJECT_0);
    DWORD code;
    CHECK(GetExitCodeProcess(target,&code) && !code);
    BOOL zero=FALSE;
    ULONGLONG deadline=GetTickCount64()+3000;
    while(!zero){
        ULONGLONG now=GetTickCount64();if(now>=deadline)break;
        DWORD message;
        ULONG_PTR key;
        OVERLAPPED *overlap;
        CHECK(GetQueuedCompletionStatus(port,&message,&key,&overlap,(DWORD)(deadline-now)));
        if(key==(ULONG_PTR)job && message==JOB_OBJECT_MSG_ACTIVE_PROCESS_ZERO)zero=TRUE;
    }
    CHECK(zero);
    JOBOBJECT_BASIC_ACCOUNTING_INFORMATION active;
    CHECK(QueryInformationJobObject(job,JobObjectBasicAccountingInformation,&active,sizeof(active),NULL) && !active.ActiveProcesses);
    CloseHandle(target);
    CloseHandle(p.hProcess);
    CloseHandle(p.hThread);
    UnhookWinEvent(hook);
    CloseHandle(ready);
    CloseHandle(job);
    CloseHandle(port);
    if(queue_state){
        CHECK(FreeLibrary(queue_module) && UnmapViewOfFile(queue_state));queue_state=NULL;
        CHECK(CloseHandle(queue_ready) && CloseHandle(queue_mapping));
        CHECK(SetEnvironmentVariableW(VB_QUEUE_ENV,NULL));
        puts("VB_SAMPLE_QUEUE_CLEANUP_OK owned_thread_hooks=0 mappings=0 events=0 active=0");
    }
    printf("SAMPLE_PASS styled=%d count=%u active=0\n",styled,count);
    return 0;
}
int wmain(int argc,WCHAR **argv){
    return isolated_process(argc,argv,worker);
}
