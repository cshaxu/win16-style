#include <windows.h>
#include <oleacc.h>
#include <stdio.h>
#include <stdint.h>

static LRESULT CALLBACK procedure(HWND w,UINT message,WPARAM wp,LPARAM lp)
{return DefWindowProcW(w,message,wp,lp);}

static BOOL blue_caption(HWND w)
{
    TITLEBARINFOEX info={.cbSize=sizeof(info)};RECT outer;
    SendMessageW(w,WM_GETTITLEBARINFOEX,0,(LPARAM)&info);
    if(!GetWindowRect(w,&outer))return FALSE;
    HDC dc=GetWindowDC(w);if(!dc)return FALSE;
    COLORREF pixel=GetPixel(dc,info.rcTitleBar.left-outer.left+40,info.rcTitleBar.top-outer.top+2);
    ReleaseDC(w,dc);
    return pixel!=CLR_INVALID && GetBValue(pixel)>GetRValue(pixel);
}

static void pump_messages(void)
{
    MSG message;
    while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}
}

static BOOL click_caption(HWND w,unsigned index,WPARAM hit)
{
    TITLEBARINFOEX info={.cbSize=sizeof(info)};
    SendMessageW(w,WM_GETTITLEBARINFOEX,0,(LPARAM)&info);
    POINT point={(info.rgrect[index].left+info.rgrect[index].right)/2,
                 (info.rgrect[index].top+info.rgrect[index].bottom)/2};
    SendMessageW(w,WM_NCLBUTTONDOWN,hit,MAKELPARAM(point.x,point.y));
    if(GetCapture()!=w)return FALSE;
    ScreenToClient(w,&point);
    SendMessageW(w,WM_LBUTTONUP,0,MAKELPARAM(point.x,point.y));
    pump_messages();return TRUE;
}

static uint64_t button_signature(HWND w,unsigned index)
{
    TITLEBARINFOEX info={.cbSize=sizeof(info)};RECT outer;
    SendMessageW(w,WM_GETTITLEBARINFOEX,0,(LPARAM)&info);
    if(index>CCHILDREN_TITLEBAR || !GetWindowRect(w,&outer))return 0;
    RECT r=info.rgrect[index];OffsetRect(&r,-outer.left,-outer.top);
    HDC dc=GetWindowDC(w);if(!dc || r.right<=r.left || r.bottom<=r.top){if(dc)ReleaseDC(w,dc);return 0;}
    uint64_t value=14695981039346656037ULL;
    for(int y=r.top;y<r.bottom;++y)for(int x=r.left;x<r.right;++x){value^=GetPixel(dc,x,y);value*=1099511628211ULL;}
    ReleaseDC(w,dc);return value;
}

typedef struct { HWND window;BOOL input,pressed,cancelled,maximized,restored;unsigned gray,native; } physical_probe;
static BOOL physical_click(POINT point)
{
    INPUT event={.type=INPUT_MOUSE};
    if(!SetCursorPos(point.x,point.y))return FALSE;
    event.mi.dwFlags=MOUSEEVENTF_LEFTDOWN;BOOL down=SendInput(1,&event,sizeof(event))==1;
    Sleep(100);event.mi.dwFlags=MOUSEEVENTF_LEFTUP;BOOL up=SendInput(1,&event,sizeof(event))==1;
    return down && up;
}
static DWORD WINAPI physical_thread(void *parameter)
{
    physical_probe *probe=parameter;HWND w=probe->window;
    TITLEBARINFOEX info={.cbSize=sizeof(info)};RECT outer;
    SendMessageW(w,WM_GETTITLEBARINFOEX,0,(LPARAM)&info);GetWindowRect(w,&outer);
    RECT button=info.rgrect[3];POINT center={(button.left+button.right)/2,(button.top+button.bottom)/2};
    INPUT event={.type=INPUT_MOUSE};event.mi.dwFlags=MOUSEEVENTF_LEFTDOWN;
    Sleep(200);probe->input=SetCursorPos(center.x,center.y) && SendInput(1,&event,sizeof(event))==1;
    Sleep(250);
    GUITHREADINFO thread={.cbSize=sizeof(thread)};
    probe->pressed=GetGUIThreadInfo(GetWindowThreadProcessId(w,NULL),&thread) && thread.hwndCapture==w;
    TITLEBARINFOEX down={.cbSize=sizeof(down)};SendMessageW(w,WM_GETTITLEBARINFOEX,0,(LPARAM)&down);
    probe->pressed=probe->pressed && (down.rgstate[3]&STATE_SYSTEM_PRESSED)!=0;
    HDC dc=GetWindowDC(w);
    if(dc){OffsetRect(&button,-outer.left,-outer.top);
        for(int y=button.top;y<button.bottom;++y)for(int x=button.left;x<button.right;++x){
            COLORREF pixel=GetPixel(dc,x,y);
            if(pixel==RGB(192,192,192))++probe->gray;
            if(pixel==GetSysColor(COLOR_3DFACE))++probe->native;
        }
        ReleaseDC(w,dc);}
    SetCursorPos(outer.left+30,outer.top+100);Sleep(150);
    event.mi.dwFlags=MOUSEEVENTF_LEFTUP;
    BOOL released=SendInput(1,&event,sizeof(event))==1;
    probe->input=probe->input && released;
    Sleep(200);probe->cancelled=IsWindow(w) && !IsZoomed(w);
    probe->input=physical_click(center) && probe->input;Sleep(200);
    probe->maximized=IsWindow(w) && IsZoomed(w);
    if(probe->maximized){
        TITLEBARINFOEX zoom={.cbSize=sizeof(zoom)};SendMessageW(w,WM_GETTITLEBARINFOEX,0,(LPARAM)&zoom);
        POINT restore={(zoom.rgrect[3].left+zoom.rgrect[3].right)/2,(zoom.rgrect[3].top+zoom.rgrect[3].bottom)/2};
        probe->input=physical_click(restore) && probe->input;Sleep(200);
        probe->restored=IsWindow(w) && !IsZoomed(w);
    }
    PostMessageW(w,WM_APP+1,0,0);return 0;
}

int wmain(int argc,WCHAR **argv)
{
    HINSTANCE instance=GetModuleHandleW(NULL);
    BOOL no_icon=argc==2 && !lstrcmpW(argv[1],L"--no-icon");
    WNDCLASSW wc={.lpfnWndProc=procedure,.hInstance=instance,.hIcon=no_icon?NULL:LoadIconW(NULL,MAKEINTRESOURCEW(32515)),.lpszClassName=L"Win2kFrameFixture"};
    if(!RegisterClassW(&wc))return 1;
    if(argc==2 && !lstrcmpW(argv[1],L"--buttons")){
        HWND w=CreateWindowExW(0,wc.lpszClassName,L"Caption commands",WS_OVERLAPPEDWINDOW,
                                80,80,420,260,NULL,NULL,instance,NULL);
        if(!w)return 2;
        ShowWindow(w,SW_SHOW);UpdateWindow(w);
        BOOL max=click_caption(w,3,HTMAXBUTTON) && IsZoomed(w);
        BOOL restore=max && click_caption(w,3,HTMAXBUTTON) && !IsZoomed(w);
        BOOL min=restore && click_caption(w,2,HTMINBUTTON) && IsIconic(w);
        if(min){ShowWindow(w,SW_RESTORE);pump_messages();}
        BOOL close=min && !IsIconic(w) && click_caption(w,5,HTCLOSE) && !IsWindow(w);
        if(IsWindow(w))DestroyWindow(w);
        printf("WIN2K COMMANDS bits=%u max=%d restore=%d min=%d close=%d\n",
               (unsigned)(sizeof(void*)*8),max,restore,min,close);
        return max && restore && min && close?0:3;
    }
    if(argc==2 && !lstrcmpW(argv[1],L"--physical")){
        HWND w=CreateWindowExW(0,wc.lpszClassName,L"Caption mouse test",WS_OVERLAPPEDWINDOW,
                                80,80,420,260,NULL,NULL,instance,NULL);
        if(!w)return 2;
        ShowWindow(w,SW_SHOW);UpdateWindow(w);SetForegroundWindow(w);
        physical_probe probe={.window=w};HANDLE worker=CreateThread(NULL,0,physical_thread,&probe,0,NULL);
        if(!worker){DestroyWindow(w);return 3;}
        MSG message;
        while(GetMessageW(&message,NULL,0,0)>0){
            if(message.message==WM_APP+1)break;
            TranslateMessage(&message);DispatchMessageW(&message);
        }
        WaitForSingleObject(worker,3000);CloseHandle(worker);DestroyWindow(w);
        printf("WIN2K PHYSICAL bits=%u input=%d pressed=%d gray=%u native=%u cancelled=%d max=%d restore=%d\n",
               (unsigned)(sizeof(void*)*8),probe.input,probe.pressed,probe.gray,probe.native,probe.cancelled,probe.maximized,probe.restored);
        return probe.input && probe.pressed && probe.gray>200 && probe.native<20 && probe.cancelled && probe.maximized && probe.restored?0:3;
    }
    if(no_icon){
        HWND w=CreateWindowExW(0,wc.lpszClassName,L"No icon",WS_OVERLAPPEDWINDOW,
                                80,80,420,260,NULL,NULL,instance,NULL);
        if(!w)return 2;
        ShowWindow(w,SW_SHOW);UpdateWindow(w);SendMessageW(w,WM_NCACTIVATE,TRUE,0);
        TITLEBARINFOEX info={.cbSize=sizeof(info)};RECT outer;
        SendMessageW(w,WM_GETTITLEBARINFOEX,0,(LPARAM)&info);GetWindowRect(w,&outer);
        HDC dc=GetWindowDC(w);COLORREF before[16*16],title_before[16*16];unsigned drawn=0,changed=0,title_changed=0;
        int left=info.rcTitleBar.left-outer.left-(info.rcTitleBar.bottom-info.rcTitleBar.top)+3;
        int top=info.rcTitleBar.top-outer.top+3;
        int title_left=info.rcTitleBar.left-outer.left+2;
        if(dc){
            COLORREF background=GetPixel(dc,left,top);
            for(int y=0;y<16;++y)for(int x=0;x<16;++x){
                COLORREF pixel=GetPixel(dc,left+x,top+y);
                before[y*16+x]=pixel;
                title_before[y*16+x]=GetPixel(dc,title_left+x,top+y);
                if(pixel!=CLR_INVALID && pixel!=background)++drawn;
            }
            SendMessageW(w,WM_SETICON,ICON_BIG,(LPARAM)LoadIconW(NULL,MAKEINTRESOURCEW(32513)));
            for(int y=0;y<16;++y)for(int x=0;x<16;++x){
                if(before[y*16+x]!=GetPixel(dc,left+x,top+y))++changed;
                if(title_before[y*16+x]!=GetPixel(dc,title_left+x,top+y))++title_changed;
            }
            ReleaseDC(w,dc);
        }
        DestroyWindow(w);
        printf("WIN2K DEFAULT ICON bits=%u drawn=%u changed=%u title_changed=%u\n",(unsigned)(sizeof(void*)*8),drawn,changed,title_changed);
        return drawn>0 && changed>0 && title_changed==0?0:3;
    }
    if(argc==2 && !lstrcmpW(argv[1],L"--tool")){
        HWND tool=CreateWindowExW(WS_EX_TOOLWINDOW,wc.lpszClassName,L"Tool window",WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU,
                                  100,100,320,180,NULL,NULL,instance,NULL);
        BOOL tool_ok=FALSE;
        if(tool){
            ShowWindow(tool,SW_SHOW);UpdateWindow(tool);SendMessageW(tool,WM_NCACTIVATE,TRUE,0);
            tool_ok=GetPropW(tool,L"Win2kStyle.Frame")!=NULL && blue_caption(tool);
            DestroyWindow(tool);
        }
        printf("WIN2K TOOL bits=%u styled=%d\n",(unsigned)(sizeof(void*)*8),tool_ok);
        return tool_ok?0:3;
    }
    if(argc!=1)return 2;
    HWND w=CreateWindowExW(0,wc.lpszClassName,L"Classic frame",WS_OVERLAPPEDWINDOW,
                            80,80,420,260,NULL,NULL,instance,NULL);
    if(!w)return 2;
    BOOL attached=GetPropW(w,L"Win2kStyle.Frame")!=NULL;
    ShowWindow(w,SW_SHOW);UpdateWindow(w);
    RedrawWindow(w,NULL,NULL,RDW_FRAME|RDW_INVALIDATE|RDW_UPDATENOW);
    TITLEBARINFOEX info={.cbSize=sizeof(info)};
    SendMessageW(w,WM_GETTITLEBARINFOEX,0,(LPARAM)&info);
    RECT window;GetWindowRect(w,&window);
    HDC dc=GetWindowDC(w);
    int x=info.rcTitleBar.left-window.left+40;
    int y=info.rcTitleBar.top-window.top+2;
    int end=info.rcTitleBar.right-window.left;
    for(unsigned i=2;i<CCHILDREN_TITLEBAR+1;++i){
        RECT r=info.rgrect[i];OffsetRect(&r,-window.left,-window.top);
        if(r.right>r.left && r.left>x && r.left<end && !(info.rgstate[i]&STATE_SYSTEM_INVISIBLE))end=r.left;
    }
    SendMessageW(w,WM_NCACTIVATE,TRUE,0);
    COLORREF pixel=dc?GetPixel(dc,x,y):CLR_INVALID;
    COLORREF pixel2=dc?GetPixel(dc,x+80,y):CLR_INVALID;
    COLORREF endpoint=dc?GetPixel(dc,end-3,y):CLR_INVALID;
    unsigned classic_button_pixels=0;
    COLORREF close_before[64*64];unsigned close_changed=0;BOOL close_disabled=FALSE;
    if(dc){
        RECT button=info.rgrect[5];OffsetRect(&button,-window.left,-window.top);
        for(int by=button.top;by<button.bottom;++by)for(int bx=button.left;bx<button.right;++bx)
            if(GetPixel(dc,bx,by)==RGB(192,192,192))++classic_button_pixels;
        int bw=button.right-button.left,bh=button.bottom-button.top;
        HMENU system=GetSystemMenu(w,FALSE);
        if(system && bw>0 && bw<=64 && bh>0 && bh<=64){
            for(int by=0;by<bh;++by)for(int bx=0;bx<bw;++bx)
                close_before[by*64+bx]=GetPixel(dc,button.left+bx,button.top+by);
            EnableMenuItem(system,SC_CLOSE,MF_BYCOMMAND|MF_GRAYED);
            DrawMenuBar(w);
            TITLEBARINFOEX disabled={.cbSize=sizeof(disabled)};
            SendMessageW(w,WM_GETTITLEBARINFOEX,0,(LPARAM)&disabled);
            close_disabled=(disabled.rgstate[5]&STATE_SYSTEM_UNAVAILABLE)!=0;
            for(int by=0;by<bh;++by)for(int bx=0;bx<bw;++bx)
                if(close_before[by*64+bx]!=GetPixel(dc,button.left+bx,button.top+by))++close_changed;
            EnableMenuItem(system,SC_CLOSE,MF_BYCOMMAND|MF_ENABLED);
            DrawMenuBar(w);
        }
    }
    unsigned icon_pixels=0;
    COLORREF icon_before[16*12];
    if(dc){
        for(int iy=info.rcTitleBar.top-window.top+2;iy<info.rcTitleBar.bottom-window.top-2;++iy)
            for(int ix=info.rcTitleBar.left-window.left-(info.rcTitleBar.bottom-info.rcTitleBar.top)+4;ix<info.rcTitleBar.left-window.left-(info.rcTitleBar.bottom-info.rcTitleBar.top)+20;++ix){
                COLORREF c=GetPixel(dc,ix,iy);
                if(c!=CLR_INVALID && GetRValue(c)>=GetBValue(c))++icon_pixels;
            }
    }
    unsigned icon_changed=0;
    if(dc){
        int left=info.rcTitleBar.left-window.left-(info.rcTitleBar.bottom-info.rcTitleBar.top)+4,top=info.rcTitleBar.top-window.top+4;
        for(int iy=0;iy<12;++iy)for(int ix=0;ix<16;++ix)icon_before[iy*16+ix]=GetPixel(dc,left+ix,top+iy);
        SendMessageW(w,WM_SETICON,ICON_SMALL,(LPARAM)LoadIconW(NULL,MAKEINTRESOURCEW(32513)));
        for(int iy=0;iy<12;++iy)for(int ix=0;ix<16;++ix)
            if(icon_before[iy*16+ix]!=GetPixel(dc,left+ix,top+iy))++icon_changed;
    }
    COLORREF before[100*12];unsigned changed=0;
    if(dc){
        for(int iy=0;iy<12;++iy)for(int ix=0;ix<100;++ix)
            before[iy*100+ix]=GetPixel(dc,x+ix,y+iy+2);
        SetWindowTextW(w,L"WWWWWWWWWW");
        for(int iy=0;iy<12;++iy)for(int ix=0;ix<100;++ix)
            if(before[iy*100+ix]!=GetPixel(dc,x+ix,y+iy+2))++changed;
    }
    SendMessageW(w,WM_NCACTIVATE,FALSE,0);
    COLORREF inactive=dc?GetPixel(dc,x,y):CLR_INVALID;
    COLORREF inactive_end=dc?GetPixel(dc,end-3,y):CLR_INVALID;
    if(dc)ReleaseDC(w,dc);
    LRESULT hit=SendMessageW(w,WM_NCHITTEST,0,MAKELPARAM(info.rcTitleBar.left+50,(info.rcTitleBar.top+info.rcTitleBar.bottom)/2));
    SendMessageW(w,WM_NCACTIVATE,TRUE,0);
    BOOL resized=SetWindowPos(w,NULL,0,0,520,320,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE) && blue_caption(w);
    uint64_t normal_button=button_signature(w,3);
    SendMessageW(w,WM_SYSCOMMAND,SC_MAXIMIZE,0);
    uint64_t zoom_button=button_signature(w,3);
    BOOL maximized=IsZoomed(w) && blue_caption(w) && normal_button && zoom_button && normal_button!=zoom_button;
    SendMessageW(w,WM_SYSCOMMAND,SC_RESTORE,0);
    uint64_t restore_button=button_signature(w,3);
    BOOL restored=!IsZoomed(w) && GetPropW(w,L"Win2kStyle.Frame")!=NULL && blue_caption(w) && restore_button==normal_button;
    printf("WIN2K FRAME bits=%u attached=%d title=%ldx%ld pixels=%06lx/%06lx end=%06lx inactive=%06lx/%06lx button_gray=%u close_disabled=%d close_changed=%u icon_pixels=%u icon_changed=%u text_pixels=%u hit=%lld\n",
        (unsigned)(sizeof(void*)*8),attached,info.rcTitleBar.right-info.rcTitleBar.left,
        info.rcTitleBar.bottom-info.rcTitleBar.top,(unsigned long)pixel,(unsigned long)pixel2,(unsigned long)endpoint,(unsigned long)inactive,(unsigned long)inactive_end,classic_button_pixels,close_disabled,close_changed,icon_pixels,icon_changed,changed,(long long)hit);
    SendMessageW(w,WM_SYSCOMMAND,SC_CLOSE,0);
    BOOL closed=!IsWindow(w);if(!closed)DestroyWindow(w);
    return attached && pixel!=CLR_INVALID && pixel2!=CLR_INVALID && pixel!=pixel2 && inactive!=CLR_INVALID &&
           GetBValue(pixel)>GetRValue(pixel) && GetRValue(endpoint)>=150 && GetGValue(endpoint)>=180 && GetBValue(endpoint)>=220 && classic_button_pixels>20 && close_disabled && close_changed>0 &&
           GetRValue(inactive)==GetGValue(inactive) && GetGValue(inactive)==GetBValue(inactive) && GetRValue(inactive_end)>=175 && GetRValue(inactive_end)<=181 && icon_pixels>0 && icon_changed>0 && changed>0 &&
           hit==HTCAPTION && resized && maximized && restored && closed?0:3;
}
