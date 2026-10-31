#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commdlg.h>
#include <dlgs.h>
#include <stdio.h>
static BOOL (__cdecl *attached)(HWND);
static BOOL cancel_mode,font_mode,color_mode,color_expand,color_keyboard_mode;
static BOOL color_expand_ok;
static BOOL font_controls_ok,color_grid_ok,color_edge_ok,color_keyboard_ok,color_unrelated_ok;
static COLORREF color_expected,color_result;
static int inits,ticks;
static BOOL frame_attached;
static BOOL capture_dialog(HWND window,PCWSTR path)
{
    RECT bounds;if(!GetWindowRect(window,&bounds))return FALSE;int width=bounds.right-bounds.left,height=bounds.bottom-bounds.top;
    if(width<=0||height<=0||width>4096||height>4096)return FALSE;
    BITMAPINFO info={0};info.bmiHeader.biSize=sizeof(info.bmiHeader);info.bmiHeader.biWidth=width;info.bmiHeader.biHeight=-height;
    info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;HDC screen=GetDC(window),memory=screen?CreateCompatibleDC(screen):NULL;
    void *pixels=NULL;HBITMAP bitmap=memory?CreateDIBSection(screen,&info,DIB_RGB_COLORS,&pixels,NULL,0):NULL;
    HGDIOBJ previous=bitmap?SelectObject(memory,bitmap):NULL;BOOL ok=FALSE;
    if(previous&&previous!=HGDI_ERROR&&pixels&&PrintWindow(window,memory,0)){
        DWORD size=(DWORD)((size_t)width*(size_t)height*4),written;BITMAPFILEHEADER header={0};header.bfType=0x4d42;
        header.bfOffBits=sizeof(header)+sizeof(info.bmiHeader);header.bfSize=header.bfOffBits+size;
        HANDLE file=CreateFileW(path,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
        if(file!=INVALID_HANDLE_VALUE){ok=WriteFile(file,&header,sizeof(header),&written,NULL)&&written==sizeof(header)&&
            WriteFile(file,&info.bmiHeader,sizeof(info.bmiHeader),&written,NULL)&&written==sizeof(info.bmiHeader)&&
            WriteFile(file,pixels,size,&written,NULL)&&written==size;CloseHandle(file);}
    }
    if(previous&&previous!=HGDI_ERROR)SelectObject(memory,previous);
    if(bitmap)DeleteObject(bitmap);
    if(memory)DeleteDC(memory);
    if(screen)ReleaseDC(window,screen);
    return ok;
}
static UINT_PTR CALLBACK app_hook(HWND window,UINT message,WPARAM wparam,LPARAM lparam)
{
    (void)lparam;
    if(message==WM_INITDIALOG){++inits;SetTimer(window,916,100,NULL);}
    if(message==WM_TIMER && wparam==916){
        ++ticks;KillTimer(window,916);frame_attached=attached&&attached(window);
        if(color_mode){
            HDC screen=GetDC(window),memory=screen?CreateCompatibleDC(screen):NULL;
            HBITMAP bitmap=memory?CreateCompatibleBitmap(screen,20,20):NULL;HGDIOBJ previous=bitmap?SelectObject(memory,bitmap):NULL;
            RECT other_edge={0,0,16,16};
            if(previous&&previous!=HGDI_ERROR)color_unrelated_ok=DrawEdge(memory,&other_edge,EDGE_SUNKEN,BF_RECT|BF_ADJUST)&&other_edge.left==2&&other_edge.top==2&&other_edge.right==14&&other_edge.bottom==14;
            if(previous&&previous!=HGDI_ERROR)SelectObject(memory,previous);
            if(bitmap)DeleteObject(bitmap);
            if(memory)DeleteDC(memory);
            if(screen)ReleaseDC(window,screen);
            printf("COLOR_UNRELATED native=%d\n",color_unrelated_ok);
            HWND grid=GetDlgItem(window,720);RECT r={0};HDC dc=grid?GetDC(grid):NULL;
            if(dc && GetClientRect(grid,&r) && r.right>=16 && r.bottom>=12){
                int colorful=0;
                for(int row=0;row<6;++row)for(int col=0;col<8;++col){
                    int x=(2*col+1)*r.right/16,y=(2*row+1)*r.bottom/12;
                    COLORREF c=GetPixel(dc,x,y);
                    if(c!=CLR_INVALID && (GetRValue(c)!=GetGValue(c) || GetGValue(c)!=GetBValue(c)))++colorful;
                }
                int x=r.right/16,y=r.bottom/12;color_expected=GetPixel(dc,x,y);
                color_grid_ok=colorful>=8 && color_expected!=CLR_INVALID && color_expected!=RGB(0,0,0);
                int dpi=GetDeviceCaps(dc,LOGPIXELSX),bx=MulDiv(4,dpi,96),by=MulDiv(3,dpi,96);
                COLORREF corner=GetPixel(dc,bx,by),inner=GetPixel(dc,bx+1,by+1);
                printf("COLOR_EDGE corner=%08lx inner=%08lx center=%08lx at=%d,%d dpi=%d\n",(DWORD)corner,(DWORD)inner,(DWORD)color_expected,bx,by,dpi);
                HWND other=GetDlgItem(window,721);HDC other_dc=other?GetDC(other):NULL;
                if(other_dc){COLORREF custom_corner=GetPixel(other_dc,bx,by),custom_inner=GetPixel(other_dc,bx+1,by+1);
                    color_edge_ok=dpi==96&&corner==RGB(0,0,0)&&inner==color_expected&&custom_corner==RGB(0,0,0)&&custom_inner==RGB(12,34,56);
                    printf("COLOR_CUSTOM_EDGE corner=%08lx inner=%08lx flat=%d\n",(DWORD)custom_corner,(DWORD)custom_inner,color_edge_ok);ReleaseDC(other,other_dc);}
                POINT point={x,y};MapWindowPoints(grid,window,&point,1);
                SendMessageW(window,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(point.x,point.y));
                SendMessageW(window,WM_LBUTTONUP,0,MAKELPARAM(point.x,point.y));
                SetFocus(grid);color_keyboard_ok=GetFocus()==grid;
                SendMessageW(grid,WM_KEYDOWN,VK_RIGHT,0);SendMessageW(grid,WM_KEYUP,VK_RIGHT,0);
                if(color_keyboard_mode){SendMessageW(grid,WM_CHAR,VK_SPACE,0);color_expected=GetPixel(dc,3*r.right/16,y);}
                else{SendMessageW(grid,WM_KEYDOWN,VK_LEFT,0);SendMessageW(grid,WM_KEYUP,VK_LEFT,0);}
                printf("COLOR_KEYBOARD focus=%d\n",color_keyboard_ok);
                printf("COLOR_GRID colorful=%d expected=%08lx\n",colorful,(DWORD)color_expected);
            }
            if(dc)ReleaseDC(grid,dc);
            if(color_expand){
                HWND more=GetDlgItem(window,719);
                if(more)SendMessageW(window,WM_COMMAND,MAKEWPARAM(719,BN_CLICKED),(LPARAM)more);
                RedrawWindow(window,NULL,NULL,RDW_INVALIDATE|RDW_ALLCHILDREN|RDW_UPDATENOW);
                HWND gradient=GetDlgItem(window,710),red=GetDlgItem(window,706);RECT bounds={0};
                HDC paint=gradient?GetDC(gradient):NULL;
                if(paint && IsWindowVisible(gradient) && GetClientRect(gradient,&bounds) && bounds.right>20 && bounds.bottom>20){
                    COLORREF a=GetPixel(paint,bounds.right/8,bounds.bottom/8);
                    COLORREF b=GetPixel(paint,bounds.right/2,bounds.bottom/8);
                    WCHAR value[16]=L"";
                    if(red){SetWindowTextW(red,L"123");GetWindowTextW(red,value,16);}
                    color_expand_ok=a!=CLR_INVALID && b!=CLR_INVALID && a!=b && !lstrcmpW(value,L"123");
                    printf("COLOR_EXPANDED a=%08lx b=%08lx edit=%ls\n",(DWORD)a,(DWORD)b,value);
                }
                if(paint)ReleaseDC(gradient,paint);
            }
            WCHAR capture_path[MAX_PATH];DWORD capture_length=GetEnvironmentVariableW(L"WIN16S_COLOR_CAPTURE",capture_path,ARRAYSIZE(capture_path));
            if(capture_length&&capture_length<ARRAYSIZE(capture_path))printf("COLOR_CAPTURE saved=%d\n",capture_dialog(window,capture_path));
        }
        if(font_mode){
            HWND names=GetDlgItem(window,cmb1),style=GetDlgItem(window,cmb2),script=GetDlgItem(window,cmb5);
            COMBOBOXINFO info={.cbSize=sizeof(info)};
            LRESULT count=SendMessageW(style,CB_GETCOUNT,0,0),original=SendMessageW(style,CB_GETCURSEL,0,0);
            LRESULT changed=count>1?SendMessageW(style,CB_SETCURSEL,(original+1)%count,0):CB_ERR;
            if(original!=CB_ERR)SendMessageW(style,CB_SETCURSEL,original,0);
            LRESULT selected=SendMessageW(style,CB_GETCURSEL,0,0),script_before=SendMessageW(script,CB_GETCURSEL,0,0);
            SendMessageW(script,CB_SHOWDROPDOWN,TRUE,0);
            LRESULT opened=SendMessageW(script,CB_GETDROPPEDSTATE,0,0);
            SendMessageW(script,CB_SHOWDROPDOWN,FALSE,0);
            LRESULT script_after=SendMessageW(script,CB_GETCURSEL,0,0),before=LB_ERR,after=LB_ERR;
            if(GetComboBoxInfo(names,&info) && info.hwndList){
                before=SendMessageW(info.hwndList,LB_GETTOPINDEX,0,0);
                SendMessageW(info.hwndList,WM_VSCROLL,SB_LINEDOWN,0);
                after=SendMessageW(info.hwndList,LB_GETTOPINDEX,0,0);
            }
            font_controls_ok=count>1 && changed!=CB_ERR && selected==original &&
                script_before==script_after && opened && before>=0 && after>before;
            printf("FONT_CONTROLS choices=%lld restored=%d drop=%lld list_top=%lld->%lld\n",
                (long long)count,selected==original,(long long)opened,(long long)before,(long long)after);
        }
        PostMessageW(window,WM_COMMAND,cancel_mode?IDCANCEL:IDOK,0);
    }
    return 0;
}
int wmain(int argc,WCHAR **argv)
{
    if(argc!=4)return 2;
    BOOL font=argv[1][0]==L'f',ansi=argv[2][0]==L'a';font_mode=font;color_mode=!font;color_expand=color_mode && argv[3][0]==L'e';color_keyboard_mode=color_mode&&argv[3][0]==L'k';cancel_mode=argv[3][0]==L'c' || color_expand;
    HMODULE module=GetModuleHandleW(sizeof(void*)==4?L"ntw16s32.dll":L"ntw16s64.dll");
    attached=module?(BOOL (__cdecl *)(HWND))(void *)GetProcAddress(module,"Ntw16sWindowAttached"):NULL;
    BOOL result;COLORREF custom[16]={0};custom[0]=RGB(12,34,56);LOGFONTA la={0};LOGFONTW lw={0};
    if(font && ansi){CHOOSEFONTA x={0};x.lStructSize=sizeof(x);x.lpLogFont=&la;x.Flags=CF_SCREENFONTS|CF_ENABLEHOOK;x.lpfnHook=app_hook;result=ChooseFontA(&x);}
    else if(font){CHOOSEFONTW x={0};x.lStructSize=sizeof(x);x.lpLogFont=&lw;x.Flags=CF_SCREENFONTS|CF_ENABLEHOOK;x.lpfnHook=app_hook;result=ChooseFontW(&x);}
    else if(ansi){CHOOSECOLORA x={0};x.lStructSize=sizeof(x);x.lpCustColors=custom;x.rgbResult=RGB(0,0,0);x.Flags=CC_RGBINIT|CC_ENABLEHOOK;x.lpfnHook=app_hook;result=ChooseColorA(&x);color_result=x.rgbResult;}
    else{CHOOSECOLORW x={0};x.lStructSize=sizeof(x);x.lpCustColors=custom;x.rgbResult=RGB(0,0,0);x.Flags=CC_RGBINIT|CC_ENABLEHOOK;x.lpfnHook=app_hook;result=ChooseColorW(&x);color_result=x.rgbResult;}
    DWORD error=CommDlgExtendedError();
    if(color_mode)printf("COLOR_RESULT value=%08lx expected=%08lx valid=%d\n",(DWORD)color_result,(DWORD)color_expected,color_grid_ok);
    printf("COMMON_DIALOG bits=%u font=%d ansi=%d cancel=%d result=%d error=%lu inits=%d ticks=%d frame=%d\n",(unsigned)(sizeof(void*)*8),font,ansi,cancel_mode,result,error,inits,ticks,frame_attached);
    return inits==1 && ticks==1 && frame_attached && (!font || font_controls_ok) && (!color_mode || (color_grid_ok && color_edge_ok && color_keyboard_ok && color_unrelated_ok && (!color_expand || color_expand_ok) && (cancel_mode || color_result==color_expected))) && !error && (cancel_mode?!result:result)?0:1;
}
