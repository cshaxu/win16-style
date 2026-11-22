#include <windows.h>
#include <oleacc.h>
#include <stdio.h>
#include <inttypes.h>

#define CHECK(x) do{if(!(x)){fprintf(stderr,"LUNA FRAME FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)

typedef BOOL (__cdecl *remove_frame)(HWND);

static uint64_t sample(HWND window,RECT area,POINT point,COLORREF *center)
{
    RECT bounds;
    if(!GetWindowRect(window,&bounds))return 0;
    HDC display=GetDC(window);
    if(!display)return 0;
    HDC dc=CreateCompatibleDC(display);
    HBITMAP bitmap=dc?CreateCompatibleBitmap(display,bounds.right-bounds.left,bounds.bottom-bounds.top):NULL;
    HGDIOBJ before=bitmap?SelectObject(dc,bitmap):NULL;
    uint64_t hash=0;
    if(before && before!=HGDI_ERROR && PrintWindow(window,dc,2)){
        hash=UINT64_C(1469598103934665603);
        for(LONG y=area.top;y<area.bottom;++y)
            for(LONG x=area.left;x<area.right;++x)
                hash=(hash^GetPixel(dc,x,y))*UINT64_C(1099511628211);
        *center=GetPixel(dc,point.x,point.y);
        if(*center==CLR_INVALID)hash=0;
    }
    if(before && before!=HGDI_ERROR)SelectObject(dc,before);
    if(bitmap)DeleteObject(bitmap);
    if(dc)DeleteDC(dc);
    ReleaseDC(window,display);
    return hash;
}

typedef struct caption_witness {
    DWORD states[3];
    uint64_t faces[3];
    COLORREF outer;
} caption_witness;

static BOOL read_caption(HWND window,caption_witness *sample,TITLEBARINFOEX *info)
{
    static const unsigned slots[]={2,3,5};
    *info=(TITLEBARINFOEX){.cbSize=sizeof(*info)};
    SendMessageW(window,WM_GETTITLEBARINFOEX,0,(LPARAM)info);
    RECT window_rect;
    if(!GetWindowRect(window,&window_rect))return FALSE;
    HDC dc=GetWindowDC(window);
    if(!dc)return FALSE;
    sample->outer=GetPixel(dc,(window_rect.right-window_rect.left)/2,2);
    BOOL valid=sample->outer!=CLR_INVALID;
    for(unsigned i=0;i<ARRAYSIZE(slots);++i){
        RECT face=info->rgrect[slots[i]];
        OffsetRect(&face,-window_rect.left,-window_rect.top);
        if(IsRectEmpty(&face)){valid=FALSE;break;}
        uint64_t hash=UINT64_C(1469598103934665603);
        for(LONG y=face.top;y<face.bottom;++y){
            for(LONG x=face.left;x<face.right;++x){
                COLORREF pixel=GetPixel(dc,x,y);
                if(pixel==CLR_INVALID)valid=FALSE;
                hash=(hash^pixel)*UINT64_C(1099511628211);
            }
        }
        sample->states[i]=info->rgstate[slots[i]];
        sample->faces[i]=hash;
    }
    ReleaseDC(window,dc);
    return valid;
}

static unsigned native_hover_messages;

static LRESULT CALLBACK witness_proc(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    if(message==WM_NCMOUSEMOVE || message==WM_NCMOUSELEAVE)++native_hover_messages;
    return DefWindowProcW(window,message,wp,lp);
}

static int check_caption_input(HWND window)
{
    static const unsigned slots[]={2,3,5};
    static const WPARAM hit[]={HTMINBUTTON,HTMAXBUTTON,HTCLOSE};
    const DWORD mouse_states=STATE_SYSTEM_PRESSED|STATE_SYSTEM_HOTTRACKED;
    caption_witness normal,hover,pressed,restored,inactive;
    TITLEBARINFOEX info;
    SendMessageW(window,WM_NCMOUSELEAVE,0,0);
    native_hover_messages=0;
    CHECK(read_caption(window,&normal,&info));
    for(unsigned i=0;i<ARRAYSIZE(slots);++i){
        RECT button=info.rgrect[slots[i]];
        POINT center={(button.left+button.right)/2,(button.top+button.bottom)/2};
        LPARAM point=MAKELPARAM(center.x,center.y);
        CHECK(!(normal.states[i]&mouse_states));
        CHECK(SendMessageW(window,WM_NCHITTEST,0,point)==(LRESULT)hit[i]);
        SendMessageW(window,WM_NCMOUSEMOVE,hit[i],point);
        CHECK(read_caption(window,&hover,&info));
        CHECK((hover.states[i]&mouse_states)==STATE_SYSTEM_HOTTRACKED);
        CHECK(hover.faces[i]!=normal.faces[i]);
        SendMessageW(window,WM_NCLBUTTONDOWN,hit[i],point);
        CHECK(read_caption(window,&pressed,&info));
        CHECK(GetCapture()==window && (pressed.states[i]&mouse_states)==STATE_SYSTEM_PRESSED);
        CHECK(pressed.faces[i]!=hover.faces[i] && pressed.faces[i]!=normal.faces[i]);
        for(unsigned j=0;j<ARRAYSIZE(slots);++j)
            if(j!=i)CHECK(!(pressed.states[j]&mouse_states) && pressed.faces[j]==normal.faces[j]);
        SendMessageW(window,WM_CANCELMODE,0,0);
        SendMessageW(window,WM_NCMOUSELEAVE,0,0);
        CHECK(read_caption(window,&restored,&info));
        CHECK(!GetCapture() && !(restored.states[i]&mouse_states) && restored.faces[i]==normal.faces[i]);
    }
    CHECK(native_hover_messages==0);
    SendMessageW(window,WM_NCACTIVATE,FALSE,0);
    CHECK(read_caption(window,&inactive,&info) && inactive.outer!=normal.outer);
    SendMessageW(window,WM_NCACTIVATE,TRUE,0);
    CHECK(read_caption(window,&restored,&info) && restored.outer==normal.outer);
    printf("LUNA CAPTION INPUT PASS bits=%u dpi=%u outer=%06lx/%06lx three_buttons native_hover=0\n",
        (unsigned)(sizeof(void *)*8),GetDpiForWindow(window),normal.outer,inactive.outer);
    return 0;
}

int wmain(int argc,WCHAR **argv)
{
    BOOL classic=argc==2 && !lstrcmpW(argv[1],L"--classic");
    BOOL input=argc==2 && !lstrcmpW(argv[1],L"--caption-input");
    int expected=25;
    if(argc==2 && !classic && !input){
        if(!lstrcmpW(argv[1],L"LargeFonts"))expected=29;
        else if(!lstrcmpW(argv[1],L"ExtraLargeFonts"))expected=35;
        else CHECK(!lstrcmpW(argv[1],L"NormalSize"));
    }else CHECK(argc==1 || classic || input);
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    const WCHAR class_name[]=L"Run16s.LunaFrameProbe";
    WNDCLASSW type={.lpfnWndProc=input?witness_proc:DefWindowProcW,.hInstance=GetModuleHandleW(NULL),
        .hbrBackground=(HBRUSH)(COLOR_WINDOW+1),.lpszClassName=class_name};
    CHECK(RegisterClassW(&type));
    HWND window=CreateWindowExW(0,class_name,L"Luna frame probe",WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,CW_USEDEFAULT,420,240,NULL,NULL,type.hInstance,NULL);
    CHECK(window);
    HWND push=CreateWindowExW(0,L"Button",L"Luna push",WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
        36,64,120,36,window,NULL,type.hInstance,NULL);
    HWND check=CreateWindowExW(0,L"Button",L"Native checkbox",WS_CHILD|WS_VISIBLE|BS_CHECKBOX,
        36,118,160,36,window,NULL,type.hInstance,NULL);
    CHECK(push && check);
    ShowWindow(window,SW_SHOWNORMAL);
    for(unsigned i=0;i<10;++i){
        MSG message;
        while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}
        Sleep(20);
    }
    SetForegroundWindow(window);
    SendMessageW(window,WM_NCACTIVATE,TRUE,0);
    CHECK(GetPropW(window,L"Win2kStyle.Frame"));
    if(input){
        int result=check_caption_input(window);
        CHECK(DestroyWindow(window));
        return result;
    }
    TITLEBARINFOEX title={.cbSize=sizeof(title)};
    SendMessageW(window,WM_GETTITLEBARINFOEX,0,(LPARAM)&title);
    RECT bounds;CHECK(GetWindowRect(window,&bounds));
    RECT band=title.rcTitleBar;
    OffsetRect(&band,-bounds.left,-bounds.top);
    RECT close=title.rgrect[5];
    OffsetRect(&close,-bounds.left,-bounds.top);
    UINT dpi=GetDpiForWindow(window);
    printf("LUNA FRAME GEOMETRY dpi=%u band=%ld,%ld,%ld,%ld close=%ld,%ld,%ld,%ld\n",dpi,
        band.left,band.top,band.right,band.bottom,close.left,close.top,close.right,close.bottom);
    fflush(stdout);
    CHECK(dpi && !IsRectEmpty(&close));
    if(classic){
        printf("CLASSIC HEIGHT bits=%u dpi=%u height=%ld\n",(unsigned)(sizeof(void *)*8),dpi,band.bottom-band.top);
        CHECK(DestroyWindow(window));
        return 0;
    }
    CHECK(band.bottom-band.top==MulDiv(expected,(int)dpi,96));
    POINT inside_close={(close.left+close.right)/2,(close.top+close.bottom)/2};
    LRESULT hit=SendMessageW(window,WM_NCHITTEST,0,MAKELPARAM(inside_close.x+bounds.left,inside_close.y+bounds.top));
    CHECK(hit==HTCLOSE);
    RECT strip={band.left+100,band.top+3,band.left+230,band.bottom-3};
    POINT center={(strip.left+strip.right)/2,strip.top+5};
    COLORREF caption=CLR_INVALID,button=CLR_INVALID,restored=CLR_INVALID;
    uint64_t before=sample(window,strip,center,&caption),button_pattern=sample(window,close,inside_close,&button);
    CHECK(before && button_pattern);
    HMODULE runtime=GetModuleHandleW(sizeof(void *)==8?L"ntwshk64.dll":L"ntwshk32.dll");
    CHECK(runtime);
    remove_frame remove=(remove_frame)(uintptr_t)GetProcAddress(runtime,"Ntw2ksWindowRemove");
    remove_frame attached=(remove_frame)(uintptr_t)GetProcAddress(runtime,"Ntw2ksWindowAttached");
    CHECK(attached && attached(push) && !attached(check));
    RECT push_region={3,3,112,32};
    POINT push_sample={14,16};
    COLORREF push_center=CLR_INVALID,pressed_center=CLR_INVALID;
    uint64_t push_normal=sample(push,push_region,push_sample,&push_center);
    SendMessageW(push,BM_SETSTATE,TRUE,0);
    uint64_t push_pressed=sample(push,push_region,push_sample,&pressed_center);
    SendMessageW(push,BM_SETSTATE,FALSE,0);
    CHECK(push_normal && push_pressed && push_normal!=push_pressed);
    CHECK(remove && remove(push) && !attached(push));
    COLORREF push_restored=CLR_INVALID;
    uint64_t native_button=sample(push,push_region,push_sample,&push_restored);
    CHECK(native_button && native_button!=push_normal);
    CHECK(remove(window) && !GetPropW(window,L"Win2kStyle.Frame"));
    printf("LUNA PUSH PASS normal=%016" PRIx64 " pressed=%016" PRIx64 " restored=%016" PRIx64
        " native_checkbox=1\n",push_normal,push_pressed,native_button);
    uint64_t after=sample(window,strip,center,&restored);
    CHECK(after && before!=after);
    printf("LUNA FRAME PASS bits=%u dpi=%u height=%ld sample=%06lx close=%06lx caption=%016" PRIx64
        " button=%016" PRIx64 " restored=%016" PRIx64 "\n",(unsigned)(sizeof(void *)*8),dpi,
        band.bottom-band.top,(unsigned long)caption,(unsigned long)button,before,button_pattern,after);
    CHECK(DestroyWindow(window));
    return 0;
}