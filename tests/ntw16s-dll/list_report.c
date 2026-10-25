#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <wchar.h>
#include "isolated_process.h"
#define CHECK(x) do { if(!(x)){fprintf(stderr,"LIST_FAIL line=%u err=%lu %s\n",__LINE__,GetLastError(),#x);return 1;} }while(0)
static HFONT appfont;
static unsigned pre,items,draw_mode,trace;
static BOOL destroy_in_draw,remove_in_draw;
static BOOL (__cdecl *remove_style)(HWND);

static HWND list;
static LRESULT CALLBACK rootproc(HWND w,UINT m,WPARAM p,LPARAM l)
{
    if(m==WM_NOTIFY && l){NMHDR *n=(NMHDR *)l;
        if(n->code==NM_CUSTOMDRAW && n->hwndFrom==list){NMLVCUSTOMDRAW *cd=(NMLVCUSTOMDRAW *)l;
            if(cd->nmcd.dwDrawStage==CDDS_PREPAINT){++pre;trace=(trace^CDDS_PREPAINT)*16777619u;
                if(remove_in_draw){remove_in_draw=FALSE;remove_style(list);}
                if(destroy_in_draw){destroy_in_draw=FALSE;DestroyWindow(list);return CDRF_SKIPDEFAULT;}
                return draw_mode?CDRF_NOTIFYITEMDRAW:CDRF_DODEFAULT;}
            if(cd->nmcd.dwDrawStage==CDDS_ITEMPREPAINT){++items;trace=(trace^(unsigned)cd->nmcd.dwItemSpec)*16777619u;SelectObject(cd->nmcd.hdc,appfont);
                cd->clrText=RGB(0,0,0);cd->clrTextBk=RGB(238,238,238);
                if(draw_mode==2){RECT r={.left=LVIR_BOUNDS};
                    /* Actual x86 COMCTL5 leaves nmcd.rc indeterminate here.
                     * Query the initialized public item geometry explicitly. */
                    if(!SendMessageW(list,LVM_GETITEMRECT,cd->nmcd.dwItemSpec,(LPARAM)&r))ExitProcess(1);
                    FillRect(cd->nmcd.hdc,&r,GetStockObject(GRAY_BRUSH));
                    TextOutA(cd->nmcd.hdc,r.left+4,r.top,"application",11);return CDRF_SKIPDEFAULT;}
                return CDRF_NEWFONT;}
        }
    }
    return DefWindowProcW(w,m,p,l);
}
static DWORD bits[600*180],native[600*180];
static unsigned capture(HWND w,DWORD *out)
{
    HDC dc=CreateCompatibleDC(NULL);BITMAPINFO info={0};void *data=NULL;
    info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=600;info.bmiHeader.biHeight=-180;
    info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
    HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&data,NULL,0);HGDIOBJ old=SelectObject(dc,bitmap);
    RECT r={0,0,600,180};FillRect(dc,&r,GetStockObject(WHITE_BRUSH));pre=items=0;trace=2166136261u;
    SendMessageW(w,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT|PRF_ERASEBKGND);GdiFlush();
    CopyMemory(out,data,sizeof(bits));SelectObject(dc,old);DeleteObject(bitmap);DeleteDC(dc);
    return items;
}
/* Independent bitmap reference: host System16 glyphs scaled to the native
 * application's M advance and cell, without using the product font builder. */
static BOOL system_mask(const DWORD *pixels,const RECT *row,HFONT font)
{
    HDC src=CreateCompatibleDC(NULL),dst=CreateCompatibleDC(NULL);if(!src || !dst)return FALSE;
    HBITMAP mono=CreateBitmap(128,64,1,1,NULL);BITMAPINFO bi={0};DWORD *expected=NULL;
    bi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);bi.bmiHeader.biWidth=200;bi.bmiHeader.biHeight=-64;bi.bmiHeader.biPlanes=1;bi.bmiHeader.biBitCount=32;
    HBITMAP bm=CreateDIBSection(dst,&bi,DIB_RGB_COLORS,(void **)&expected,NULL,0);
    HFONT system=CreateFontW(16,0,0,0,FW_BOLD,0,0,0,ANSI_CHARSET,OUT_RASTER_PRECIS,CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,DEFAULT_PITCH,L"System");
    if(!mono || !bm || !system)ExitProcess(2);
    HGDIOBJ os=SelectObject(src,mono),od=SelectObject(dst,bm),of=SelectObject(src,system),nf=SelectObject(dst,font);
    TEXTMETRICW st,nt;GetTextMetricsW(src,&st);GetTextMetricsW(dst,&nt);
    RECT a={0,0,128,64},b={0,0,200,64};FillRect(src,&a,GetStockObject(WHITE_BRUSH));FillRect(dst,&b,GetStockObject(WHITE_BRUSH));
    SetBkColor(src,RGB(255,255,255));SetTextColor(src,0);SetStretchBltMode(dst,BLACKONWHITE);
    const WCHAR text[]=L"E8M0M0E8";int width=0;
    for(int i=0;i<8;++i){SIZE sw,nw;GetTextExtentPoint32W(src,text+i,1,&sw);GetTextExtentPoint32W(dst,text+i,1,&nw);
        FillRect(src,&a,GetStockObject(WHITE_BRUSH));TextOutW(src,0,0,text+i,1);
        StretchBlt(dst,width,0,nw.cx,nt.tmHeight,src,0,0,sw.cx,st.tmHeight,SRCCOPY);width+=nw.cx;}
    GdiFlush();BOOL found=FALSE;
    for(int y=row->top;y+nt.tmHeight<=row->bottom && !found;++y)for(int x=row->left;x<row->left+12 && !found;++x){
        BOOL equal=TRUE;
        for(int dy=0;dy<nt.tmHeight && equal;++dy)for(int dx=0;dx<width;++dx)
            if(((pixels[(y+dy)*600+x+dx]&0xffffff)==0)!=((expected[dy*200+dx]&0xffffff)==0)){equal=FALSE;break;}
        if(equal)found=TRUE;
    }
    SelectObject(src,of);SelectObject(src,os);SelectObject(dst,nf);SelectObject(dst,od);
    DeleteObject(system);DeleteObject(mono);DeleteObject(bm);DeleteDC(src);DeleteDC(dst);return found;
}
static int child(BOOL styled,BOOL aware)
{
    if(aware)CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));
    INITCOMMONCONTROLSEX init={sizeof(init),ICC_LISTVIEW_CLASSES};CHECK(InitCommonControlsEx(&init));
    HMODULE dll=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");
    BOOL (__cdecl *attached)(HWND)=dll?(void *)GetProcAddress(dll,"Ntw16sWindowAttached"):NULL;
    remove_style=dll?(void *)GetProcAddress(dll,"Ntw16sWindowRemove"):NULL;
    WNDCLASSW cls={0};cls.lpfnWndProc=rootproc;cls.hInstance=GetModuleHandleW(NULL);cls.lpszClassName=L"ListReportProbe";CHECK(RegisterClassW(&cls));
    HWND root=CreateWindowW(cls.lpszClassName,L"report",WS_OVERLAPPEDWINDOW,0,0,660,250,NULL,NULL,cls.hInstance,NULL);CHECK(root);
    WNDCLASSEXW alias={.cbSize=sizeof(alias)};CHECK(GetClassInfoExW(NULL,WC_LISTVIEWW,&alias));
    alias.hInstance=cls.hInstance;alias.lpszClassName=L"TListView";CHECK(RegisterClassExW(&alias));
    for(unsigned kind=0;kind<3;++kind)for(unsigned font_case=0;font_case<2;++font_case)for(draw_mode=0;draw_mode<3;++draw_mode){
    list=CreateWindowExW(0,kind?L"TListView":WC_LISTVIEWW,L"",WS_CHILD|WS_VISIBLE|WS_BORDER|LVS_SHAREIMAGELISTS|(kind==2?LVS_ICON:LVS_REPORT),0,0,600,180,root,(HMENU)11,cls.hInstance,NULL);CHECK(list);
    CHECK(!styled || (attached && attached(list)));
    if(kind==2)SetWindowLongPtrW(list,GWL_STYLE,(GetWindowLongPtrW(list,GWL_STYLE)&~LVS_TYPEMASK)|LVS_REPORT);
    appfont=CreateFontW(font_case?-11:-16,0,0,0,FW_NORMAL,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,DEFAULT_PITCH,font_case?L"MS Sans Serif":L"Arial");CHECK(appfont);
    SendMessageW(list,WM_SETFONT,(WPARAM)appfont,FALSE);
    HIMAGELIST images=ImageList_Create(16,16,ILC_COLOR32,1,1);CHECK(images);
    HDC icon_dc=CreateCompatibleDC(NULL);HBITMAP icon=CreateBitmap(16,16,1,32,NULL);CHECK(icon_dc && icon);
    HGDIOBJ ib=SelectObject(icon_dc,icon);RECT ir={0,0,16,16};HBRUSH brush=CreateSolidBrush(RGB(31,127,219));CHECK(brush);
    FillRect(icon_dc,&ir,brush);DeleteObject(brush);SelectObject(icon_dc,ib);DeleteDC(icon_dc);CHECK(ImageList_Add(images,icon,NULL)==0);DeleteObject(icon);
    SendMessageW(list,LVM_SETIMAGELIST,LVSIL_SMALL,(LPARAM)images);
    LVCOLUMNW col={.mask=LVCF_TEXT|LVCF_WIDTH,.cx=240,.pszText=L"Name"};CHECK(SendMessageW(list,LVM_INSERTCOLUMNW,0,(LPARAM)&col)==0);
    col.pszText=L"Value";CHECK(SendMessageW(list,LVM_INSERTCOLUMNW,1,(LPARAM)&col)==1);
    for(int i=0;i<2;++i){LVITEMW item={.mask=LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE,.iItem=i,.pszText=L"E8M0M0E8",.lParam=100+i};CHECK(SendMessageW(list,LVM_INSERTITEMW,0,(LPARAM)&item)==i);
        item.iSubItem=1;item.pszText=L"value";CHECK(SendMessageW(list,LVM_SETITEMTEXTW,i,(LPARAM)&item));}
    ShowWindow(root,SW_SHOWNORMAL);UpdateWindow(root);
    RECT before,after;GetClientRect(list,&before);
    unsigned ni=capture(list,bits),ti=trace;CHECK(pre && ni==(draw_mode?2u:0u));CHECK((HFONT)SendMessageW(list,WM_GETFONT,0,0)==appfont);
    if(styled){CHECK(remove_style(list));CHECK(!attached(list));}
    unsigned nn=capture(list,native);CHECK(pre && nn==ni && ti==trace);
    GetClientRect(list,&after);CHECK(EqualRect(&before,&after));
    CHECK(SendMessageW(list,LVM_GETCOLUMNWIDTH,0,0)==240);
    CHECK((HIMAGELIST)SendMessageW(list,LVM_GETIMAGELIST,LVSIL_SMALL,0)==images && ImageList_GetImageCount(images)==1);
    WCHAR value[32];LVITEMW sub={.iSubItem=1,.pszText=value,.cchTextMax=ARRAYSIZE(value)};
    CHECK(SendMessageW(list,LVM_GETITEMTEXTW,1,(LPARAM)&sub)==5 && !wcscmp(value,L"value"));
    RECT icon_rect={.left=LVIR_ICON};CHECK(SendMessageW(list,LVM_GETITEMRECT,0,(LPARAM)&icon_rect));
    for(int y=icon_rect.top;y<icon_rect.bottom;++y)for(int x=icon_rect.left;x<icon_rect.right;++x)CHECK(bits[y*600+x]==native[y*600+x]);
    LVITEMW item={.mask=LVIF_PARAM,.iItem=1};CHECK(SendMessageW(list,LVM_GETITEMW,0,(LPARAM)&item) && item.lParam==101);
    unsigned changed=0;RECT label={.left=LVIR_LABEL};CHECK(SendMessageW(list,LVM_GETITEMRECT,0,(LPARAM)&label));
    for(int y=label.top;y<label.bottom;++y)for(int x=label.left;x<200;++x)changed+=bits[y*600+x]!=native[y*600+x];
    printf("LIST_CAPTURE bits=%u styled=%u changed=%u label=%ld,%ld,%ld,%ld callbacks=%u,%u\n",(unsigned)sizeof(void*)*8,styled,changed,label.left,label.top,label.right,label.bottom,ni,nn);
    CHECK(styled && draw_mode!=2?changed>20:changed==0);
    if(styled && draw_mode!=2){CHECK(system_mask(bits,&label,appfont));puts("LIST_REPORT_SYSTEM_MASK_OK host_system16_native_cell=1 small_E8M0=1");}
    LVITEMW state={.stateMask=LVIS_SELECTED|LVIS_FOCUSED,.state=LVIS_SELECTED|LVIS_FOCUSED};
    CHECK(SendMessageW(list,LVM_SETITEMSTATE,1,(LPARAM)&state));SetFocus(list);
    CHECK(SendMessageW(list,LVM_GETSELECTEDCOUNT,0,0)==1);
    SendMessageW(list,WM_KEYDOWN,VK_HOME,0);SendMessageW(list,WM_KEYUP,VK_HOME,0);
    CHECK(SendMessageW(list,LVM_GETNEXTITEM,(WPARAM)-1,LVNI_FOCUSED)==0);
    CHECK(DestroyWindow(list));CHECK(ImageList_GetImageCount(images)==1 && ImageList_Destroy(images));CHECK(DeleteObject(appfont));
    }
    if(styled){
        const DWORD unsupported[]={LVS_ICON,LVS_REPORT|LVS_OWNERDATA,LVS_REPORT|LVS_OWNERDRAWFIXED};
        for(unsigned i=0;i<ARRAYSIZE(unsupported);++i){
            list=CreateWindowExW(0,WC_LISTVIEWW,L"",WS_CHILD|WS_VISIBLE|WS_BORDER|unsupported[i],0,0,600,180,root,(HMENU)14,cls.hInstance,NULL);CHECK(list);
            draw_mode=0;(void)capture(list,bits);
            CHECK(!GetPropW(list,L"ntw16s.private.control-protocol.v1"));CHECK(DestroyWindow(list));
        }
        puts("LIST_REPORT_UNSUPPORTED_NATIVE_OK view_ownerdata_ownerdraw=3");
        list=CreateWindowExW(0,WC_LISTVIEWW,L"",WS_CHILD|WS_VISIBLE|LVS_REPORT,0,0,600,180,root,(HMENU)12,cls.hInstance,NULL);CHECK(list && attached(list));
        appfont=(HFONT)GetStockObject(SYSTEM_FONT);draw_mode=0;remove_in_draw=TRUE;(void)capture(list,bits);CHECK(!remove_in_draw && !attached(list));CHECK(DestroyWindow(list));
        list=CreateWindowExW(0,WC_LISTVIEWW,L"",WS_CHILD|WS_VISIBLE|LVS_REPORT,0,0,600,180,root,(HMENU)13,cls.hInstance,NULL);CHECK(list && attached(list));
        destroy_in_draw=TRUE;(void)capture(list,bits);CHECK(!destroy_in_draw && !IsWindow(list));
        puts("LIST_REPORT_CALLBACK_RETIRE_OK remove=1 destroy=1");
    }
    printf("LIST_REPORT_OK bits=%u styled=%u aware=%u dpi=%u variants=18 native_font_geometry_data_callbacks=preserved\n",(unsigned)sizeof(void*)*8,styled,aware,GetDpiForWindow(root));
    CHECK(DestroyWindow(root));CHECK(UnregisterClassW(L"TListView",cls.hInstance));return 0;
}
static int worker(int argc,WCHAR **argv)
{
    CHECK(argc==2);WCHAR self[MAX_PATH],command[2048];CHECK(GetModuleFileNameW(NULL,self,MAX_PATH));
    for(unsigned aware=0;aware<2;++aware)for(unsigned styled=0;styled<2;++styled){
        if(styled)swprintf_s(command,2048,L"\"%ls\" --wait \"%ls\" --child %ls",argv[1],self,aware?L"styled-aware":L"styled");
        else swprintf_s(command,2048,L"\"%ls\" --child %ls",self,aware?L"native-aware":L"native");
        HANDLE job=CreateJobObjectW(NULL,NULL);JOBOBJECT_EXTENDED_LIMIT_INFORMATION limit={0};limit.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        CHECK(job && SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limit,sizeof(limit)));
        STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION pi={0};si.dwFlags=STARTF_USESTDHANDLES;si.hStdInput=GetStdHandle(STD_INPUT_HANDLE);si.hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE);si.hStdError=GetStdHandle(STD_ERROR_HANDLE);
        CHECK(CreateProcessW(NULL,command,NULL,NULL,TRUE,CREATE_NO_WINDOW|CREATE_SUSPENDED,NULL,NULL,&si,&pi));
        CHECK(AssignProcessToJobObject(job,pi.hProcess));ResumeThread(pi.hThread);DWORD code=1,wait=WaitForSingleObject(pi.hProcess,10000);
        if(wait==WAIT_OBJECT_0)GetExitCodeProcess(pi.hProcess,&code);
        CloseHandle(pi.hThread);CloseHandle(pi.hProcess);CloseHandle(job);CHECK(wait==WAIT_OBJECT_0 && !code);
    }
    puts("LIST_REPORT_CLEANUP_OK");return 0;
}
int wmain(int argc,WCHAR **argv)
{if(argc==3 && !wcscmp(argv[1],L"--child"))return child(!wcsncmp(argv[2],L"styled",6),wcsstr(argv[2],L"aware")!=NULL);return isolated_process(argc,argv,worker);}
