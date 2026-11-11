#include <windows.h>
#include <oleacc.h>
#include <stdio.h>

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

int wmain(int argc,WCHAR **argv)
{
    HINSTANCE instance=GetModuleHandleW(NULL);
    BOOL no_icon=argc==2 && !lstrcmpW(argv[1],L"--no-icon");
    WNDCLASSW wc={.lpfnWndProc=procedure,.hInstance=instance,.hIcon=no_icon?NULL:LoadIconW(NULL,MAKEINTRESOURCEW(32515)),.lpszClassName=L"Win2kFrameFixture"};
    if(!RegisterClassW(&wc))return 1;
    if(no_icon){
        HWND w=CreateWindowExW(0,wc.lpszClassName,L"No icon",WS_OVERLAPPEDWINDOW,
                                80,80,420,260,NULL,NULL,instance,NULL);
        if(!w)return 2;
        ShowWindow(w,SW_SHOW);UpdateWindow(w);SendMessageW(w,WM_NCACTIVATE,TRUE,0);
        TITLEBARINFOEX info={.cbSize=sizeof(info)};RECT outer;
        SendMessageW(w,WM_GETTITLEBARINFOEX,0,(LPARAM)&info);GetWindowRect(w,&outer);
        HDC dc=GetWindowDC(w);COLORREF before[16*16];unsigned drawn=0,changed=0;
        int left=info.rcTitleBar.left-outer.left+3,top=info.rcTitleBar.top-outer.top+3;
        if(dc){
            COLORREF background=GetPixel(dc,left,top);
            for(int y=0;y<16;++y)for(int x=0;x<16;++x){
                COLORREF pixel=GetPixel(dc,left+x,top+y);
                before[y*16+x]=pixel;
                if(pixel!=CLR_INVALID && pixel!=background)++drawn;
            }
            SendMessageW(w,WM_SETICON,ICON_BIG,(LPARAM)LoadIconW(NULL,MAKEINTRESOURCEW(32513)));
            for(int y=0;y<16;++y)for(int x=0;x<16;++x)
                if(before[y*16+x]!=GetPixel(dc,left+x,top+y))++changed;
            ReleaseDC(w,dc);
        }
        DestroyWindow(w);
        printf("WIN2K DEFAULT ICON bits=%u drawn=%u changed=%u\n",(unsigned)(sizeof(void*)*8),drawn,changed);
        return drawn>0 && changed>0?0:3;
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
    unsigned icon_pixels=0;
    COLORREF icon_before[16*12];
    if(dc){
        for(int iy=info.rcTitleBar.top-window.top+2;iy<info.rcTitleBar.bottom-window.top-2;++iy)
            for(int ix=info.rcTitleBar.left-window.left+4;ix<info.rcTitleBar.left-window.left+20;++ix){
                COLORREF c=GetPixel(dc,ix,iy);
                if(c!=CLR_INVALID && GetRValue(c)>=GetBValue(c))++icon_pixels;
            }
    }
    unsigned icon_changed=0;
    if(dc){
        int left=info.rcTitleBar.left-window.left+4,top=info.rcTitleBar.top-window.top+4;
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
    SendMessageW(w,WM_SYSCOMMAND,SC_MAXIMIZE,0);
    BOOL maximized=IsZoomed(w) && blue_caption(w);
    SendMessageW(w,WM_SYSCOMMAND,SC_RESTORE,0);
    BOOL restored=!IsZoomed(w) && GetPropW(w,L"Win2kStyle.Frame")!=NULL && blue_caption(w);
    printf("WIN2K FRAME bits=%u attached=%d title=%ldx%ld pixels=%06lx/%06lx end=%06lx inactive=%06lx/%06lx icon_pixels=%u icon_changed=%u text_pixels=%u hit=%lld\n",
        (unsigned)(sizeof(void*)*8),attached,info.rcTitleBar.right-info.rcTitleBar.left,
        info.rcTitleBar.bottom-info.rcTitleBar.top,(unsigned long)pixel,(unsigned long)pixel2,(unsigned long)endpoint,(unsigned long)inactive,(unsigned long)inactive_end,icon_pixels,icon_changed,changed,(long long)hit);
    SendMessageW(w,WM_SYSCOMMAND,SC_CLOSE,0);
    BOOL closed=!IsWindow(w);if(!closed)DestroyWindow(w);
    return attached && pixel!=CLR_INVALID && pixel2!=CLR_INVALID && pixel!=pixel2 && inactive!=CLR_INVALID &&
           GetBValue(pixel)>GetRValue(pixel) && GetRValue(endpoint)>=12 && GetGValue(endpoint)>=120 && GetBValue(endpoint)>=200 &&
           GetRValue(inactive)==GetGValue(inactive) && GetGValue(inactive)==GetBValue(inactive) && GetRValue(inactive_end)>=175 && GetRValue(inactive_end)<=181 && icon_pixels>0 && icon_changed>0 && changed>0 &&
           hit==HTCAPTION && resized && maximized && restored && closed?0:3;
}
