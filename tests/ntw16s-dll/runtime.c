/* Owned private desktop, actual DLL imports, no cursor/input-desktop injection. */
#include "ntw16s-dll/ntw16s.h"
#include <commctrl.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include "isolated_process.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"CORE FAIL line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x);ExitProcess(1);}}while(0)
static HWND thread_window;
static unsigned commands;
static BOOL styled=TRUE;

static LRESULT CALLBACK owner(HWND w,UINT message,WPARAM a,LPARAM b)
{
    if(message==WM_APP+42)return 4242;
    if(message==WM_COMMAND){++commands;return 0;}
    if(message==WM_ERASEBKGND){RECT r;GetClientRect(w,&r);
        FillRect((HDC)a,&r,(HBRUSH)(COLOR_BTNFACE+1));return 1;}
    return DefWindowProcW(w,message,a,b);
}

static DWORD WINAPI wrong_thread(void *unused)
{
    (void)unused;
    CHECK(GetWindowThreadProcessId(thread_window,NULL)!=GetCurrentThreadId());
    if(styled){
        CHECK(!Ntw16sWindowRemove(thread_window) && GetLastError()==ERROR_INVALID_THREAD_ID);
        CHECK(!win16_style_install(thread_window,NULL) && GetLastError()==ERROR_INVALID_THREAD_ID);}
    return 0;
}

static void capture(HWND window)
{
    WCHAR path[MAX_PATH];DWORD length=GetEnvironmentVariableW(L"WIN16STYLE_CAPTURE_DIR",path,MAX_PATH);
    RECT r;BITMAPINFO info={0};BITMAPFILEHEADER header={0};void *bits;HDC dc;
    HBITMAP bitmap,old;HANDLE file;DWORD bytes,written;
    CHECK(length && length<MAX_PATH-20);
    wcscat_s(path,MAX_PATH,styled?L"\\core-styled.bmp":L"\\core-native.bmp");CHECK(GetWindowRect(window,&r));
    dc=CreateCompatibleDC(NULL);CHECK(dc);
    info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth=r.right-r.left;info.bmiHeader.biHeight=-(r.bottom-r.top);
    info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
    bytes=(DWORD)info.bmiHeader.biWidth*(DWORD)(-info.bmiHeader.biHeight)*4;
    bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&bits,NULL,0);CHECK(bitmap);
    old=(HBITMAP)SelectObject(dc,bitmap);CHECK(old);
    PatBlt(dc,0,0,r.right-r.left,r.bottom-r.top,WHITENESS);
    SendMessageW(window,WM_PRINT,(WPARAM)dc,PRF_NONCLIENT|PRF_CLIENT|PRF_CHILDREN|PRF_ERASEBKGND);GdiFlush();
    header.bfType=0x4d42;header.bfOffBits=sizeof(header)+sizeof(info.bmiHeader);header.bfSize=header.bfOffBits+bytes;
    file=CreateFileW(path,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);CHECK(file!=INVALID_HANDLE_VALUE);
    CHECK(WriteFile(file,&header,sizeof(header),&written,NULL) && written==sizeof(header));
    CHECK(WriteFile(file,&info.bmiHeader,sizeof(info.bmiHeader),&written,NULL) && written==sizeof(info.bmiHeader));
    CHECK(WriteFile(file,bits,bytes,&written,NULL) && written==bytes && CloseHandle(file));
    SelectObject(dc,old);CHECK(DeleteObject(bitmap) && DeleteDC(dc));
    wprintf(L"CORE WM_PRINT capture=%ls\n",path);
}

static void menu_scales(void)
{
    UINT dpi;HDC screen=GetDC(NULL),dc;HBITMAP bitmap,old;
    HFONT font=win16_style_font_acquire();win16_style_palette palette;
    CHECK(screen);dc=CreateCompatibleDC(screen);bitmap=CreateCompatibleBitmap(screen,400,100);ReleaseDC(NULL,screen);
    CHECK(dc && bitmap && font);old=(HBITMAP)SelectObject(dc,bitmap);win16_style_default_palette(&palette);
    for(dpi=96;dpi<=192;dpi+=48){RECT bounds={0,0,360,72};SIZE size;
        CHECK(win16_style_menu_measure(dc,font,dpi,L"Checked\tCtrl+C",WIN16_STYLE_MENU_ROW,&size));
        CHECK(size.cx>0 && size.cy>=20);
        CHECK(win16_style_menu_text(dc,&bounds,font,dpi,&palette,L"Checked\tCtrl+C",ODS_CHECKED,WIN16_STYLE_MENU_ROW,TRUE));
        CHECK(win16_style_menu_text(dc,&bounds,font,dpi,&palette,L"Disabled",ODS_DISABLED|ODS_CHECKED,WIN16_STYLE_MENU_ROW,TRUE));}
    SelectObject(dc,old);CHECK(DeleteObject(bitmap) && DeleteDC(dc));win16_style_font_release(font);
}

static void round_trip(BOOL observe)
{
    WNDCLASSW cls={0};HMENU menu=CreateMenu(),popup=CreatePopupMenu();HWND root,children[9];
    ntw16s_creation_scope *scope=NULL;ntw16s_creation_result result={0};HFONT font=NULL,second=NULL;
    HANDLE thread;DWORD exit_code;SCROLLINFO scroll={sizeof(scroll),SIF_ALL,0,100,10,35,0};
    CHECK(menu && popup && AppendMenuW(popup,MF_STRING,101,L"Open") && AppendMenuW(menu,MF_POPUP,(UINT_PTR)popup,L"File"));
    cls.lpfnWndProc=owner;cls.lpszClassName=L"Win16StyleCore";cls.hInstance=GetModuleHandleW(NULL);
    cls.hbrBackground=(HBRUSH)(COLOR_BTNFACE+1);
    CHECK(RegisterClassW(&cls) || GetLastError()==ERROR_CLASS_ALREADY_EXISTS);
    if(styled){scope=Ntw16sCreationBegin(TRUE);CHECK(scope);}
    root=CreateWindowW(cls.lpszClassName,L"Win16 Style - standalone core",WS_OVERLAPPEDWINDOW,
        0,0,850,540,NULL,menu,cls.hInstance,NULL);CHECK(root);
    children[0]=CreateWindowW(L"STATIC",L"Native application content",WS_CHILD|WS_VISIBLE,20,20,600,30,root,NULL,cls.hInstance,NULL);
    children[1]=CreateWindowW(L"BUTTON",L"Checked",WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,20,65,200,30,root,(HMENU)201,cls.hInstance,NULL);
    children[2]=CreateWindowW(L"BUTTON",L"Radio",WS_CHILD|WS_VISIBLE|BS_AUTORADIOBUTTON,240,65,180,30,root,(HMENU)202,cls.hInstance,NULL);
    children[3]=CreateWindowW(L"BUTTON",L"Default action",WS_CHILD|WS_VISIBLE|BS_DEFPUSHBUTTON,470,60,240,42,root,(HMENU)203,cls.hInstance,NULL);
    children[4]=CreateWindowW(L"EDIT",L"Native editable text\r\nSecond line",WS_CHILD|WS_VISIBLE|WS_BORDER|ES_MULTILINE,20,120,380,110,root,(HMENU)204,cls.hInstance,NULL);
    children[5]=CreateWindowW(L"LISTBOX",NULL,WS_CHILD|WS_VISIBLE|WS_BORDER|LBS_NOINTEGRALHEIGHT,440,120,280,110,root,(HMENU)205,cls.hInstance,NULL);
    children[6]=CreateWindowW(L"COMBOBOX",NULL,WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST,20,270,300,160,root,(HMENU)206,cls.hInstance,NULL);
    children[7]=CreateWindowW(L"SCROLLBAR",NULL,WS_CHILD|WS_VISIBLE|SBS_HORZ,440,280,280,24,root,(HMENU)207,cls.hInstance,NULL);
    children[8]=CreateWindowW(L"SCROLLBAR",NULL,WS_CHILD|WS_VISIBLE|SBS_VERT,750,120,24,200,root,(HMENU)208,cls.hInstance,NULL);
    if(styled){CHECK(Ntw16sCreationEnd(scope,&result) && !result.failure);CHECK(Ntw16sWindowAttached(root));}
    {unsigned i;for(i=0;i<9;++i){CHECK(children[i]);if(styled)CHECK(Ntw16sWindowAttached(children[i]));}}
    CHECK(SendMessageW(root,WM_APP+42,0,0)==4242);
    if(styled)CHECK(!win16_style_install(root,NULL) && GetLastError()==ERROR_ALREADY_EXISTS);
    SendMessageW(children[1],BM_SETCHECK,BST_CHECKED,0);CHECK(SendMessageW(children[1],BM_GETCHECK,0,0)==BST_CHECKED);
    SendMessageW(children[2],BM_SETCHECK,BST_CHECKED,0);
    SendMessageW(children[5],LB_ADDSTRING,0,(LPARAM)L"First native string");
    SendMessageW(children[5],LB_ADDSTRING,0,(LPARAM)L"Selected string");
    CHECK(SendMessageW(children[5],LB_SETCURSEL,1,0)!=LB_ERR && SendMessageW(children[5],LB_GETCURSEL,0,0)==1);
    SendMessageW(children[6],CB_ADDSTRING,0,(LPARAM)L"Native selection");
    CHECK(SendMessageW(children[6],CB_SETCURSEL,0,0)!=CB_ERR && SendMessageW(children[6],CB_GETCURSEL,0,0)==0);
    SendMessageW(children[4],EM_SETSEL,2,8);CHECK(LOWORD(SendMessageW(children[4],EM_GETSEL,0,0))==2);
    SendMessageW(children[4],EM_REPLACESEL,TRUE,(LPARAM)L"changed");CHECK(SendMessageW(children[4],EM_CANUNDO,0,0));
    CHECK(SendMessageW(children[4],EM_UNDO,0,0));
    SetScrollInfo(children[7],SB_CTL,&scroll,TRUE);SetScrollInfo(children[8],SB_CTL,&scroll,TRUE);
    CHECK(GetScrollPos(children[7],SB_CTL)==35 && GetScrollPos(children[8],SB_CTL)==35);
    if(styled){font=win16_style_font_acquire();second=win16_style_font_acquire();CHECK(font && second==font);
    {HDC dc=CreateCompatibleDC(NULL);HFONT old=(HFONT)SelectObject(dc,font);TEXTMETRICW metric;
        CHECK(GetTextMetricsW(dc,&metric) && metric.tmHeight==20);SelectObject(dc,old);DeleteDC(dc);}}
    thread_window=root;thread=CreateThread(NULL,0,wrong_thread,NULL,0,NULL);CHECK(thread);
    CHECK(WaitForSingleObject(thread,5000)==WAIT_OBJECT_0 && GetExitCodeThread(thread,&exit_code) && exit_code==0);CloseHandle(thread);
    {unsigned before=commands;SendMessageW(children[3],BM_CLICK,0,0);CHECK(commands==before+1);}
    ShowWindow(root,SW_SHOWNOACTIVATE);UpdateWindow(root);
    if(observe){printf("CORE families=6 creation installed=%lu unsupported=%lu dpi=%u\n",result.installed,result.unsupported,GetDpiForWindow(root));capture(root);}
    if(styled){CHECK(Ntw16sWindowRemove(root) && !Ntw16sWindowAttached(root));
        CHECK(Ntw16sWindowAttached(children[1]));
        CHECK(Ntw16sWindowRemove(children[1]) && !Ntw16sWindowAttached(children[1]));}
    CHECK(DestroyWindow(root) && !IsWindow(root));
    {unsigned i;for(i=0;i<9;++i)CHECK(!IsWindow(children[i]));}
    if(styled){LOGFONTW deleted;
        win16_style_font_release(second);win16_style_font_release(font);
        CHECK(GetObjectW(font,sizeof(deleted),&deleted)==0);}
}

static int runtime_worker(int argc,WCHAR **argv)
{
    DWORD before_gdi,after_gdi,before_user,after_user;unsigned i;
    CHECK(argc==1 || (argc==2 && !wcscmp(argv[1],L"--native")));styled=argc==1;
    CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));
    CHECK(Ntw16sGetAbiVersion()==3);
    if(styled)menu_scales();
    /* Native USER/GDI lazily allocate process caches during early creation.
     * Warm the same full workload, then assert no per-round growth, as well
     * as direct destruction of every HWND and the final leased HFONT. */
    round_trip(TRUE);round_trip(FALSE);round_trip(FALSE);GdiFlush();
    before_gdi=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);before_user=GetGuiResources(GetCurrentProcess(),GR_USEROBJECTS);
    for(i=0;i<20;++i){round_trip(FALSE);
        if(styled){CHECK(GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS)<=before_gdi);
            CHECK(GetGuiResources(GetCurrentProcess(),GR_USEROBJECTS)<=before_user);}}
    GdiFlush();after_gdi=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);after_user=GetGuiResources(GetCurrentProcess(),GR_USEROBJECTS);
    fprintf(stderr,"CORE runtime GDI=%lu/%lu USER=%lu/%lu rounds=20\n",before_gdi,after_gdi,before_user,after_user);
    if(styled)CHECK(after_gdi<=before_gdi && after_user<=before_user);
    printf("CORE RUNTIME PASS styled=%d actual DLL, six families, native state/notifications, owner thread, cleanup\n",styled);return 0;
}

int wmain(int argc,WCHAR **argv)
{return isolated_process(argc,argv,runtime_worker);}
