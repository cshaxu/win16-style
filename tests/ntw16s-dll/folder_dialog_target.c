#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commdlg.h>
#include <dwmapi.h>
#include <shlobj.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
static DWORD owner_thread;
static volatile LONG seen, changed, initialized;
static HWND dialog;
static BOOL cancel_mode, new_folder_mode, same_mode, controls_mode;
static BOOL controls_ok;
static volatile LONG shell_seen;
static WCHAR wanted[MAX_PATH], initial[MAX_PATH], current[MAX_PATH];
static BOOL screen_pixels(int x,int y,int width,int height,DWORD *pixels)
{
    HDC screen=GetDC(NULL);if(!screen)return FALSE;
    HDC memory=CreateCompatibleDC(screen);BITMAPINFO bi={0};bi.bmiHeader.biSize=sizeof(bi.bmiHeader);
    bi.bmiHeader.biWidth=width;bi.bmiHeader.biHeight=-height;bi.bmiHeader.biPlanes=1;bi.bmiHeader.biBitCount=32;
    void *bits=NULL;HBITMAP bitmap=CreateDIBSection(screen,&bi,DIB_RGB_COLORS,&bits,NULL,0);BOOL ok=FALSE;
    if(memory && bitmap && bits){HGDIOBJ prior=SelectObject(memory,bitmap);
        ok=BitBlt(memory,0,0,width,height,screen,x,y,SRCCOPY|CAPTUREBLT);GdiFlush();
        if(ok)memcpy(pixels,bits,(size_t)width*(size_t)height*sizeof(DWORD));
        SelectObject(memory,prior);}
    if(bitmap)DeleteObject(bitmap);
    if(memory)DeleteDC(memory);
    ReleaseDC(NULL,screen);return ok;
}
static void CALLBACK test_controls(HWND window,UINT message,UINT_PTR timer,DWORD time)
{
    (void)message;(void)time;KillTimer(window,timer);
    HWND files=GetDlgItem(window,1120),drives=GetDlgItem(window,1137);
    LRESULT count=SendMessageW(files,LB_GETCOUNT,0,0),before=SendMessageW(files,LB_GETTOPINDEX,0,0);
    SendMessageW(files,WM_VSCROLL,SB_LINEDOWN,0);
    LRESULT after=SendMessageW(files,LB_GETTOPINDEX,0,0),selected=SendMessageW(drives,CB_GETCURSEL,0,0);
    SCROLLBARINFO bar={.cbSize=sizeof(bar)};BOOL bar_ok=GetScrollBarInfo(files,OBJID_VSCROLL,&bar);
    if(bar_ok){int x=(bar.rcScrollBar.left+bar.rcScrollBar.right)/2,y=bar.rcScrollBar.bottom-bar.dxyLineButton/2;
        LPARAM point=MAKELPARAM(x,y);SendMessageW(files,WM_NCLBUTTONDOWN,HTVSCROLL,point);
        POINT up={x,y};ScreenToClient(files,&up);SendMessageW(files,WM_LBUTTONUP,0,MAKELPARAM(up.x,up.y));}
    LRESULT clicked=SendMessageW(files,LB_GETTOPINDEX,0,0);
    SendMessageW(drives,CB_SHOWDROPDOWN,TRUE,0);LRESULT opened=SendMessageW(drives,CB_GETDROPPEDSTATE,0,0);
    SendMessageW(drives,CB_SHOWDROPDOWN,FALSE,0);LRESULT final=SendMessageW(drives,CB_GETCURSEL,0,0);
    HWND folders=GetDlgItem(window,1121);
    SetWindowPos(window,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
    DwmFlush();LRESULT folder_count=SendMessageW(folders,LB_GETCOUNT,0,0);
    LRESULT folder_before=SendMessageW(folders,LB_GETTOPINDEX,0,0);
    SCROLLBARINFO folder_bar={.cbSize=sizeof(folder_bar)};RECT rect;BOOL folder_bar_ok=GetScrollBarInfo(folders,OBJID_VSCROLL,&folder_bar) && GetWindowRect(folders,&rect);
    DWORD pixels[4096],after_pixels[4096],repaint_pixels[4096];
    POINT logical_start={folder_bar.rcScrollBar.left,folder_bar.rcScrollBar.bottom-folder_bar.dxyLineButton};
    POINT logical_end={folder_bar.rcScrollBar.right,folder_bar.rcScrollBar.bottom};
    POINT physical_start=logical_start,physical_end=logical_end;
    BOOL physical_ok=LogicalToPhysicalPointForPerMonitorDPI(folders,&physical_start) && LogicalToPhysicalPointForPerMonitorDPI(folders,&physical_end);
    int width=physical_end.x-physical_start.x,height=physical_end.y-physical_start.y;
    BOOL same_pixels=folder_bar_ok && physical_ok && width>0 && height>0 && width<=64 && height<=64;
    int left=physical_start.x,top=physical_start.y;
    POINT probe_point={(logical_start.x+logical_end.x)/2,(logical_start.y+logical_end.y)/2};HWND initial_hit=WindowFromPoint(probe_point);DWORD initial_pid=0;
    if(initial_hit)GetWindowThreadProcessId(initial_hit,&initial_pid);
    BOOL initial_own=initial_hit && initial_pid==GetCurrentProcessId() && GetAncestor(initial_hit,GA_ROOT)==GetAncestor(window,GA_ROOT);
    same_pixels=same_pixels && initial_own;
    if(same_pixels)same_pixels=screen_pixels(left,top,width,height,pixels);
    SendMessageW(folders,WM_VSCROLL,SB_LINEDOWN,0);LRESULT folder_after=SendMessageW(folders,LB_GETTOPINDEX,0,0);
    BOOL folder_capture=FALSE;
    if(folder_bar_ok){int x=(folder_bar.rcScrollBar.left+folder_bar.rcScrollBar.right)/2,y=folder_bar.rcScrollBar.bottom-folder_bar.dxyLineButton/2;
        LPARAM point=MAKELPARAM(x,y);SendMessageW(folders,WM_NCLBUTTONDOWN,HTVSCROLL,point);
        folder_capture=GetCapture()==folders;
        POINT up={x,y};ScreenToClient(folders,&up);SendMessageW(folders,WM_LBUTTONUP,0,MAKELPARAM(up.x,up.y));}
    printf("FOLDER_SCROLL_TRACK capture_on_press=%d\n",folder_capture);
    LRESULT folder_click=SendMessageW(folders,LB_GETTOPINDEX,0,0);
    SCROLLINFO setpos={.cbSize=sizeof(setpos),.fMask=SIF_POS,.nPos=3};SetScrollInfo(folders,SB_VERT,&setpos,TRUE);
    GdiFlush();DwmFlush();if(same_pixels)same_pixels=screen_pixels(left,top,width,height,after_pixels);
    /* Compare the settled bar against the same state after an explicit frame
     * repaint, not against a disabled pre-scroll arrow. */
    BOOL repaint_ok=RedrawWindow(folders,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW);DwmFlush();
    if(same_pixels)same_pixels=repaint_ok && screen_pixels(left,top,width,height,repaint_pixels);
    int differences=0;COLORREF prior=0,next=0;int diffx=-1,diffy=-1;
    if(same_pixels)for(int y=0;y<height;++y)for(int x=0;x<width;++x){COLORREF actual=repaint_pixels[y*width+x];if(after_pixels[y*width+x]!=actual){
        if(!differences){prior=after_pixels[y*width+x];next=actual;diffx=x;diffy=y;}++differences;}}
    if(differences)same_pixels=FALSE;
    HWND hit=WindowFromPoint(probe_point);DWORD hit_pid=0;if(hit)GetWindowThreadProcessId(hit,&hit_pid);
    BOOL own_hit=initial_own && hit && hit_pid==GetCurrentProcessId() && GetAncestor(hit,GA_ROOT)==GetAncestor(window,GA_ROOT);
    HDC screen=GetDC(NULL);COLORREF desktop=screen?GetPixel(screen,left,top):CLR_INVALID;
    printf("FOLDER_ARROW_DIFF pixels=%d first=%d,%d %08lx->%08lx screen=%08lx rect=%d,%d size=%dx%d visible=%d own_hit=%d hitpid=%lu\n",differences,diffx,diffy,(unsigned long)prior,(unsigned long)next,(unsigned long)desktop,left,top,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN),IsWindowVisible(folders),own_hit,hit_pid);if(screen)ReleaseDC(NULL,screen);
    printf("FOLDER_RIGHT_SCROLL count=%lld top=%lld->%lld->%lld arrow_unchanged=%d size=%dx%d\n",(long long)folder_count,(long long)folder_before,(long long)folder_after,(long long)folder_click,same_pixels,width,height);
    if(!own_hit)printf("FOLDER_RIGHT_VISUAL_SKIP reason=occluded\n");
    else if(same_pixels)printf("FOLDER_RIGHT_VISUAL_PASS settled_equals_repaint=1\n");
    controls_ok=count>=15 && before>=0 && after>before && bar_ok && clicked>after && opened && selected==final &&
        folder_count>=15 && folder_after>folder_before && folder_click>folder_after && (!own_hit || same_pixels);
    printf("FOLDER_CONTROLS count=%lld top=%lld->%lld click=%lld drop=%lld preserved=%d\n",(long long)count,(long long)before,(long long)after,(long long)clicked,(long long)opened,selected==final);
}
static BOOL CALLBACK inspect(HWND window,LPARAM ignored)
{
    (void)ignored;
    WCHAR text[128];GetWindowTextW(window,text,128);
    if(!IsWindowVisible(window))return TRUE;
    if(new_folder_mode && seen && window!=dialog && GetDlgItem(window,IDOK)){
        InterlockedExchange(&shell_seen,1);
        PostMessageW(window,WM_COMMAND,IDCANCEL,0);
        return FALSE;
    }
    if(lstrcmpW(text,L"Select Folder") || !GetDlgItem(window,1121))return TRUE;
    WCHAR file[MAX_PATH];
    if(swprintf(file,MAX_PATH,L"%ls\\__win16s_folder__",wanted)<0)return FALSE;
    WCHAR olddir[MAX_PATH]=L"",oldfile[MAX_PATH]=L"";GetDlgItemTextW(window,1088,olddir,MAX_PATH);GetDlgItemTextW(window,1152,oldfile,MAX_PATH);
    printf("OLD_DIALOG directory=%ls file=%ls current=%ls\n",olddir,oldfile,current);fflush(stdout);
    if(!same_mode)SetDlgItemTextW(window,1152,file);
    if(controls_mode)SetTimer(window,922,100,test_controls);
    dialog=window;InterlockedExchange(&seen,1);
    return FALSE;
}
static DWORD WINAPI observer(void *ignored)
{
    (void)ignored;
    for(unsigned i=0;i<400;++i){
        if(!seen || (new_folder_mode && !shell_seen))EnumThreadWindows(owner_thread,inspect,0);
        if(seen && IsWindow(dialog) && (!controls_mode || i>20) && i%10==0)PostMessageW(dialog,WM_COMMAND,new_folder_mode?0x5717:(cancel_mode?IDCANCEL:IDOK),0);
        if(seen && !IsWindow(dialog) && (!new_folder_mode || shell_seen))break;
        Sleep(50);
    }
    if(!seen || IsWindow(dialog)){fprintf(stderr,"FOLDER_OBSERVER_TIMEOUT seen=%ld alive=%d\n",seen,IsWindow(dialog));PostThreadMessageW(owner_thread,WM_QUIT,0,0);}
    return 0;
}
static int CALLBACK callback(HWND window,UINT message,LPARAM data,LPARAM context)
{
    if(context!=0x12345678)return 1;
    if(message==BFFM_INITIALIZED){
        InterlockedIncrement(&initialized);
        SendMessageW(window,BFFM_SETSELECTIONW,TRUE,(LPARAM)(controls_mode?wanted:initial));
    }else if(message==BFFM_SELCHANGED){
        InterlockedIncrement(&changed);current[0]=0;
        if(data)SHGetPathFromIDListW((PCIDLIST_ABSOLUTE)data,current);
        printf("SHELL_SELECTION %ls\n",current);fflush(stdout);
        SendMessageW(window,BFFM_ENABLEOK,0,TRUE);
    }
    return 0;
}
int wmain(int argc,WCHAR **argv)
{
    if(argc<2 || argc>3)return 2;
    cancel_mode=argc==3 && !lstrcmpW(argv[2],L"cancel");
    new_folder_mode=argc==3 && !lstrcmpW(argv[2],L"newfolder");
    controls_mode=argc==3 && !lstrcmpW(argv[2],L"controls");
    same_mode=controls_mode || (argc==3 && !lstrcmpW(argv[2],L"same"));
    if(!GetTempPathW(MAX_PATH,initial))return 3;
    if(swprintf(wanted,MAX_PATH,L"%lswin16s-%lu-%lu",initial,GetCurrentProcessId(),GetTickCount())<0)return 3;
    if(!CreateDirectoryW(wanted,NULL))return 4;
    if(controls_mode)for(int i=0;i<15;++i){
        WCHAR file[MAX_PATH];if(swprintf(file,MAX_PATH,L"%ls\\File%02d.txt",wanted,i)<0)return 4;
        HANDLE h=CreateFileW(file,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
        if(h==INVALID_HANDLE_VALUE)return 4;
        CloseHandle(h);
        if(swprintf(file,MAX_PATH,L"%ls\\Folder%02d",wanted,i)<0 || !CreateDirectoryW(file,NULL))return 4;
    }
    HRESULT hr=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(hr))return 5;
    owner_thread=GetCurrentThreadId();
    HANDLE thread=CreateThread(NULL,0,observer,NULL,0,NULL);if(!thread)return 6;
    PIDLIST_ABSOLUTE result;
    if(argv[1][0]==L'a'){
        BROWSEINFOA info={0};info.ulFlags=BIF_RETURNONLYFSDIRS|BIF_USENEWUI|BIF_VALIDATE;
        info.lpfn=callback;info.lParam=0x12345678;result=SHBrowseForFolderA(&info);
    }else{
        BROWSEINFOW info={0};info.ulFlags=BIF_RETURNONLYFSDIRS|BIF_USENEWUI|BIF_VALIDATE;
        info.lpfn=callback;info.lParam=0x12345678;result=SHBrowseForFolderW(&info);
    }
    WCHAR returned[MAX_PATH]=L"";
    BOOL ok=result && SHGetPathFromIDListW(result,returned);
    if(result)CoTaskMemFree(result);
    WaitForSingleObject(thread,12000);CloseHandle(thread);
    WCHAR expected[MAX_PATH];lstrcpynW(expected,(same_mode&&!controls_mode)?initial:wanted,MAX_PATH);
    size_t len=wcslen(expected);if(len>3 && expected[len-1]==L'\\')expected[len-1]=0;
    BOOL same=ok&&!lstrcmpiW(expected,returned);
    printf("FOLDER_DIALOG bits=%u ansi=%d seen=%ld init=%ld changed=%ld ok=%d same=%d cancel=%d new=%d shell=%ld wanted=%ls returned=%ls last=%ls\n",
        (unsigned)(sizeof(void*)*8),argv[1][0]==L'a',seen,initialized,changed,ok,same,cancel_mode,new_folder_mode,shell_seen,wanted,returned,current);
    CoUninitialize();
    if(controls_mode)for(int i=0;i<15;++i){WCHAR file[MAX_PATH];if(swprintf(file,MAX_PATH,L"%ls\\File%02d.txt",wanted,i)>=0)DeleteFileW(file);
        if(swprintf(file,MAX_PATH,L"%ls\\Folder%02d",wanted,i)>=0)RemoveDirectoryW(file);}
    RemoveDirectoryW(wanted);
    return seen && initialized==1 && changed && (!controls_mode || controls_ok) && ((cancel_mode || new_folder_mode)?(!result && (!new_folder_mode || shell_seen)):same)?0:1;
}
