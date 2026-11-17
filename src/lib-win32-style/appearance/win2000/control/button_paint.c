/* Win2000 USER: btnctl.c xxxBNInitDC/BNCalcRect/xxxBNDrawText and
 * xxxButtonDrawCheck. Pixel references and the finite contract are in T27. */
#include "button_paint.h"
#include "background.h"
#include "../common/paint.h"
#include "lib-win32-style/application/control/button_snapshot.h"
#include <wchar.h>

static style_control_button_role role(HWND window,void *context,DWORD style)
{(void)window;(void)context;return (style_control_button_role){style&BS_TYPEMASK,FALSE};}
static HFONT font(HWND window,void *context)
{
    (void)context;HFONT value=(HFONT)SendMessageW(window,WM_GETFONT,0,0);
    return value?value:(HFONT)GetStockObject(SYSTEM_FONT);
}
static BOOL push_style(DWORD style)
{return (style&BS_TYPEMASK)<=BS_DEFPUSHBUTTON || (style&BS_PUSHLIKE);}

typedef struct button_text {const WCHAR *value;UINT format;} button_text;
static BOOL CALLBACK multiline(HDC dc,LPARAM value,WPARAM length,int width,int height)
{
    const button_text *text=(const button_text *)value;RECT r={0,0,width,height};(void)length;
    return DrawTextW(dc,text->value,-1,&r,text->format)!=0;
}
static BOOL label(HWND window,style_instance *instance,HDC dc,const style_control_button_snapshot *s,
    RECT bounds,const RECT *client,BOOL push,UINT cues,const win2k_palette *p)
{
    int width=0,height=0;UINT type;LPARAM value;WPARAM length=0;DRAWSTATEPROC draw=NULL;
    button_text text={s->text,DT_WORDBREAK|DT_EDITCONTROL};style_control_button_image image;
    if(cues&UISF_HIDEACCEL)text.format|=DT_HIDEPREFIX;
    UINT horizontal=s->style&BS_CENTER,vertical=s->style&BS_VCENTER;
    if(!horizontal)horizontal=push?BS_CENTER:BS_LEFT;
    if(horizontal==BS_RIGHT)text.format|=DT_RIGHT;
    else if(horizontal!=BS_LEFT)text.format|=DT_CENTER;
    if(vertical==BS_BOTTOM)text.format|=DT_BOTTOM;
    else if(vertical!=BS_TOP)text.format|=DT_VCENTER;
    if(s->style&(BS_ICON|BS_BITMAP)){
        UINT kind=(s->style&BS_BITMAP)?IMAGE_BITMAP:IMAGE_ICON;
        if(!style_control_button_image_read(window,instance,kind,&image) || instance->detached || image.listed)return FALSE;
        if(!image.image)return TRUE;
        width=image.size.cx;height=image.size.cy;value=(LPARAM)image.image;
        type=kind==IMAGE_ICON?DST_ICON:DST_BITMAP;
    }else{
        if(!*s->text)return TRUE;
        BOOL multi=(s->style&BS_MULTILINE)!=0;
        RECT measured={0,0,max(0,bounds.right-bounds.left-(multi?4:0)),max(0,bounds.bottom-bounds.top-(multi?2:0))};
        UINT format=multi?text.format:DT_SINGLELINE;
        if(!DrawTextW(dc,s->text,-1,&measured,format|DT_CALCRECT))return FALSE;
        width=measured.right-measured.left;height=measured.bottom-measured.top+1;
        value=(LPARAM)s->text;length=wcslen(s->text);type=DST_PREFIXTEXT;
        if(cues&UISF_HIDEACCEL)type|=DSS_HIDEPREFIX;
        if(multi){type=DST_COMPLEX;value=(LPARAM)&text;draw=multiline;}
    }
    int x=horizontal==BS_LEFT?bounds.left+2:horizontal==BS_RIGHT?bounds.right-width-2:(bounds.left+bounds.right-width)/2;
    int y=vertical==BS_TOP?bounds.top+1:vertical==BS_BOTTOM?bounds.bottom-height-1:(bounds.top+bounds.bottom-height)/2;
    if(push && (s->state&BST_PUSHED)){++x;++y;}
    BOOL result=win2k_draw_content(dc,value,length,x,y,width,height,type,draw,(s->style&WS_DISABLED)!=0,
        push && s->check==BST_INDETERMINATE,p);
    if(result && (s->state&BST_FOCUS) && !(cues&UISF_HIDEFOCUS)){
        if(push)InflateRect(&bounds,-1,-1);
        else{
            bounds.left=max(client->left,x-1);bounds.top=max(client->top,y-1);
            bounds.right=min(client->right,bounds.left+width+2);bounds.bottom=min(client->bottom,bounds.top+height+2);
        }
        result=DrawFocusRect(dc,&bounds);
    }
    return result;
}
BOOL win2k_button_paint(HWND window,style_instance *instance,HDC dc,const win2k_palette *p)
{
    int saved=SaveDC(dc);if(!saved)return FALSE;
    BOOL result=FALSE;RECT client,bounds,glyph;HBRUSH brush=NULL;
    DWORD initial_style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE);HWND parent=GetParent(window);
    const style_control_button_reader reader={NULL,role,font};style_control_button_snapshot snapshot={0};
    /* Later image-list-only content remains native; never silently drop it. */
    if(!(initial_style&(BS_ICON|BS_BITMAP))){
        BUTTON_IMAGELIST images={0};BOOL listed=(BOOL)SendMessageW(window,BCM_GETIMAGELIST,0,(LPARAM)&images);
        if(instance->detached || (listed && images.himl && images.himl!=BCCL_NOGLYPH))goto done;
    }
    if(!GetClientRect(window,&client))goto done;
    BOOL group=(initial_style&BS_TYPEMASK)==BS_GROUPBOX;
    if(!push_style(initial_style)){
        brush=win2k_control_brush(window,instance,dc,p);if(!brush)goto done;
        if(!group && !FillRect(dc,&client,brush))goto done;
    }
    if(!style_control_button_snapshot_read(window,instance,&reader,&snapshot))goto done;
    UINT cues=(UINT)SendMessageW(window,WM_QUERYUISTATE,0,0);
    if(instance->detached || !IsWindow(window) || initial_style!=snapshot.style ||
       (DWORD)GetWindowLongPtrW(window,GWL_STYLE)!=snapshot.style || GetParent(window)!=parent)goto done;
    HGDIOBJ previous=SelectObject(dc,snapshot.font);if(!previous || previous==HGDI_ERROR)goto done;
    if(IntersectClipRect(dc,client.left,client.top,client.right,client.bottom)==ERROR)goto done;
    if(group){
        SIZE zero;TEXTMETRICW system;RECT text={0};HGDIOBJ old=SelectObject(dc,GetStockObject(SYSTEM_FONT));
        if(!old||old==HGDI_ERROR||!GetTextMetricsW(dc,&system)||!SelectObject(dc,old)||!GetTextExtentPoint32W(dc,L"0",1,&zero))goto done;
        bounds=client;bounds.top+=zero.cy/2;
        if(!win2k_edge(dc,&bounds,EDGE_ETCHED,BF_RECT|((snapshot.style&BS_FLAT)?BF_FLAT|BF_MONO:0),p))goto done;
        if(*snapshot.text){
            if(!DrawTextW(dc,snapshot.text,-1,&text,DT_CALCRECT|DT_SINGLELINE))goto done;
            int width=text.right-text.left+4;bounds=client;
            UINT align=snapshot.style&BS_CENTER;
            bounds.left=align==BS_RIGHT?client.right-(system.tmAveCharWidth-1)-width:
                align==BS_CENTER?(client.left+client.right-width)/2:client.left+system.tmAveCharWidth-1;
            bounds.right=bounds.left+width;bounds.bottom=bounds.top+text.bottom-text.top+2;
            if(!FillRect(dc,&bounds,brush))goto done;
        }
        result=label(window,instance,dc,&snapshot,bounds,&client,FALSE,cues|UISF_HIDEFOCUS,p);goto done;
    }
    POINT origin={0,0};
    if(!LPtoDP(dc,&origin,1)||!SetBrushOrgEx(dc,origin.x,origin.y,NULL))goto done;
    BOOL push=push_style(snapshot.style);bounds=client;
    UINT state=(snapshot.state&BST_PUSHED)?DFCS_PUSHED:0;
    if(snapshot.style&BS_FLAT)state|=DFCS_FLAT|DFCS_MONO;
    if(push){
        SetBkMode(dc,TRANSPARENT);SetTextColor(dc,p->text);SetBkColor(dc,p->face);
        if(snapshot.check)state|=DFCS_CHECKED;
        if(!win2k_push_button_frame(dc,&bounds,state|DFCS_ADJUSTRECT,snapshot.role.type==BS_DEFPUSHBUTTON,p))goto done;
        IntersectClipRect(dc,bounds.left,bounds.top,bounds.right,bounds.bottom);
    }else{
        SIZE zero;if(!GetTextExtentPoint32W(dc,L"0",1,&zero))goto done;
        glyph=client;UINT vertical=snapshot.style&BS_VCENTER;
        if(vertical==BS_TOP || vertical==BS_BOTTOM){
            int row=zero.cy+zero.cy/4,offset=(row-WIN2K_CHOICE_SIZE)/2;
            glyph.top=vertical==BS_TOP?client.top+offset:client.bottom-row+offset;
        }else glyph.top=(client.top+client.bottom-WIN2K_CHOICE_SIZE)/2;
        glyph.left=(snapshot.style&BS_LEFTTEXT)?client.right-WIN2K_CHOICE_SIZE:client.left;
        glyph.right=glyph.left+WIN2K_CHOICE_SIZE;glyph.bottom=glyph.top+WIN2K_CHOICE_SIZE;
        if(snapshot.style&BS_LEFTTEXT)bounds.right-=WIN2K_CHOICE_SIZE+zero.cx/2;
        else bounds.left+=WIN2K_CHOICE_SIZE+zero.cx/2;
    }
    result=label(window,instance,dc,&snapshot,bounds,&client,push,cues,p);
    if(result && !push){
        BOOL radio=snapshot.role.type==BS_RADIOBUTTON || snapshot.role.type==BS_AUTORADIOBUTTON;
        state|=radio?DFCS_BUTTONRADIO:snapshot.check==BST_INDETERMINATE?DFCS_BUTTON3STATE:DFCS_BUTTONCHECK;
        if(snapshot.check)state|=DFCS_CHECKED;
        if(snapshot.style&WS_DISABLED)state|=DFCS_INACTIVE;
        result=win2k_choice_glyph(dc,&glyph,state,p);
    }
done:
    if(!RestoreDC(dc,saved))result=FALSE;
    style_control_button_snapshot_clear(&snapshot);return result;
}
