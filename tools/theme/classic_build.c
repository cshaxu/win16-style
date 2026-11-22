#include "lib-win32-style/appearance/msstyles/theme_file.h"
#include "lib-win32-style/appearance/msstyles/classic_style.h"
#include "lib-win32-style/appearance/msstyles/v4_endian.h"
#include "lib-win32-style/appearance/msstyles/v4_property.h"
#include "lib-win32-style/appearance/win3x/resources/scroll_glyphs.h"
#include "lib-win32-style/appearance/win3x/resources/combo_glyph.h"
#include "classic_source.h"
#include "legacy_image.h"
#include <vsstyle.h>
#include <vssym32.h>
#include <string.h>
#include <wchar.h>

#define THEME_NAME(s) {(const unsigned char *)(s),ARRAYSIZE(s)}

enum { sysmetrics_class=5, button_class, scrollbar_class, window_class, menu_class,
    edit_class, combo_class, tab_class, header_class, progress_class, trackbar_class,
    listview_class, treeview_class, toolbar_class, rebar_class, status_class,
    monthcal_class, datepicker_class, first_font_id=601 };
static const msstyles_v4_name classes[]={
    THEME_NAME(L"documentation"),THEME_NAME(L"sizevariant.NormalSize"),
    THEME_NAME(L"sizevariant.Default"),THEME_NAME(L"colorvariant.NormalColor"),
    THEME_NAME(L"globals"),THEME_NAME(L"sysmetrics"),
    THEME_NAME(L"Button"),THEME_NAME(L"ScrollBar"),THEME_NAME(L"Window"),THEME_NAME(L"Menu"),
    THEME_NAME(L"Edit"),THEME_NAME(L"ComboBox"),THEME_NAME(L"Tab"),THEME_NAME(L"Header"),THEME_NAME(L"Progress"),
    THEME_NAME(L"TrackBar"),THEME_NAME(L"ListView"),THEME_NAME(L"TreeView"),
    THEME_NAME(L"Toolbar"),THEME_NAME(L"Rebar"),THEME_NAME(L"Status"),THEME_NAME(L"MonthCal"),THEME_NAME(L"DatePicker")
};
static const msstyles_v4_variant variant={
    THEME_NAME(L"Normal"),THEME_NAME(L"NormalSize"),THEME_NAME(L"NormalColor")
};

static bool bytes(FILE *output,const void *data,size_t size)
{
    return fwrite(data,1,size,output)==size;
}

static bool property(FILE *output,uint32_t name,uint32_t type,
    uint32_t cls,uint32_t part,uint32_t state,uint32_t value)
{
    unsigned char data[4],record[40];
    msstyles_v4_write_word(data,value);
    msstyles_v4_property item={name,type,cls,part,state,0,sizeof(data),data};
    size_t length;
    return msstyles_v4_property_write(record,sizeof(record),ARRAYSIZE(classes),&item,&length) &&
        bytes(output,record,length);
}

static bool caption(FILE *output,uint32_t state,classic_caption_source colors,bool gradient)
{
    return property(output,TMT_FILLCOLOR,TMT_COLOR,window_class,WP_CAPTION,state,colors.start) &&
        property(output,TMT_TEXTCOLOR,TMT_COLOR,window_class,WP_CAPTION,state,colors.text) &&
        (!gradient ||
         (property(output,TMT_GRADIENTCOLOR1,TMT_COLOR,window_class,WP_CAPTION,state,colors.start) &&
          property(output,TMT_GRADIENTCOLOR2,TMT_COLOR,window_class,WP_CAPTION,state,colors.end)));
}

static bool button(FILE *output,uint32_t cls,uint32_t part,const classic_theme_source *source)
{
    return property(output,TMT_FILLCOLOR,TMT_COLOR,cls,part,0,source->face) &&
        property(output,TMT_BORDERCOLOR,TMT_COLOR,cls,part,0,source->edge) &&
        property(output,TMT_BGTYPE,TMT_ENUM,cls,part,0,BT_BORDERFILL) &&
        property(output,TMT_BORDERTYPE,TMT_ENUM,cls,part,0,BT_RECT) &&
        property(output,TMT_FILLTYPE,TMT_ENUM,cls,part,0,FT_SOLID);
}

static bool push_button(FILE *output,const classic_theme_source *source)
{
    const uint32_t states[]={PBS_NORMAL,PBS_PRESSED,PBS_DISABLED,PBS_DEFAULTED};
    if(!button(output,button_class,BP_PUSHBUTTON,source))return false;
    for(size_t i=0;i<ARRAYSIZE(states);++i)
        if(!property(output,TMT_FILLCOLOR,TMT_COLOR,button_class,BP_PUSHBUTTON,states[i],source->face) ||
           !property(output,TMT_BORDERCOLOR,TMT_COLOR,button_class,BP_PUSHBUTTON,states[i],source->edge))
            return false;
    return true;
}

static bool short_property(FILE *output,msstyles_v4_property value);

static bool choice_buttons(FILE *output,const classic_theme_source *source)
{
    const unsigned parts[]={BP_CHECKBOX,BP_RADIOBUTTON};
    for(unsigned i=0;i<ARRAYSIZE(parts);++i){
        if(source->legacy_caption_images &&
           !property(output,TMT_BGTYPE,TMT_ENUM,button_class,parts[i],0,BT_IMAGEFILE))return false;
        unsigned count=i?CLASSIC_RADIO_STATES:CLASSIC_CHECK_STATES;
        for(unsigned state=0;state<count;++state){
            BOOL pressed_or_disabled=state%4>=2;
            BOOL muted=state%4==3 || state>=8;
            COLORREF fill=pressed_or_disabled?source->choice_pressed_face:source->choice_face;
            COLORREF ink=muted?source->choice_muted_ink:source->choice_ink;
            if(!property(output,TMT_FILLCOLOR,TMT_COLOR,button_class,parts[i],state+1,fill) ||
               !property(output,TMT_GLYPHTEXTCOLOR,TMT_COLOR,button_class,parts[i],state+1,ink))
                return false;
            if(source->legacy_caption_images){
                unsigned row=state>=8?2:i;
                unsigned column=(state>=4)+2*(state%4==2);
                unsigned id=CLASSIC_CHOICE_FIRST_IMAGE_ID+4*row+column;
                unsigned backup=CLASSIC_CHOICE_FALLBACK_FIRST_IMAGE_ID+4*row+column;
                if(!short_property(output,(msstyles_v4_property){TMT_IMAGEFILE,TMT_FILENAME,button_class,
                    parts[i],state+1,id,16,NULL}) ||
                   !short_property(output,(msstyles_v4_property){TMT_IMAGEFILE1,TMT_FILENAME,button_class,
                    parts[i],state+1,backup,16,NULL}))return false;
            }
        }
    }
    return true;
}

static bool scroll_surface(FILE *output,uint32_t part,const classic_theme_source *source)
{
    return button(output,scrollbar_class,part,source) &&
        property(output,TMT_EDGELIGHTCOLOR,TMT_COLOR,scrollbar_class,part,0,source->bevel_light) &&
        property(output,TMT_EDGEHIGHLIGHTCOLOR,TMT_COLOR,scrollbar_class,part,0,RGB(255,255,255)) &&
        property(output,TMT_EDGESHADOWCOLOR,TMT_COLOR,scrollbar_class,part,0,RGB(128,128,128)) &&
        property(output,TMT_EDGEDKSHADOWCOLOR,TMT_COLOR,scrollbar_class,part,0,source->edge);
}

static bool title_metrics(FILE *output,const classic_caption_metrics *metrics)
{
    return property(output,TMT_HEIGHT,TMT_INT,window_class,WP_CAPTION,0,metrics->height) &&
        property(output,TMT_WIDTH,TMT_INT,window_class,WP_MINBUTTON,0,metrics->button.cx) &&
        property(output,TMT_HEIGHT,TMT_INT,window_class,WP_MINBUTTON,0,metrics->button.cy) &&
        (!metrics->tool_height ||
         (property(output,TMT_HEIGHT,TMT_INT,window_class,WP_SMALLCAPTION,0,metrics->tool_height) &&
          property(output,TMT_WIDTH,TMT_INT,window_class,WP_SMALLCLOSEBUTTON,0,metrics->tool_button.cx) &&
          property(output,TMT_HEIGHT,TMT_INT,window_class,WP_SMALLCLOSEBUTTON,0,metrics->tool_button.cy)));
}

static bool short_property(FILE *output,msstyles_v4_property value)
{
    unsigned char record[32];
    size_t length;
    return msstyles_v4_property_write(record,sizeof(record),ARRAYSIZE(classes),&value,&length) &&
        bytes(output,record,length);
}

static bool legacy_images(FILE *output)
{
    const struct {uint32_t part,pressed,base_image,pressed_image;} parts[]={
        {WP_MINBUTTON,MINBS_PUSHED,CLASSIC_MINIMIZE_IMAGE_ID,CLASSIC_MINIMIZE_DOWN_IMAGE_ID},
        {WP_MAXBUTTON,MAXBS_PUSHED,CLASSIC_MAXIMIZE_IMAGE_ID,CLASSIC_MAXIMIZE_DOWN_IMAGE_ID},
        {WP_RESTOREBUTTON,RBS_PUSHED,CLASSIC_RESTORE_IMAGE_ID,CLASSIC_RESTORE_DOWN_IMAGE_ID},
        {WP_SYSBUTTON,SBS_PUSHED,CLASSIC_SYSTEM_IMAGE_ID,CLASSIC_SYSTEM_DOWN_IMAGE_ID}
    };
    for(size_t i=0;i<ARRAYSIZE(parts);++i){
        if(!property(output,TMT_BGTYPE,TMT_ENUM,window_class,parts[i].part,0,BT_IMAGEFILE) ||
           !short_property(output,(msstyles_v4_property){TMT_IMAGEFILE,TMT_FILENAME,window_class,
               parts[i].part,0,parts[i].base_image,16,NULL}) ||
           !short_property(output,(msstyles_v4_property){TMT_IMAGEFILE,TMT_FILENAME,window_class,
               parts[i].part,parts[i].pressed,parts[i].pressed_image,16,NULL}))
            return false;
    }
    const unsigned menu_parts[]={MENU_POPUPCHECK,MENU_POPUPSUBMENU};
    for(unsigned i=0;i<ARRAYSIZE(menu_parts);++i)
        if(!property(output,TMT_BGTYPE,TMT_ENUM,menu_class,menu_parts[i],0,BT_IMAGEFILE) ||
           !property(output,TMT_TRANSPARENT,TMT_BOOL,menu_class,menu_parts[i],0,TRUE) ||
           !property(output,TMT_TRANSPARENTCOLOR,TMT_COLOR,menu_class,menu_parts[i],0,RGB(255,255,255)) ||
           !short_property(output,(msstyles_v4_property){TMT_IMAGEFILE,TMT_FILENAME,menu_class,
               menu_parts[i],0,CLASSIC_MENU_CHECK_IMAGE_ID+i,16,NULL}))return false;
    return true;
}

static bool menu_colors(FILE *output,const classic_menu_colors *menu)
{
    if(!menu->present)return true;
    const struct {unsigned background,item,normal,hot,disabled;} roles[]={
        {MENU_BARBACKGROUND,MENU_BARITEM,MBI_NORMAL,MBI_HOT,MBI_DISABLED},
        {MENU_POPUPBACKGROUND,MENU_POPUPITEM,MPI_NORMAL,MPI_HOT,MPI_DISABLED}
    };
    for(size_t i=0;i<ARRAYSIZE(roles);++i)
        if(!property(output,TMT_FILLCOLOR,TMT_COLOR,menu_class,roles[i].background,0,menu->background) ||
           !property(output,TMT_FILLCOLOR,TMT_COLOR,menu_class,roles[i].item,roles[i].hot,menu->selection) ||
           !property(output,TMT_TEXTCOLOR,TMT_COLOR,menu_class,roles[i].item,roles[i].normal,menu->text) ||
           !property(output,TMT_TEXTCOLOR,TMT_COLOR,menu_class,roles[i].item,roles[i].hot,menu->selected_text) ||
           !property(output,TMT_TEXTCOLOR,TMT_COLOR,menu_class,roles[i].item,roles[i].disabled,menu->disabled_text))
            return false;
    return property(output,TMT_BORDERCOLOR,TMT_COLOR,menu_class,MENU_POPUPSEPARATOR,0,menu->separator);
}

static bool progress_colors(FILE *output,const classic_theme_source *source)
{
    const unsigned bar[]={PP_BAR,PP_BARVERT},fill[]={PP_FILL,PP_FILLVERT};
    for(size_t i=0;i<ARRAYSIZE(bar);++i){
        if(!property(output,TMT_FILLCOLOR,TMT_COLOR,progress_class,bar[i],0,source->progress_trough) ||
           !property(output,TMT_FILLCOLOR,TMT_COLOR,progress_class,fill[i],PBFS_NORMAL,source->selection))
            return false;
        if(source->own_progress_states &&
           (!property(output,TMT_BORDERCOLOR,TMT_COLOR,progress_class,bar[i],0,source->edge) ||
            !property(output,TMT_FILLCOLOR,TMT_COLOR,progress_class,fill[i],PBFS_ERROR,RGB(192,0,0)) ||
            !property(output,TMT_FILLCOLOR,TMT_COLOR,progress_class,fill[i],PBFS_PAUSED,RGB(192,192,0))))
            return false;
    }
    return true;
}

static bool report_colors(FILE *output,const classic_theme_source *source)
{
    const unsigned classes_by_role[]={listview_class,treeview_class},parts[]={LVP_LISTITEM,TVP_TREEITEM};
    const unsigned states[][CLASSIC_REPORT_STATE_COUNT]={
        {LISS_SELECTED,LISS_SELECTEDNOTFOCUS,LISS_DISABLED},
        {TREIS_SELECTED,TREIS_SELECTEDNOTFOCUS,TREIS_DISABLED}
    };
    const COLORREF text[]={source->selection_text,RGB(0,0,0),RGB(128,128,128)};
    const COLORREF fill[]={source->selection,source->face,source->face};
    for(size_t role=0;role<ARRAYSIZE(parts);++role)
        for(size_t state=0;state<ARRAYSIZE(text);++state)
            if(!property(output,TMT_TEXTCOLOR,TMT_COLOR,classes_by_role[role],parts[role],states[role][state],text[state]) ||
               !property(output,TMT_FILLCOLOR,TMT_COLOR,classes_by_role[role],parts[role],states[role][state],fill[state]))
                return false;
    return true;
}

static bool control_bar_colors(FILE *output,const classic_theme_source *source)
{
    const COLORREF highlight=RGB(255,255,255),shadow=RGB(128,128,128);
    return property(output,TMT_FILLCOLOR,TMT_COLOR,toolbar_class,TP_BUTTON,0,source->face) &&
        property(output,TMT_FILLCOLOR,TMT_COLOR,toolbar_class,TP_BUTTON,TS_PRESSED,source->pressed_toolbar) &&
        property(output,TMT_BORDERCOLOR,TMT_COLOR,toolbar_class,TP_BUTTON,0,source->edge) &&
        property(output,TMT_EDGEHIGHLIGHTCOLOR,TMT_COLOR,toolbar_class,TP_BUTTON,0,highlight) &&
        property(output,TMT_EDGESHADOWCOLOR,TMT_COLOR,toolbar_class,TP_BUTTON,0,shadow) &&
        property(output,TMT_TEXTCOLOR,TMT_COLOR,toolbar_class,TP_BUTTON,0,RGB(0,0,0)) &&
        property(output,TMT_FILLCOLOR,TMT_COLOR,rebar_class,RP_GRIPPER,0,source->face) &&
        property(output,TMT_EDGEHIGHLIGHTCOLOR,TMT_COLOR,rebar_class,RP_GRIPPER,0,highlight) &&
        property(output,TMT_EDGESHADOWCOLOR,TMT_COLOR,rebar_class,RP_GRIPPER,0,shadow) &&
        property(output,TMT_BORDERCOLOR,TMT_COLOR,status_class,SP_PANE,0,RGB(0,0,0)) &&
        property(output,TMT_EDGEHIGHLIGHTCOLOR,TMT_COLOR,status_class,SP_PANE,0,highlight) &&
        property(output,TMT_EDGESHADOWCOLOR,TMT_COLOR,status_class,SP_PANE,0,shadow);
}

static bool write_properties(FILE *output,const classic_theme_source *source)
{
    const unsigned font_properties[]={TMT_CAPTIONFONT,TMT_SMALLCAPTIONFONT,TMT_MENUFONT};
    for(unsigned i=0;i<ARRAYSIZE(font_properties);++i)
        if(source->fonts[i] && !short_property(output,(msstyles_v4_property){font_properties[i],TMT_FONT,
            sysmetrics_class,0,0,first_font_id+i,sizeof(LOGFONTW),NULL}))return false;
    if(source->legacy_caption_images && !legacy_images(output))return false;
    const unsigned edit_states[]={ETS_NORMAL,ETS_READONLY,ETS_DISABLED};
    for(size_t i=0;i<ARRAYSIZE(edit_states);++i)
        if(!property(output,TMT_BORDERCOLOR,TMT_COLOR,edit_class,EP_EDITTEXT,edit_states[i],RGB(0,0,0)) ||
           !property(output,TMT_EDGEDKSHADOWCOLOR,TMT_COLOR,edit_class,EP_EDITTEXT,edit_states[i],source->edge))
            return false;
    const unsigned combo_states[]={CBXS_NORMAL,CBXS_PRESSED,CBXS_DISABLED};
    for(size_t i=0;i<ARRAYSIZE(combo_states);++i)
        if(!property(output,TMT_FILLCOLOR,TMT_COLOR,combo_class,CP_DROPDOWNBUTTON,combo_states[i],source->face))
            return false;
    const unsigned track_part[]={TKP_TRACK,TKP_TRACKVERT},thumb_part[]={TKP_THUMB,TKP_THUMBVERT};
    for(size_t i=0;i<ARRAYSIZE(track_part);++i)
        if(!property(output,TMT_FILLCOLOR,TMT_COLOR,trackbar_class,track_part[i],0,RGB(255,255,255)) ||
           !property(output,TMT_FILLCOLOR,TMT_COLOR,trackbar_class,thumb_part[i],0,source->face) ||
           !property(output,TMT_BORDERCOLOR,TMT_COLOR,trackbar_class,track_part[i],0,source->edge))
            return false;
    return property(output,TMT_BTNFACE,TMT_COLOR,sysmetrics_class,0,0,source->face) &&
        property(output,TMT_BTNHIGHLIGHT,TMT_COLOR,sysmetrics_class,0,0,RGB(255,255,255)) &&
        property(output,TMT_BTNSHADOW,TMT_COLOR,sysmetrics_class,0,0,RGB(128,128,128)) &&
        property(output,TMT_WINDOWFRAME,TMT_COLOR,sysmetrics_class,0,0,RGB(0,0,0)) &&
        property(output,TMT_WINDOWTEXT,TMT_COLOR,sysmetrics_class,0,0,RGB(0,0,0)) &&
        property(output,TMT_GRAYTEXT,TMT_COLOR,sysmetrics_class,0,0,RGB(128,128,128)) &&
        property(output,TMT_WINDOW,TMT_COLOR,sysmetrics_class,0,0,RGB(255,255,255)) &&
        property(output,TMT_HIGHLIGHT,TMT_COLOR,sysmetrics_class,0,0,source->selection) &&
        property(output,TMT_HIGHLIGHTTEXT,TMT_COLOR,sysmetrics_class,0,0,source->selection_text) &&
        property(output,TMT_DKSHADOW3D,TMT_COLOR,sysmetrics_class,0,0,source->edge) &&
        property(output,TMT_BORDERCOLOR,TMT_COLOR,combo_class,CP_BORDER,CBB_NORMAL,source->edge) &&
        property(output,TMT_BORDERCOLOR,TMT_COLOR,combo_class,CP_BORDER,CBB_DISABLED,source->edge) &&
        property(output,TMT_FILLCOLOR,TMT_COLOR,tab_class,TABP_PANE,0,source->face) &&
        property(output,TMT_BORDERCOLOR,TMT_COLOR,tab_class,TABP_PANE,0,source->edge) &&
        property(output,TMT_FILLCOLOR,TMT_COLOR,tab_class,TABP_TABITEM,TIS_NORMAL,source->face) &&
        property(output,TMT_FILLCOLOR,TMT_COLOR,tab_class,TABP_TABITEM,TIS_SELECTED,
            source->selected_tab) &&
        property(output,TMT_FILLCOLOR,TMT_COLOR,header_class,HP_HEADERITEM,HIS_NORMAL,source->face) &&
        property(output,TMT_FILLCOLOR,TMT_COLOR,header_class,HP_HEADERITEM,HIS_PRESSED,source->face) &&
        property(output,TMT_TEXTCOLOR,TMT_COLOR,button_class,BP_GROUPBOX,GBS_NORMAL,RGB(0,0,0)) &&
        property(output,TMT_TEXTCOLOR,TMT_COLOR,button_class,BP_GROUPBOX,GBS_DISABLED,source->group_disabled_text) &&
        property(output,TMT_EDGESHADOWCOLOR,TMT_COLOR,button_class,BP_GROUPBOX,0,RGB(128,128,128)) &&
        property(output,TMT_EDGEHIGHLIGHTCOLOR,TMT_COLOR,button_class,BP_GROUPBOX,0,RGB(255,255,255)) &&
        menu_colors(output,&source->menu) && progress_colors(output,source) && report_colors(output,source) &&
        control_bar_colors(output,source) &&
        property(output,TMT_FILLCOLOR,TMT_COLOR,monthcal_class,MC_BACKGROUND,0,RGB(255,255,255)) &&
        property(output,TMT_FILLCOLOR,TMT_COLOR,monthcal_class,MC_GRIDBACKGROUND,0,RGB(255,255,255)) &&
        property(output,TMT_TEXTCOLOR,TMT_COLOR,monthcal_class,MC_GRIDCELL,0,RGB(0,0,0)) &&
        property(output,TMT_TEXTCOLOR,TMT_COLOR,monthcal_class,MC_TRAILINGGRIDCELL,0,RGB(128,128,128)) &&
        property(output,TMT_FILLCOLOR,TMT_COLOR,monthcal_class,MC_GRIDCELLBACKGROUND,MCGCB_SELECTED,source->selection) &&
        property(output,TMT_TEXTCOLOR,TMT_COLOR,monthcal_class,MC_GRIDCELLBACKGROUND,MCGCB_SELECTED,source->selection_text) &&
        property(output,TMT_FILLCOLOR,TMT_COLOR,datepicker_class,DP_SHOWCALENDARBUTTONRIGHT,0,source->face) &&
        title_metrics(output,&source->caption_metrics) && push_button(output,source) && choice_buttons(output,source) &&
        button(output,scrollbar_class,SBP_ARROWBTN,source) &&
        scroll_surface(output,SBP_THUMBBTNHORZ,source) &&
        scroll_surface(output,SBP_THUMBBTNVERT,source) &&
        scroll_surface(output,SBP_LOWERTRACKHORZ,source) &&
        scroll_surface(output,SBP_UPPERTRACKHORZ,source) &&
        scroll_surface(output,SBP_LOWERTRACKVERT,source) &&
        scroll_surface(output,SBP_UPPERTRACKVERT,source) &&
        (!source->own_sizebox || scroll_surface(output,SBP_SIZEBOX,source)) &&
        property(output,TMT_BGTYPE,TMT_ENUM,window_class,WP_CAPTION,0,BT_BORDERFILL) &&
        property(output,TMT_FILLTYPE,TMT_ENUM,window_class,WP_CAPTION,0,
                 source->gradient?FT_HORZGRADIENT:FT_SOLID) &&
        caption(output,CS_ACTIVE,source->active,source->gradient) &&
        caption(output,CS_INACTIVE,source->inactive,source->gradient);
}

static bool write_classes(FILE *output,const classic_theme_source *source)
{
    (void)source;
    for(size_t i=0;i<ARRAYSIZE(classes);++i){
        unsigned char buffer[64];
        size_t length;
        if(!msstyles_v4_class_write(buffer,sizeof(buffer),classes[i],&length) ||
           !bytes(output,buffer,length))return false;
    }
    return true;
}

static bool write_variant(FILE *output,const classic_theme_source *source)
{
    (void)source;
    unsigned char buffer[128];
    size_t length;
    return msstyles_v4_variant_write(buffer,sizeof(buffer),&variant,&length) &&
        bytes(output,buffer,length);
}

static bool write_base_classes(FILE *output,const classic_theme_source *source)
{
    (void)source;
    unsigned char table[4*(ARRAYSIZE(classes)-3)];
    memset(table,0xff,sizeof(table));
    msstyles_v4_write_word(table,ARRAYSIZE(classes)-4);
    return bytes(output,table,sizeof(table));
}

static bool write_metadata(FILE *output,const classic_theme_source *source)
{
    unsigned char name[64]={0},record[96];
    size_t length=strlen(source->label);
    if(length>=sizeof(name)/2)return false;
    for(size_t i=0;i<length;++i)name[2*i]=(unsigned char)source->label[i];
    msstyles_v4_property item={TMT_DISPLAYNAME,TMT_STRING,0,0,0,0,(uint32_t)((length+1)*2),name};
    size_t written;
    return msstyles_v4_property_write(record,sizeof(record),ARRAYSIZE(classes),&item,&written) &&
        bytes(output,record,written);
}

static bool write_description(FILE *output,const classic_theme_source *source)
{
    return msstyles_theme_file_write(output,source->label,source->stem,&variant,source->host_options);
}

static bool write_extension(FILE *output,const classic_theme_source *source)
{
    unsigned char buffer[CLASSIC_STYLE_RESOURCE_CAPACITY];
    const classic_menu_metrics *menu=source->menu_metrics.check_column?&source->menu_metrics:NULL;
    const classic_dialog_metrics *dialog=source->dialog_metrics.print_width?&source->dialog_metrics:NULL;
    size_t length=classic_style_write(buffer,sizeof(buffer),&source->buttons,&source->scroll_metrics,menu,dialog,
        &source->dialog_policy,&source->caption_presentation,&source->border_presentation,&source->controls);
    return length && bytes(output,buffer,length);
}

static bool write_fonts(FILE *output,const classic_theme_source *source)
{
    if(fputs("STRINGTABLE\nBEGIN\n",output)<0)return false;
    for(unsigned i=0;i<ARRAYSIZE(source->fonts);++i){
        const char *font=source->fonts[i];
        if(font && (strpbrk(font,"\"\\\r\n") || fprintf(output,"  %u \"%s\"\n",first_font_id+i,font)<0))
            return false;
    }
    return fputs("END\n",output)>=0;
}

static bool write_scroll_arrows(FILE *output,const classic_theme_source *source)
{
    const unsigned char header[]={'S','A','R',1,(unsigned char)source->arrow_source};
    if(!bytes(output,header,sizeof(header)))return false;
    if(source->arrow_source==STYLE_SCROLL_ARROW_SYMBOL)return true;
    static const char (*const originals[])[STYLE_SCROLL_ARROW_SIDE+1]={
        scroll_uparrow,scroll_dnarrow,scroll_lfarrow,scroll_rgarrow,
        scroll_uparrowd,scroll_dnarrowd,scroll_lfarrowd,scroll_rgarrowd,
        scroll_uparrowi,scroll_dnarrowi,scroll_lfarrowi,scroll_rgarrowi
    };
    _Static_assert(ARRAYSIZE(originals)==STYLE_SCROLL_ARROW_STATES*STYLE_SCROLL_ARROW_DIRECTIONS,"arrow order");
    if(source->arrow_source!=STYLE_SCROLL_ARROW_INDEXED)return false;
    for(unsigned image=0;image<ARRAYSIZE(originals);++image){
        unsigned char cell[STYLE_SCROLL_ARROW_BYTES]={0};
        for(unsigned i=0;i<STYLE_SCROLL_ARROW_SIDE*STYLE_SCROLL_ARROW_SIDE;++i){
            unsigned role=(unsigned)(originals[image][i/STYLE_SCROLL_ARROW_SIDE][i%STYLE_SCROLL_ARROW_SIDE]-'0');
            if(role>3)return false;
            cell[i/4]|=(unsigned char)(role<<(2*(i%4)));
        }
        if(!bytes(output,cell,sizeof(cell)))return false;
    }
    return true;
}

static bool write_combo_arrow(FILE *output,const classic_theme_source *source)
{
    if(!source->legacy_caption_images)return true;
    unsigned char data[4+CLASSIC_COMBO_ARROW_HEIGHT]={'C','A','R',1};
    for(unsigned y=0;y<CLASSIC_COMBO_ARROW_HEIGHT;++y)
        for(unsigned x=0;x<CLASSIC_COMBO_ARROW_WIDTH;++x)
            if(combo_arrow[y][x]=='0')data[4+y]|=(unsigned char)(1u<<x);
    return bytes(output,data,sizeof(data));
}

static bool write_calendar_font(FILE *output,const classic_theme_source *source)
{
    if(!source->calendar_font_dpi)return true;
    if(source->calendar_font_dpi<48 || source->calendar_font_dpi>96 ||
       !source->calendar_font_points || source->calendar_font_points>48)return false;
    const unsigned char data[]={2,(unsigned char)source->calendar_font_dpi,(unsigned char)source->calendar_font_points};
    return bytes(output,data,sizeof(data));
}

static const struct {
    const char *filename;
    bool (*write)(FILE *,const classic_theme_source *);
} outputs[]={
    {"cmap.bin",write_classes},{"bcmap.bin",write_base_classes},
    {"vmap.bin",write_variant},{"rmap.bin",write_metadata},
    {"normal.bin",write_properties},{"classic.bin",write_extension},{"fonts.rcinc",write_fonts},
    {"arrows.bin",write_scroll_arrows},{"combo-arrow.bin",write_combo_arrow},
    {"calendar-font.bin",write_calendar_font},
    {NULL,write_description}
};

int main(int argc,char **argv)
{
    if(argc!=2)return 2;
    const classic_theme_source *source=NULL;
    for(size_t i=0;i<ARRAYSIZE(classic_sources);++i)
        if(!strcmp(argv[1],classic_sources[i].stem))source=&classic_sources[i];
    if(!source)return 2;

    char description[32];
    snprintf(description,sizeof(description),"%s.theme",source->stem);
    for(size_t i=0;i<ARRAYSIZE(outputs);++i){
        const char *name=outputs[i].filename?outputs[i].filename:description;
        FILE *output=fopen(name,"wb");
        if(!output){perror(name);return 1;}
        bool written=outputs[i].write(output,source);
        if(fclose(output)!=0 || !written){
            fprintf(stderr,"Failed to write %s\n",name);
            return 1;
        }
    }
    for(unsigned id=CLASSIC_MINIMIZE_IMAGE_ID;source->legacy_caption_images && id<=CLASSIC_CHOICE_LAST_IMAGE_ID;++id){
        WCHAR filename[24];
        const WCHAR *pattern=id>=CLASSIC_CHOICE_FIRST_IMAGE_ID?L"choice%u.png":
            id>=CLASSIC_MENU_CHECK_IMAGE_ID?L"menu%u.png":L"caption%u.png";
        swprintf(filename,ARRAYSIZE(filename),pattern,id);
        HRESULT result=classic_write_legacy_png(id,filename);
        if(FAILED(result)){
            fprintf(stderr,"Failed to write original bitmap %u (0x%08lx)\n",id,(unsigned long)result);
            return 1;
        }
    }
    return 0;
}
