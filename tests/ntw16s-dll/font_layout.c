/* Actual candidate DLL, native dialog units and application font ownership. */
#include "ntw16s-dll/win16_style.h"
#define NTW16S_TEST_PROCESS_TIMEOUT 170000
#include "isolated_process.h"
#include <commctrl.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"FONT LAYOUT FAIL line=%d error=%lu\n",__LINE__,GetLastError());ExitProcess(1);}}while(0)
typedef BOOL (__cdecl *install_fn)(HWND,const win16_style_palette *);
typedef BOOL (__cdecl *remove_fn)(HWND);
static install_fn install_frame,install_control;
static BOOL (__cdecl *install_edit)(HWND,BOOL,const win16_style_palette *);
static remove_fn remove_frame,remove_control;
static BOOL verify_pixels;
typedef struct warm_stability {DWORD previous;unsigned samples,nonincreases;} warm_stability;
/* A bounded warmup needs two consecutive non-increasing post-cleanup counts.
 * Any growth restarts the streak; the final independent before/after assertion
 * remains in force. A plateau is not a claim about the whole process lifetime. */
static BOOL warm_sample(warm_stability *state,DWORD count)
{
    if(state->samples){if(count<=state->previous)++state->nonincreases;else state->nonincreases=0;}
    state->previous=count;++state->samples;
    return state->samples>=3 && state->nonincreases>=2;
}
static void warm_selftest(void)
{
    warm_stability transient={0},growing={0},oscillating={0},actual={0};
    const DWORD sequence[]={22,45,43,43},leaks[]={10,11,12,13,14,15,16,17},jitter[]={10,11,10,11,10,11,10,11};
    for(unsigned i=0;i<ARRAYSIZE(sequence);++i){BOOL ready=warm_sample(&transient,sequence[i]);CHECK(ready==(i==3));}
    for(unsigned i=0;i<ARRAYSIZE(leaks);++i)CHECK(!warm_sample(&growing,leaks[i]));
    for(unsigned i=0;i<ARRAYSIZE(jitter);++i)CHECK(!warm_sample(&oscillating,jitter[i]));
    HDC probe_dc=CreateCompatibleDC(NULL);HBITMAP probe_bitmap=CreateBitmap(2,2,1,32,NULL);CHECK(probe_dc && probe_bitmap);
    HGDIOBJ old_bitmap=SelectObject(probe_dc,probe_bitmap);CHECK(old_bitmap);
    HBRUSH prime=CreateSolidBrush(RGB(4,5,6));RECT area={0,0,2,2};CHECK(prime && FillRect(probe_dc,&area,prime) && DeleteObject(prime));GdiFlush();
    HDC held[4]={0};DWORD before=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS),previous=before;
    CHECK(!warm_sample(&actual,before));
    for(unsigned i=0;i<ARRAYSIZE(held);++i){held[i]=CreateCompatibleDC(NULL);CHECK(held[i]);GdiFlush();
        DWORD now=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);printf("FONT WARMUP LEAK PROBE iteration=%u previous=%lu now=%lu\n",i,previous,now);
        CHECK(now>previous && !warm_sample(&actual,now));previous=now;}
    for(unsigned i=0;i<ARRAYSIZE(held);++i)CHECK(DeleteDC(held[i]));
    GdiFlush();
    CHECK(GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS)<=before);
    CHECK(SelectObject(probe_dc,old_bitmap));CHECK(DeleteObject(probe_bitmap));CHECK(DeleteDC(probe_dc));
    puts("FONT WARMUP SELFTEST PASS transient=1 rising=1 real_gdi_leak=1 cleanup=1");
}
static void settle(void)
{MSG m;while(PeekMessageW(&m,NULL,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessageW(&m);}GdiFlush();}
static RECT rect(HWND w,HWND parent)
{RECT r;CHECK(GetWindowRect(w,&r));MapWindowPoints(NULL,parent,(POINT *)&r,2);return r;}
static SIZE text_size(HWND w,HFONT f)
{WCHAR text[128];RECT r={0};HDC dc=GetDC(w);CHECK(dc);HGDIOBJ old=SelectObject(dc,f?f:GetStockObject(SYSTEM_FONT));
 if(!GetWindowTextW(w,text,128))lstrcpyW(text,L"Representative text");
 CHECK(DrawTextW(dc,text,-1,&r,DT_CALCRECT|DT_SINGLELINE));
 SelectObject(dc,old);ReleaseDC(w,dc);return (SIZE){r.right,r.bottom};}
static HFONT system_font(HWND w,HFONT original)
{
    HFONT font=(HFONT)SendMessageW(w,WM_GETFONT,0,0);HDC dc=CreateCompatibleDC(NULL),expected=CreateCompatibleDC(NULL);
    HBITMAP a=CreateBitmap(128,64,1,1,NULL),b=CreateBitmap(128,64,1,1,NULL);
    HFONT base=CreateFontW(16,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_RASTER_PRECIS,0,NONANTIALIASED_QUALITY,0,L"System");
    CHECK(dc && expected && a && b && base);HGDIOBJ oa=SelectObject(dc,a),ob=SelectObject(expected,b);
    HGDIOBJ of=SelectObject(dc,original?original:GetStockObject(SYSTEM_FONT));TEXTMETRICW before,after,sys;
    CHECK(GetTextMetricsW(dc,&before));SelectObject(dc,font?font:GetStockObject(SYSTEM_FONT));
    char face[32];CHECK(GetTextFaceA(dc,32,face)>0 && (!strcmp(face,"System") || !strncmp(face,"Win16 System",12)));
    CHECK(GetTextMetricsW(dc,&after) && after.tmHeight==before.tmHeight &&
        after.tmAscent==before.tmAscent && after.tmInternalLeading==before.tmInternalLeading &&
        after.tmExternalLeading==before.tmExternalLeading);
    /* Keep the same single-character extent oracle, but avoid switching
     * fonts twice for each of the 95 characters in every warmup pass. */
    SIZE original_sizes[95];SelectObject(dc,original?original:GetStockObject(SYSTEM_FONT));
    for(WCHAR ch=32;ch<127;++ch)CHECK(GetTextExtentPoint32W(dc,&ch,1,&original_sizes[ch-32]));
    SelectObject(dc,font?font:GetStockObject(SYSTEM_FONT));
    for(WCHAR ch=32;ch<127;++ch){SIZE measured;
        CHECK(GetTextExtentPoint32W(dc,&ch,1,&measured) && measured.cx==original_sizes[ch-32].cx);}
    HGDIOBJ ef=SelectObject(expected,base);CHECK(GetTextMetricsW(expected,&sys));
    /* Independent host-System raster oracle. Exact pixel comparison rejects
     * merely renaming a Tahoma/MS Sans Serif font and the previous S1 DLL. */
    for(const WCHAR *ch=L"GQag012";verify_pixels && *ch;++ch){SIZE width,sw;CHECK(GetTextExtentPoint32W(dc,ch,1,&width) && GetTextExtentPoint32W(expected,ch,1,&sw));
        RECT clear={0,0,128,64};FillRect(dc,&clear,GetStockObject(WHITE_BRUSH));FillRect(expected,&clear,GetStockObject(WHITE_BRUSH));
        SetTextColor(dc,RGB(0,0,0));SetBkColor(dc,RGB(255,255,255));SetTextColor(expected,RGB(0,0,0));SetBkColor(expected,RGB(255,255,255));
        CHECK(TextOutW(dc,0,0,ch,1) && TextOutW(expected,0,0,ch,1));
        HDC scaled=CreateCompatibleDC(NULL);HBITMAP bitmap=CreateBitmap(128,64,1,1,NULL);CHECK(scaled && bitmap);
        HGDIOBJ old=SelectObject(scaled,bitmap);SetStretchBltMode(scaled,COLORONCOLOR);FillRect(scaled,&clear,GetStockObject(WHITE_BRUSH));
        int left=sw.cx,right=-1;GdiFlush();
        for(int yy=0;yy<sys.tmHeight;++yy)for(int xx=0;xx<sw.cx;++xx)if(GetPixel(expected,xx,yy)==RGB(0,0,0)){left=min(left,xx);right=max(right,xx);}
        CHECK(right>=left);int ink=right-left+1,dw=min(width.cx,max(1,MulDiv(ink,after.tmHeight,sys.tmHeight)));
        CHECK(StretchBlt(scaled,(width.cx-dw)/2,0,dw,after.tmHeight,expected,left,0,ink,sys.tmHeight,SRCCOPY));
        for(int y=0;y<after.tmHeight;++y)for(int x=0;x<width.cx;++x)CHECK(GetPixel(dc,x,y)==GetPixel(scaled,x,y));
        SelectObject(scaled,old);DeleteObject(bitmap);DeleteDC(scaled);
    }
    SelectObject(dc,of);SelectObject(expected,ef);SelectObject(dc,oa);SelectObject(expected,ob);
    DeleteObject(a);DeleteObject(b);DeleteObject(base);DeleteDC(dc);DeleteDC(expected);return font;
}
static void unchanged(HWND w,HWND parent,HFONT font,RECT before,SIZE text)
{RECT now=rect(w,parent);SIZE size=text_size(w,font);CHECK((HFONT)SendMessageW(w,WM_GETFONT,0,0)==font);
 CHECK(EqualRect(&now,&before) && size.cx==text.cx && size.cy==text.cy);}
static INT_PTR CALLBACK dialog_proc(HWND w,UINT m,WPARAM a,LPARAM b)
{(void)w;(void)a;(void)b;return m==WM_INITDIALOG;}
typedef struct reentrant_font {HFONT next;BOOL armed,entered;} reentrant_font;
static LRESULT CALLBACK font_callback(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    reentrant_font *state=(reentrant_font *)data;(void)id;
    if(m==WM_SETFONT && state->armed && !state->entered){LOGFONTW lf;state->entered=TRUE;
        SendMessageW(w,WM_SETFONT,(WPARAM)state->next,FALSE);
        CHECK(GetObjectW((HFONT)a,sizeof(lf),&lf)==sizeof(lf));
        CHECK(remove_control(w));
        CHECK(GetObjectW((HFONT)a,sizeof(lf),&lf)==sizeof(lf));
        CHECK((HFONT)SendMessageW(w,WM_GETFONT,0,0)==state->next);return 0;
    }
    return DefSubclassProc(w,m,a,b);
}
static void reentrant_case(void)
{
    HWND root=CreateWindowW(L"STATIC",L"Font lifetime",WS_OVERLAPPEDWINDOW,0,0,400,200,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(root);
    HWND child=CreateWindowW(L"BUTTON",L"Nested",WS_CHILD,0,0,200,40,root,NULL,GetModuleHandleW(NULL),NULL);CHECK(child);
    HFONT first=CreateFontW(-11,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,0,0,L"Tahoma");
    HFONT next=CreateFontW(-13,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,0,0,L"Arial");CHECK(first && next);
    SendMessageW(child,WM_SETFONT,(WPARAM)first,FALSE);reentrant_font state={.next=next};
    CHECK(SetWindowSubclass(child,font_callback,1,(DWORD_PTR)&state) && install_control(child,NULL));
    state.armed=TRUE;SendMessageW(child,WM_SETFONT,(WPARAM)first,FALSE);
    CHECK(state.entered && (HFONT)SendMessageW(child,WM_GETFONT,0,0)==next);
    CHECK(RemoveWindowSubclass(child,font_callback,1) && DestroyWindow(root) && DeleteObject(first) && DeleteObject(next));
    puts("FONT REENTRANT PASS nested change/remove retains outbound System font and restores latest app font");
}
/* Independent proportional oracle: host System16 pixels and advances, one
 * common DPI ratio, never MS Sans Serif metrics. Also checks shared columns. */
static int menu_width(HDC dc,const WCHAR *text,UINT dpi)
{
    int result=0;for(;*text;++text){SIZE size;CHECK(GetTextExtentPoint32W(dc,text,1,&size));result+=MulDiv(size.cx,(int)dpi,96);}return result;
}
static void menu_oracle(HDC dst,HDC src,const WCHAR *text,int x,int y,int height,UINT dpi)
{
    TEXTMETRICW tm;CHECK(GetTextMetricsW(src,&tm));
    for(;*text;++text){SIZE size;RECT clear={0,0,128,64};CHECK(GetTextExtentPoint32W(src,text,1,&size));
        CHECK(FillRect(src,&clear,GetStockObject(WHITE_BRUSH)) && TextOutW(src,0,0,text,1));
        int width=MulDiv(size.cx,(int)dpi,96);SetStretchBltMode(dst,COLORONCOLOR);
        if(width)CHECK(StretchBlt(dst,x,y,width,height,src,0,0,size.cx,tm.tmHeight,SRCCOPY));
        x+=width;
    }
}
static void menu_case(void)
{
    HWND w=CreateWindowW(L"STATIC",L"Typography",WS_OVERLAPPEDWINDOW,0,0,900,700,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(w);
    UINT dpi=GetDpiForWindow(w);HMENU popup=CreatePopupMenu(),child=CreatePopupMenu(),bar=CreateMenu();CHECK(popup && child && bar);
    CHECK(AppendMenuW(popup,MF_STRING,101,L"GQag012\tF2") && AppendMenuW(popup,MF_STRING,102,L"WMi\tCtrl+Shift+W") &&
        AppendMenuW(popup,MF_POPUP,(UINT_PTR)child,L"Nested") && AppendMenuW(child,MF_STRING,103,L"Command"));
    CHECK(AppendMenuW(bar,MF_POPUP,(UINT_PTR)popup,L"GQag012") && SetMenu(w,bar));
    CHECK(install_frame(w,NULL));SendMessageW(w,WM_INITMENUPOPUP,(WPARAM)popup,0);
    HFONT base=CreateFontW(16,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_RASTER_PRECIS,0,NONANTIALIASED_QUALITY,0,L"System");CHECK(base);
    HDC dc=CreateCompatibleDC(NULL),expected=CreateCompatibleDC(NULL),src=CreateCompatibleDC(NULL);
    HBITMAP a=CreateBitmap(800,100,1,1,NULL),b=CreateBitmap(800,100,1,1,NULL),c=CreateBitmap(128,64,1,1,NULL);CHECK(dc && expected && src && a && b && c);
    HGDIOBJ oa=SelectObject(dc,a),ob=SelectObject(expected,b),oc=SelectObject(src,c),of=SelectObject(src,base);TEXTMETRICW tm;CHECK(GetTextMetricsW(src,&tm));
    SetTextColor(src,RGB(0,0,0));SetBkColor(src,RGB(255,255,255));
    int height=MulDiv(tm.tmHeight,(int)dpi,96),u=max(1,MulDiv(1,(int)dpi,96));
    int label=max(menu_width(src,L"GQag012",dpi),max(menu_width(src,L"WMi",dpi),menu_width(src,L"Nested",dpi)));
    int shortcut=menu_width(src,L"Ctrl+Shift+W",dpi),width=MulDiv(18,(int)dpi,96)+label+MulDiv(8,(int)dpi,96)+shortcut+MulDiv(12,(int)dpi,96);
    /* Real menu bar drawing uses the same proportional font, not an
     * independently correct popup paired with a compressed bar. */
    RECT outer;POINT origin={0};CHECK(GetWindowRect(w,&outer) && ClientToScreen(w,&origin));
    int x0=origin.x-outer.left+MulDiv(6,(int)dpi,96),y0=origin.y-outer.top-(height+2*u);
    CHECK(y0>=0 && y0+height<100);
    RECT clear={0,0,800,100};CHECK(FillRect(dc,&clear,GetStockObject(WHITE_BRUSH)) && FillRect(expected,&clear,GetStockObject(WHITE_BRUSH)));
    SendMessageW(w,WM_PRINT,(WPARAM)dc,PRF_NONCLIENT|PRF_CLIENT|PRF_ERASEBKGND);
    menu_oracle(expected,src,L"GQag012",x0,y0,height,dpi);
    for(int y=y0;y<y0+height;++y)for(int x=x0;x<x0+menu_width(src,L"GQag012",dpi);++x)
        CHECK((GetPixel(dc,x,y)==RGB(0,0,0))==(GetPixel(expected,x,y)==RGB(0,0,0)));
    printf("FONT MENU BAR PASS dpi=%u System proportional glyph pixels\n",dpi);
    const WCHAR *labels[]={L"GQag012",L"WMi"},*keys[]={L"F2",L"Ctrl+Shift+W"};
    for(unsigned i=0;i<2;++i){
        MENUITEMINFOW item={.cbSize=sizeof(item),.fMask=MIIM_DATA|MIIM_FTYPE};CHECK(GetMenuItemInfoW(popup,i,TRUE,&item) && (item.fType&MFT_OWNERDRAW));
        MEASUREITEMSTRUCT measure={.CtlType=ODT_MENU,.itemID=101+i,.itemData=item.dwItemData};CHECK(SendMessageW(w,WM_MEASUREITEM,0,(LPARAM)&measure));
        printf("FONT MENU MEASURE dpi=%u row=%u size=%ux%u expected=%dx%d label=%d shortcut=%d\n",dpi,i,measure.itemWidth,measure.itemHeight,width,height+2*u,label,shortcut);fflush(stdout);
        CHECK(measure.itemHeight==(UINT)(height+2*u) && measure.itemWidth>0 && measure.itemWidth<=(UINT)width && width<800 && height<100);
        DRAWITEMSTRUCT draw={.CtlType=ODT_MENU,.itemID=101+i,.itemAction=ODA_DRAWENTIRE,.hwndItem=(HWND)popup,.hDC=dc,
            .rcItem={0,0,width,(LONG)measure.itemHeight},.itemData=item.dwItemData};
        CHECK(SendMessageW(w,WM_DRAWITEM,0,(LPARAM)&draw));RECT r=draw.rcItem;
        CHECK(FillRect(expected,&r,GetStockObject(WHITE_BRUSH)));
        menu_oracle(expected,src,labels[i],MulDiv(18,(int)dpi,96),u,height,dpi);
        menu_oracle(expected,src,keys[i],MulDiv(18,(int)dpi,96)+label+MulDiv(8,(int)dpi,96),u,height,dpi);
        for(int y=0;y<r.bottom;++y)for(int x=0;x<r.right;++x)
            CHECK((GetPixel(dc,x,y)==RGB(0,0,0))==(GetPixel(expected,x,y)==RGB(0,0,0)));
    }
    SelectObject(src,of);SelectObject(dc,oa);SelectObject(expected,ob);SelectObject(src,oc);
    DeleteObject(a);DeleteObject(b);DeleteObject(c);DeleteObject(base);DeleteDC(dc);DeleteDC(expected);DeleteDC(src);
    CHECK(remove_frame(w) && SetMenu(w,NULL) && DestroyWindow(w) && DestroyMenu(bar));
    printf("FONT MENU SYSTEM PASS dpi=%u proportional System16 glyph pixels; common shortcut/check/arrow columns\n",dpi);
}
static void put_word(BYTE **p,WORD w){memcpy(*p,&w,2);*p+=2;}
static void put_string(BYTE **p,const WCHAR *s)
{do{put_word(p,*s);}while(*s++);}
static HWND native_dialog(void)
{
    BYTE data[2048]={0},*p=data+sizeof(DLGTEMPLATE);
    DLGTEMPLATE t={.style=WS_POPUP|WS_CAPTION|WS_SYSMENU|DS_SETFONT,.cdit=4,.cx=190,.cy=95};memcpy(data,&t,sizeof(t));
    put_word(&p,0);put_word(&p,0);put_string(&p,L"Options");put_word(&p,8);put_string(&p,L"MS Sans Serif");
    const WCHAR *labels[]={L"Draw three",L"Standard",L"Timed game",L"Outline dragging"};
    for(unsigned i=0;i<4;++i){
        p=data+(((size_t)(p-data)+3)&~(size_t)3);
        DLGITEMTEMPLATE item={.style=WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,.x=8,.y=(short)(8+i*19),.cx=172,.cy=16,.id=(WORD)(100+i)};
        memcpy(p,&item,sizeof(item));p+=sizeof(item);put_word(&p,0xffff);put_word(&p,0x80);put_string(&p,labels[i]);put_word(&p,0);
    }
    return CreateDialogIndirectParamW(GetModuleHandleW(NULL),(DLGTEMPLATE *)data,NULL,dialog_proc,0);
}
static void dialog_case(void)
{
    HWND w=native_dialog();CHECK(w);HFONT fonts[4];RECT bounds[4];SIZE sizes[4];
    for(unsigned i=0;i<4;++i){HWND child=GetDlgItem(w,100+i);CHECK(child);fonts[i]=(HFONT)SendMessageW(child,WM_GETFONT,0,0);bounds[i]=rect(child,w);sizes[i]=text_size(child,fonts[i]);
        CHECK(sizes[i].cx+26<=bounds[i].right-bounds[i].left && sizes[i].cy<=bounds[i].bottom-bounds[i].top);}
    CHECK(install_frame(w,NULL));
    for(unsigned refresh=0;refresh<3;++refresh)SendMessageW(w,WM_SETTINGCHANGE,0,0);
    for(unsigned i=0;i<4;++i){HWND child=GetDlgItem(w,100+i);CHECK(install_control(child,NULL));unchanged(child,w,system_font(child,fonts[i]),bounds[i],sizes[i]);CHECK(remove_control(child));unchanged(child,w,fonts[i],bounds[i],sizes[i]);}
    printf("FONT LAYOUT DIALOG PASS dpi=%u native DS_SETFONT DLU labels fit; repeated frame font refresh\n",GetDpiForWindow(w));
    CHECK(remove_frame(w) && DestroyWindow(w));
}
static void controls_case(unsigned scale,unsigned bold)
{
    DWORD start=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
    HWND root=CreateWindowW(L"STATIC",L"Typography",WS_OVERLAPPEDWINDOW,0,0,900,700,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(root);
    HFONT font=CreateFontW(-MulDiv(11,(int)scale,96),0,0,0,bold?FW_BOLD:FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,NONANTIALIASED_QUALITY,0,L"Tahoma");
    HFONT next=CreateFontW(-MulDiv(10,(int)scale,96),0,0,0,FW_NORMAL,TRUE,FALSE,FALSE,DEFAULT_CHARSET,0,0,NONANTIALIASED_QUALITY,0,L"Arial");CHECK(font && next);
    const WCHAR *classes[]={L"BUTTON",L"BUTTON",L"BUTTON",L"STATIC",L"EDIT",L"LISTBOX",L"COMBOBOX"};
    const DWORD styles[]={BS_AUTOCHECKBOX,BS_AUTORADIOBUTTON,BS_GROUPBOX,SS_LEFT,ES_AUTOHSCROLL,LBS_NOINTEGRALHEIGHT,CBS_DROPDOWNLIST};
    const WCHAR *labels[]={L"Outline dragging",L"Standard scoring",L"Draw",L"Timed game",L"Editable text",L"List",L"Combo"};
    for(unsigned i=0;i<7;++i){
        HWND w=CreateWindowW(classes[i],labels[i],WS_CHILD|WS_VISIBLE|styles[i],10,(int)i*80,600,60,root,NULL,GetModuleHandleW(NULL),NULL);CHECK(w);
        SendMessageW(w,WM_SETFONT,(WPARAM)font,FALSE);RECT before=rect(w,root);SIZE size=text_size(w,font);
        CHECK(size.cx+32<=before.right-before.left && size.cy<=before.bottom-before.top);
        CHECK(i==4?install_edit(w,FALSE,NULL):install_control(w,NULL));unchanged(w,root,system_font(w,font),before,size);
        SendMessageW(w,WM_SETFONT,(WPARAM)next,FALSE);system_font(w,next);
        CHECK(remove_control(w));CHECK((HFONT)SendMessageW(w,WM_GETFONT,0,0)==next);
        CHECK(i==4?install_edit(w,FALSE,NULL):install_control(w,NULL));SendMessageW(w,WM_SETFONT,0,FALSE);CHECK(!SendMessageW(w,WM_GETFONT,0,0));CHECK(remove_control(w));CHECK(!SendMessageW(w,WM_GETFONT,0,0));
        CHECK(DestroyWindow(w));
    }
    CHECK(DestroyWindow(root));CHECK(DeleteObject(next) && DeleteObject(font));
    settle();
    printf("FONT RESOURCE DIAGNOSTIC start=%lu end=%lu\n",start,GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS));
    printf("FONT LAYOUT CONTROLS PASS nominal-dpi=%u weight=%u seven classes; app update/null/detach\n",scale,bold?700:400);
}
static int worker(int argc,WCHAR **argv)
{
    BOOL menu_only=argc==3 && !wcscmp(argv[2],L"--menu-only"),stress=argc==3 && !wcscmp(argv[2],L"--stress");
    CHECK(argc==2 || menu_only || stress);HMODULE dll=LoadLibraryExW(argv[1],NULL,LOAD_WITH_ALTERED_SEARCH_PATH);CHECK(dll);
    install_frame=(install_fn)(void *)GetProcAddress(dll,"win16_style_install");remove_frame=(remove_fn)(void *)GetProcAddress(dll,"win16_style_remove");
    install_control=(install_fn)(void *)GetProcAddress(dll,"win16_style_control_install");remove_control=(remove_fn)(void *)GetProcAddress(dll,"win16_style_control_remove");CHECK(install_frame && remove_frame && install_control && remove_control);
    install_edit=(void *)GetProcAddress(dll,"win16_style_edit_install");CHECK(install_edit);
    if(!menu_only)warm_selftest();
    for(unsigned context=0;context<2;++context){
        CHECK(SetThreadDpiAwarenessContext(context?DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2:DPI_AWARENESS_CONTEXT_UNAWARE));
        if(menu_only){menu_case();continue;}
        /* USER lazily allocates per-class/font caches. Exercise every variant
         * in each cycle, stopping only after bounded post-cleanup stability. */
        warm_stability warm_state={0};BOOL stable=FALSE;
        for(unsigned warm=0;warm<8 && (stress || !stable);++warm){
            dialog_case();for(unsigned dpi=96;dpi<=192;dpi+=48)for(unsigned bold=0;bold<2;++bold)controls_case(dpi,bold);
            settle();
            DWORD count=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
            stable=warm_sample(&warm_state,count);
            printf("FONT WARMUP context=%u iteration=%u gdi=%lu stable=%u\n",context,warm,count,stable);
        }
        CHECK(stable);
        if(stress){CHECK(warm_state.samples==8);printf("FONT WARMUP STRESS PASS context=%u rounds=8\n",context);}
        verify_pixels=TRUE;menu_case();reentrant_case();settle();
        DWORD before=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
        dialog_case();for(unsigned dpi=96;dpi<=192;dpi+=48)for(unsigned bold=0;bold<2;++bold)controls_case(dpi,bold);
        settle();
        DWORD after=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
        printf("FONT LAYOUT CONTEXT PASS aware=%u GDI=%lu/%lu\n",context,before,after);
        CHECK(after<=before);
        verify_pixels=FALSE;
    }
    CHECK(FreeLibrary(dll));if(menu_only){puts("FONT MENU ONLY PASS: proportional bar/popup subset, not the full control matrix");return 0;}puts("FONT LAYOUT PASS 2 contexts 12 font variants native dialog and 84 control scenarios; repeated after warmup; actual candidate DLL");return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
