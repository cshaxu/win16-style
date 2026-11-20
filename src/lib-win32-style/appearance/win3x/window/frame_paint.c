#include "frame_paint.h"
#include "../resources/frame_buttons.h"

void win16_control_fill(HDC dc,RECT r,COLORREF c)
{
    SetDCBrushColor(dc,c);FillRect(dc,&r,(HBRUSH)GetStockObject(DC_BRUSH));
}

void win16_control_outline(HDC dc,RECT r,int u,COLORREF c)
{
    RECT q=r;SetDCBrushColor(dc,c);HBRUSH brush=(HBRUSH)GetStockObject(DC_BRUSH);
    q.bottom=q.top+u;FillRect(dc,&q,brush);
    q=r;q.top=q.bottom-u;FillRect(dc,&q,brush);
    q=r;q.right=q.left+u;FillRect(dc,&q,brush);
    q=r;q.left=q.right-u;FillRect(dc,&q,brush);
}

static void paint_button(HDC dc,RECT r,int kind,BOOL down,BOOL zoom,BOOL child,int u,
    const win16_style_palette *p,const classic_caption_art *art,const style_caption_presentation *presentation)
{
    const BOOL system=kind==HTSYSMENU;
    const int width=system?18:19;
    const unsigned role=system?(child?CLASSIC_IMAGE_ROLE_COUNT:CLASSIC_IMAGE_SYSTEM):
        kind==HTMINBUTTON?CLASSIC_IMAGE_MINIMIZE:zoom?CLASSIC_IMAGE_RESTORE:CLASSIC_IMAGE_MAXIMIZE;
    const BYTE *roles=art && art->available?art->roles[role][!!down]:NULL;
    const char *fallback=system?&system_menu_bitmap[0][child?18:0]:&frame_buttons[role+(down?3:0)][0][0];
    const int stride=system?37:20;
    const COLORREF palette[]={p->edge,p->light,p->shadow,p->face};
    const COLORREF pressed_roles[]={p->light,p->edge,p->face,p->shadow};
    const style_caption_raster raster={
        roles?roles:(const BYTE *)fallback,(unsigned)width,18,
        (unsigned)(roles?width:stride),(BYTE)(roles?0:'0'),!roles && system && down?pressed_roles:NULL
    };
    style_caption_button_paint(dc,r,style_caption_role(kind),zoom,down?DFCS_PUSHED:0,u,presentation,&raster,palette);
}

void win16_style_merged_control(HDC dc,RECT bounds,BOOL system,BOOL selected,int unit,
    const win16_style_palette *palette,const classic_caption_art *art)
{
    const style_caption_presentation presentation={.button_source=STYLE_CAPTION_BUTTON_INDEXED,
        .button_outline=STYLE_CAPTION_BUTTON_EDGE_OUTLINE};
    paint_button(dc,bounds,system?HTSYSMENU:HTMAXBUTTON,selected,!system,system,unit,palette,art,&presentation);
}


void win16_control_frame_caption(HDC dc,const win16_style_layout *layout,const win16_style_palette *palette,
    const win16_frame_paint_state *snapshot,PCWSTR title,BOOL child,BOOL zoomed)
{
    win16_style_layout l=*layout;win16_style_palette p=*palette;
    win16_frame_paint_state state=*snapshot;
    style_caption_presentation presentation=state.presentation;
    if(!presentation.text_mode){
        presentation.text_mode=STYLE_CAPTION_TEXT_CENTER_CLIP;
        presentation.separator=STYLE_CAPTION_SEPARATOR_AFTER;
    }
    if(!presentation.button_source){
        presentation.button_source=STYLE_CAPTION_BUTTON_INDEXED;
        presentation.button_outline=STYLE_CAPTION_BUTTON_EDGE_OUTLINE;
    }
    COLORREF start=state.active?p.active:p.inactive;
    BOOL gradient=state.gradient && !win16_control_high_contrast();
    RECT band=style_caption_band(dc,l.caption,l.caption.right,start,gradient?state.end:start,
        gradient,presentation.separator,l.unit,p.edge);
    paint_button(dc,l.buttons[CLASSIC_BUTTON_SYSTEM],HTSYSMENU,state.system_menu||(state.bar_keyboard&&state.bar_selected==-2)||(state.button.pressed==HTSYSMENU&&state.button.hot),FALSE,
        child,l.unit,&p,state.art,&presentation);
    paint_button(dc,l.buttons[CLASSIC_BUTTON_MINIMIZE],HTMINBUTTON,state.button.pressed==HTMINBUTTON&&state.button.hot,FALSE,FALSE,l.unit,&p,state.art,&presentation);
    paint_button(dc,l.buttons[CLASSIC_BUTTON_MAXIMIZE],HTMAXBUTTON,state.button.pressed==HTMAXBUTTON&&state.button.hot,zoomed,FALSE,l.unit,&p,state.art,&presentation);
    if(title){
        RECT text=l.title;
        text.bottom=min(text.bottom,band.bottom);
        style_caption_title(dc,text,title,state.fonts->title,state.active?p.active_text:p.inactive_text,
            presentation.text_mode);
    }
}
