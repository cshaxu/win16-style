/* Win2000 statctl.c xxxStaticPaint / StaticCallback. Images keep their
 * original native painter and ownership through the common basic protocol. */
#include "static_paint.h"
#include "../common/paint.h"
#include "lib-win32-style/application/control/button_snapshot.h"
#include <wchar.h>
typedef struct static_text {const WCHAR *text;UINT flags;} static_text;
static BOOL CALLBACK draw_text(HDC dc,LPARAM value,WPARAM length,int width,int height)
{
    const static_text *text=(const static_text *)value;RECT r={0,0,width,height};(void)length;
    return DrawTextW(dc,text->text,-1,&r,text->flags)!=0;
}
static style_control_button_role role(HWND window,void *context,DWORD style)
{(void)window;(void)context;return (style_control_button_role){style&SS_TYPEMASK,TRUE};}
static HFONT font(HWND window,void *context)
{(void)context;HFONT value=(HFONT)SendMessageW(window,WM_GETFONT,0,0);return value?value:(HFONT)GetStockObject(SYSTEM_FONT);}
BOOL win2k_static_paint(HWND window,style_instance *instance,HDC dc,const win2k_palette *p,style_line_profile separator)
{
    int saved=SaveDC(dc);if(!saved)return FALSE;
    BOOL result=FALSE;RECT client;HWND parent=GetParent(window);
    const style_control_button_reader reader={NULL,role,font};style_control_button_snapshot snapshot={0};
    DWORD initial=(DWORD)GetWindowLongPtrW(window,GWL_STYLE);UINT type=initial&SS_TYPEMASK;
    BOOL textual=type==SS_LEFT||type==SS_CENTER||type==SS_RIGHT||type==SS_SIMPLE||type==SS_LEFTNOWORDWRAP;
    if(textual){
        if(!style_control_button_snapshot_read(window,instance,&reader,&snapshot))goto done;
        HGDIOBJ old=SelectObject(dc,snapshot.font);if(!old||old==HGDI_ERROR)goto done;
    }else{snapshot.style=initial;snapshot.role=role(window,NULL,initial);}
    if(!GetClientRect(window,&client))goto done;
    HBRUSH brush=win2k_control_brush(window,instance,dc,p);if(!brush)goto done;
    UINT cues=textual?(UINT)SendMessageW(window,WM_QUERYUISTATE,0,0):0;
    if(instance->detached||!IsWindow(window)||GetParent(window)!=parent||
       (DWORD)GetWindowLongPtrW(window,GWL_STYLE)!=snapshot.style||
       IntersectClipRect(dc,client.left,client.top,client.right,client.bottom)==ERROR)goto done;
    if(type==SS_LEFT||type==SS_CENTER||type==SS_RIGHT||type==SS_LEFTNOWORDWRAP){
        if(!FillRect(dc,&client,brush))goto done;
        UINT flags=DT_NOCLIP|DT_EXPANDTABS;
        if(type!=SS_LEFTNOWORDWRAP){flags|=DT_WORDBREAK|type;if(snapshot.style&SS_EDITCONTROL)flags|=DT_EDITCONTROL;}
        switch(snapshot.style&SS_WORDELLIPSIS){
        case SS_ENDELLIPSIS:flags|=DT_END_ELLIPSIS|DT_SINGLELINE;break;
        case SS_PATHELLIPSIS:flags|=DT_PATH_ELLIPSIS|DT_SINGLELINE;break;
        case SS_WORDELLIPSIS:flags|=DT_WORD_ELLIPSIS|DT_SINGLELINE;break;
        }
        if(snapshot.style&SS_NOPREFIX)flags|=DT_NOPREFIX;
        if(snapshot.style&SS_CENTERIMAGE)flags|=DT_VCENTER|DT_SINGLELINE;
        if(cues&UISF_HIDEACCEL)flags|=DT_HIDEPREFIX;
        static_text text={snapshot.text,flags};
        result=!*snapshot.text || win2k_draw_content(dc,(LPARAM)&text,0,client.left,client.top,client.right-client.left,client.bottom-client.top,
            DST_COMPLEX,draw_text,(snapshot.style&WS_DISABLED)!=0,FALSE,p);
    }else if(type==SS_SIMPLE){
        if(snapshot.style&SS_NOPREFIX)result=ExtTextOutW(dc,client.left,client.top,ETO_OPAQUE|ETO_CLIPPED,&client,snapshot.text,(UINT)wcslen(snapshot.text),NULL);
        else result=!*snapshot.text || DrawTextW(dc,snapshot.text,-1,&client,DT_SINGLELINE|((cues&UISF_HIDEACCEL)?DT_HIDEPREFIX:0));
    }else if(type>=SS_BLACKRECT && type<=SS_WHITEFRAME){
        unsigned color=(type-SS_BLACKRECT)%3;COLORREF value=color==0?p->dark:color==1?p->shadow:p->highlight;
        if(type<=SS_WHITERECT)result=style_paint_color(dc,&client,value);
        else {HBRUSH frame=CreateSolidBrush(value);if(frame){result=FrameRect(dc,&client,frame)!=0;DeleteObject(frame);}}
    }else if(type>=SS_ETCHEDHORZ && type<=SS_ETCHEDFRAME){
        int unit=max(1,MulDiv(1,(int)GetDpiForWindow(window),96));
        result=style_paint_separator(dc,client,type,separator?separator:STYLE_LINE_RELIEF,
            unit,p->dark,p->shadow,p->highlight);
    }
done:
    if(!RestoreDC(dc,saved))result=FALSE;
    style_control_button_snapshot_clear(&snapshot);return result;
}
