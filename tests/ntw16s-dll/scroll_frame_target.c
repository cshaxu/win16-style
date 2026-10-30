/* Verify shared frame scrollbar redraw after native position APIs. */
#define UNICODE
#define _UNICODE
#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <dwmapi.h>
#include <stdio.h>
#include <string.h>
typedef BOOL (__cdecl *install_fn)(HWND,const void *);
typedef void (__cdecl *paint_fn)(HWND,HDC,const void *);
typedef BOOL (__cdecl *attached_fn)(HWND);
typedef BOOL (__cdecl *enable_fn)(HWND);
static BOOL snap(int x,int y,int width,int height,DWORD *out)
{
    HDC dc=GetDC(NULL),mem=dc?CreateCompatibleDC(dc):NULL;BITMAPINFO bi={0};
    bi.bmiHeader.biSize=sizeof(bi.bmiHeader);bi.bmiHeader.biWidth=width;bi.bmiHeader.biHeight=-height;
    bi.bmiHeader.biPlanes=1;bi.bmiHeader.biBitCount=32;void *bits=NULL;
    HBITMAP bmp=mem?CreateDIBSection(dc,&bi,DIB_RGB_COLORS,&bits,NULL,0):NULL;BOOL ok=FALSE;
    if(bmp&&bits){HGDIOBJ old=SelectObject(mem,bmp);ok=BitBlt(mem,0,0,width,height,dc,x,y,SRCCOPY|CAPTUREBLT);
        GdiFlush();if(ok)memcpy(out,bits,(size_t)width*height*4);SelectObject(mem,old);}
    if(bmp)DeleteObject(bmp);
    if(mem)DeleteDC(mem);
    if(dc)ReleaseDC(NULL,dc);
    return ok;
}
static int check_axis(HWND root,HWND child,int bar,paint_fn paint)
{
    LONG object=bar==SB_VERT?OBJID_VSCROLL:OBJID_HSCROLL;SCROLLBARINFO info={.cbSize=sizeof(info)};
    if(!GetScrollBarInfo(child,object,&info))return 2;
    POINT a={info.rcScrollBar.left,info.rcScrollBar.top},b={info.rcScrollBar.right,info.rcScrollBar.bottom};
    POINT arrow;if(bar==SB_VERT){arrow.x=(a.x+b.x)/2;arrow.y=b.y-info.dxyLineButton/2;}
    else{arrow.x=b.x-info.dxyLineButton/2;arrow.y=(a.y+b.y)/2;}
    POINT hitpt=arrow,pa=a,pb=b;HWND hit=WindowFromPoint(hitpt);DWORD pid=0;
    if(hit)GetWindowThreadProcessId(hit,&pid);
    if(pid!=GetCurrentProcessId() || GetAncestor(hit,GA_ROOT)!=root){printf("FRAME_VISUAL_SKIP axis=%d reason=occluded\n",bar);return 77;}
    if(!LogicalToPhysicalPointForPerMonitorDPI(child,&pa)||!LogicalToPhysicalPointForPerMonitorDPI(child,&pb))return 3;
    int width=pb.x-pa.x,height=pb.y-pa.y;if(width<=0||height<=0||width>2048||height>2048||width*height>32768)return 4;
    DWORD immediate[32768],before[32768],after[32768];SCROLLINFO change={.cbSize=sizeof(change),.fMask=SIF_POS,.nPos=45};
    POINT end=hitpt;ScreenToClient(child,&end);unsigned failures=0;
    POINT thumb={0},move={0};LONG track_start=0;BOOL drag_started=FALSE,drag_changed=FALSE;
    const char *labels[]={"setinfo","setpos","setrange","press","release","wheel","thumb-down","thumb-move","thumb-up","hover","leave"};
    for(unsigned step=0;step<ARRAYSIZE(labels);++step){
        switch(step){
        case 0:SetScrollInfo(child,bar,&change,TRUE);break;
        case 1:SetScrollPos(child,bar,55,TRUE);break;
        case 2:SetScrollRange(child,bar,0,120,TRUE);break;
        case 3:SendMessageW(child,WM_NCLBUTTONDOWN,bar==SB_VERT?HTVSCROLL:HTHSCROLL,MAKELPARAM(hitpt.x,hitpt.y));break;
        case 4:SendMessageW(child,WM_LBUTTONUP,0,MAKELPARAM(end.x,end.y));break;
        case 5:SendMessageW(child,bar==SB_VERT?WM_MOUSEWHEEL:WM_MOUSEHWHEEL,MAKEWPARAM(0,WHEEL_DELTA),MAKELPARAM(hitpt.x,hitpt.y));break;
        case 6:{SCROLLBARINFO current={.cbSize=sizeof(current)};if(!GetScrollBarInfo(child,object,&current))return 8;
            SCROLLINFO range={.cbSize=sizeof(range),.fMask=SIF_RANGE|SIF_PAGE|SIF_POS};if(!GetScrollInfo(child,bar,&range))return 8;
            int length=bar==SB_VERT?current.rcScrollBar.bottom-current.rcScrollBar.top:current.rcScrollBar.right-current.rcScrollBar.left;
            int cross=bar==SB_VERT?current.rcScrollBar.right-current.rcScrollBar.left:current.rcScrollBar.bottom-current.rcScrollBar.top;
            int border=MulDiv(1,(int)GetDpiForWindow(child),96);if(border<1)border=1;
            int origin=cross-border,travel=length-cross-origin+border-cross;
            LONG maximum=range.nMax-(range.nPage?range.nPage-1:0);if(maximum<range.nMin)maximum=range.nMin;
            LONG position=range.nPos;if(position<range.nMin)position=range.nMin;if(position>maximum)position=maximum;
            int thumb_offset=origin+cross/2;if(maximum>range.nMin && travel>0)thumb_offset+=(int)(((LONGLONG)(position-range.nMin)*travel+(maximum-range.nMin)/2)/(maximum-range.nMin));
            thumb.x=bar==SB_VERT?(current.rcScrollBar.left+current.rcScrollBar.right)/2:current.rcScrollBar.left+thumb_offset;
            thumb.y=bar==SB_VERT?current.rcScrollBar.top+thumb_offset:(current.rcScrollBar.top+current.rcScrollBar.bottom)/2;
            move=thumb;if(bar==SB_VERT)move.y+=15;else move.x+=15;ScreenToClient(child,&move);
            SendMessageW(child,WM_NCLBUTTONDOWN,bar==SB_VERT?HTVSCROLL:HTHSCROLL,MAKELPARAM(thumb.x,thumb.y));
            SCROLLINFO tracking={.cbSize=sizeof(tracking),.fMask=SIF_TRACKPOS};drag_started=GetCapture()==child && GetScrollInfo(child,bar,&tracking);track_start=tracking.nTrackPos;break;}
        case 7:{SendMessageW(child,WM_MOUSEMOVE,MK_LBUTTON,MAKELPARAM(move.x,move.y));
            SCROLLINFO tracking={.cbSize=sizeof(tracking),.fMask=SIF_TRACKPOS};drag_changed=GetCapture()==child && GetScrollInfo(child,bar,&tracking) && tracking.nTrackPos!=track_start;break;}
        case 8:SendMessageW(child,WM_LBUTTONUP,0,MAKELPARAM(move.x,move.y));break;
        case 9:SendMessageW(child,WM_NCMOUSEMOVE,bar==SB_VERT?HTVSCROLL:HTHSCROLL,MAKELPARAM(hitpt.x,hitpt.y));break;
        default:SendMessageW(child,WM_NCMOUSELEAVE,0,0);break;
        }
        if(!snap(pa.x,pa.y,width,height,immediate))return 5;
        DwmFlush();if(!snap(pa.x,pa.y,width,height,before))return 5;
        unsigned transient=0;for(int i=0;i<width*height;++i)if((immediate[i]&0xffffff)!=(before[i]&0xffffff))++transient;
        printf("FRAME_TRANSIENT axis=%d action=%s diff=%u pixels=%d\n",bar,labels[step],transient,width*height);
        paint(child,NULL,NULL);DwmFlush();if(!snap(pa.x,pa.y,width,height,after))return 6;
        HWND now=WindowFromPoint(hitpt);DWORD now_pid=0;if(now)GetWindowThreadProcessId(now,&now_pid);
        if(now_pid!=GetCurrentProcessId() || GetAncestor(now,GA_ROOT)!=root){printf("FRAME_VISUAL_SKIP axis=%d reason=occluded\n",bar);return 77;}
        unsigned diff=0;for(int i=0;i<width*height;++i)if((before[i]&0xffffff)!=(after[i]&0xffffff))++diff;
        printf("FRAME_AXIS axis=%d action=%s diff=%u pixels=%d\n",bar,labels[step],diff,width*height);
        if(diff)++failures;
    }
    printf("FRAME_DRAG axis=%d captured=%d moved=%d\n",bar,drag_started,drag_changed);
    return failures||!drag_started||!drag_changed?1:0;
}
int wmain(int argc,WCHAR **argv)
{
    BOOL list=argc>1 && !lstrcmpW(argv[1],L"list"),edit=argc>1 && !lstrcmpW(argv[1],L"edit");
    HMODULE dll=GetModuleHandleW(sizeof(void*)==4?L"ntw16s32.dll":L"ntw16s64.dll");if(!dll)return 2;
    FARPROC address=GetProcAddress(dll,"win16_style_frame_scrollbars_install");install_fn install=NULL;memcpy(&install,&address,sizeof(install));
    address=GetProcAddress(dll,"win16_style_scrollbars_paint");paint_fn paint=NULL;memcpy(&paint,&address,sizeof(paint));
    address=GetProcAddress(dll,"Ntw16sWindowAttached");attached_fn attached=NULL;memcpy(&attached,&address,sizeof(attached));
    address=GetProcAddress(dll,list?"win16_style_list_scrollbar_enable":"win16_style_edit_scrollbars_enable");
    enable_fn enable=NULL;memcpy(&enable,&address,sizeof(enable));
    if(!install||!paint||!attached||((list||edit)&&!enable))return 3;
    WNDCLASSW cls={0};cls.lpfnWndProc=DefWindowProcW;cls.hInstance=GetModuleHandleW(NULL);cls.lpszClassName=L"Win16FrameScrollProbe";
    if(!RegisterClassW(&cls))return 4;
    HWND root=CreateWindowExW(WS_EX_TOPMOST,cls.lpszClassName,L"Win16 scrollbar probe",WS_OVERLAPPEDWINDOW|WS_VISIBLE,50,50,480,360,NULL,NULL,cls.hInstance,NULL);
    const WCHAR *child_class=list?L"LISTBOX":edit?L"EDIT":cls.lpszClassName;
    DWORD child_style=WS_CHILD|WS_VISIBLE|WS_HSCROLL|WS_VSCROLL|(list?LBS_NOTIFY:edit?(ES_MULTILINE|ES_AUTOHSCROLL|ES_AUTOVSCROLL):0);
    HWND child=CreateWindowExW(0,child_class,L"",child_style,20,20,350,220,root,NULL,cls.hInstance,NULL);
    if(!root||!child)return 5;
    if(list){for(int i=0;i<80;++i){WCHAR line[100];swprintf(line,ARRAYSIZE(line),L"Item %02d - long horizontal content",i);SendMessageW(child,LB_ADDSTRING,0,(LPARAM)line);}SendMessageW(child,LB_SETHORIZONTALEXTENT,1000,0);}
    if(edit){WCHAR text[12000]=L"";for(int i=0;i<100;++i)lstrcatW(text,L"line with long horizontal content long horizontal content long horizontal content\r\n");SetWindowTextW(child,text);}
    SCROLLINFO si={.cbSize=sizeof(si),.fMask=SIF_RANGE|SIF_PAGE|SIF_POS,.nMin=0,.nMax=100,.nPage=10,.nPos=10};
    SetScrollInfo(child,SB_HORZ,&si,TRUE);SetScrollInfo(child,SB_VERT,&si,TRUE);
    BOOL before=attached(child),installed=(list||edit)?enable(child):install(child,NULL);DWORD error=GetLastError();
    printf("FRAME_HOST bits=%u attached=%d installed=%d error=%lu kind=%ls\n",(unsigned)(sizeof(void*)*8),before,installed,error,list?L"list":edit?L"edit":L"frame");
    if(!installed)return 6;
    SetWindowPos(root,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);RedrawWindow(root,NULL,NULL,RDW_INVALIDATE|RDW_ALLCHILDREN|RDW_UPDATENOW);paint(child,NULL,NULL);DwmFlush();Sleep(300);DwmFlush();
    int horizontal=check_axis(root,child,SB_HORZ,paint),vertical=check_axis(root,child,SB_VERT,paint);
    DestroyWindow(root);UnregisterClassW(cls.lpszClassName,cls.hInstance);
    printf("FRAME_SCROLL_RESULT horizontal=%d vertical=%d\n",horizontal,vertical);
    if(horizontal==1 || vertical==1)return 1;
    if(horizontal==77 || vertical==77)return 77;
    return horizontal||vertical?1:0;
}
