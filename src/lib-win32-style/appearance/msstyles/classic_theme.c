#include "classic_theme.h"
#include <commctrl.h>
#include "v4_endian.h"
#include "v4_property.h"
#include "v4_tables.h"
#include <limits.h>
#include <vsstyle.h>
#include <vssym32.h>
#include <wchar.h>

typedef struct theme_resource {
    const unsigned char *bytes;
    DWORD size;
} theme_resource;

static theme_resource resource(HMODULE module,PCWSTR type,PCWSTR name)
{
    HRSRC entry=FindResourceW(module,name,type);
    HGLOBAL loaded=entry?LoadResource(module,entry):NULL;
    return loaded?(theme_resource){LockResource(loaded),SizeofResource(module,entry)}:(theme_resource){0};
}

static bool name_equals(msstyles_v4_name name,PCWSTR expected)
{
    return name.units<=INT_MAX && CompareStringOrdinal((PCWSTR)name.bytes,(int)name.units-1,
        expected,-1,TRUE)==CSTR_EQUAL;
}

static HRESULT variant_resource(HMODULE module,PCWSTR size,PCWSTR color,theme_resource *selected)
{
    theme_resource map=resource(module,L"VMAP",L"VMAP");
    theme_resource value={0};
    if(!map.bytes || !map.size)return HRESULT_FROM_WIN32(ERROR_RESOURCE_DATA_NOT_FOUND);
    for(size_t pos=0;pos<map.size;){
        msstyles_v4_variant variant;
        size_t used;
        if(!msstyles_v4_variant_read(map.bytes+pos,map.size-pos,&variant,&used))
            return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        if(name_equals(variant.size,size) && name_equals(variant.color,color)){
            if(value.bytes)return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            value=resource(module,L"VARIANT",(PCWSTR)variant.record.bytes);
            if(!value.bytes || !value.size)return HRESULT_FROM_WIN32(ERROR_RESOURCE_DATA_NOT_FOUND);
        }
        pos+=used;
    }
    if(!value.bytes)return HRESULT_FROM_WIN32(ERROR_RESOURCE_NAME_NOT_FOUND);
    *selected=value;
    return S_OK;
}

enum theme_class { class_window, class_button, class_scroll_arrow, class_sysmetrics,
    class_menu, class_edit, class_combo, class_tab, class_header, class_progress, class_trackbar, class_count };

static HRESULT class_indices(theme_resource map,size_t indices[class_count],size_t *count)
{
    const PCWSTR names[class_count]={L"Window",L"Button",L"ScrollBar",L"SysMetrics",L"Menu",
        L"Edit",L"ComboBox",L"Tab",L"Header",L"Progress",L"TrackBar"};
    if(!map.bytes || !map.size)return HRESULT_FROM_WIN32(ERROR_RESOURCE_DATA_NOT_FOUND);
    for(unsigned i=0;i<class_count;++i)indices[i]=SIZE_MAX;
    *count=0;
    for(size_t pos=0;pos<map.size;++*count){
        msstyles_v4_name name;
        size_t used;
        if(!msstyles_v4_class_read(map.bytes+pos,map.size-pos,&name,&used))
            return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        for(unsigned i=0;i<class_count;++i){
            if(!name_equals(name,names[i]))continue;
            if(indices[i]!=SIZE_MAX)return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            indices[i]=*count;
        }
        pos+=used;
    }
    for(unsigned i=0;i<class_menu;++i)
        if(indices[i]==SIZE_MAX)return HRESULT_FROM_WIN32(ERROR_RESOURCE_NAME_NOT_FOUND);
    return S_OK;
}

typedef struct theme_field {
    enum theme_class cls;
    uint32_t part,state,name,type;
    uint32_t value;
    bool present;
} theme_field;

#define COLOR_FIELD(cls,part,state,name) {cls,part,state,name,TMT_COLOR,0,false}

enum { choice_face=1, choice_ink=2 };

const classic_choice_surface *classic_choice_entry(const classic_choice_colors *colors,
    BOOL radio,LRESULT check,BOOL disabled,LRESULT button_state)
{
    if(!colors)return NULL;
    unsigned value=check==BST_UNCHECKED?0:(!radio && check==BST_INDETERMINATE?2:1);
    unsigned state=4*value+(disabled?3:(button_state&BST_PUSHED)?2:(button_state&BST_HOT)?1:0);
    return radio?&colors->radio[state]:&colors->check[state];
}

static BOOL read_choice(const msstyles_v4_property *item,classic_choice_colors *colors)
{
    BOOL radio=item->part_id==BP_RADIOBUTTON;
    if(!radio && item->part_id!=BP_CHECKBOX)return TRUE;
    if(item->state_id==0 || item->state_id>(radio?CLASSIC_RADIO_STATES:CLASSIC_CHECK_STATES))return TRUE;
    classic_choice_surface *surface=(radio?colors->radio:colors->check)+item->state_id-1;
    if(item->name_id==TMT_IMAGEFILE){
        if(surface->image || item->type_id!=TMT_FILENAME || item->data || item->declared_size!=16 ||
           !item->short_value || item->short_value>UINT16_MAX)return FALSE;
        surface->image=(WORD)item->short_value;
        return TRUE;
    }
    unsigned field=item->name_id==TMT_FILLCOLOR?choice_face:
        item->name_id==TMT_GLYPHTEXTCOLOR?choice_ink:0;
    if(!field)return TRUE;
    if(item->type_id!=TMT_COLOR || !item->data || item->declared_size!=4)return FALSE;
    COLORREF value=msstyles_v4_read_word(item->data);
    if((surface->fields&field) || value>0xffffffu)return FALSE;
    surface->fields|=field;
    if(field==choice_face)surface->colors.face=value;
    else surface->colors.edge=value;
    return TRUE;
}

static HRESULT read_fields(const msstyles_v4_property *item,const size_t indices[class_count],
    theme_field *fields,size_t count)
{
    for(size_t i=0;i<count;++i){
        theme_field *field=&fields[i];
        if(item->class_id!=indices[field->cls] || item->part_id!=field->part ||
           item->state_id!=field->state || item->name_id!=field->name)continue;
        if(field->present || item->type_id!=field->type)
            return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        if(field->type==TMT_FILENAME || field->type==TMT_FONT){
            size_t size=field->type==TMT_FONT?sizeof(LOGFONTW):16;
            if(item->data || item->declared_size!=size || !item->short_value || item->short_value>UINT16_MAX)
                return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            field->value=item->short_value;
        }else{
            if(!item->data || item->declared_size!=4)return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            field->value=msstyles_v4_read_word(item->data);
        }
        if(field->type==TMT_COLOR && field->value>0xffffffu)
            return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        field->present=true;
        break;
    }
    return S_OK;
}

enum { scroll_surface_fields=5, scroll_track_pairs=2 };

static bool scroll_colors(theme_field fields[][scroll_surface_fields],size_t surfaces,
    classic_scroll_surface *colors,BOOL *loaded)
{
    size_t count=surfaces*scroll_surface_fields,found=0;
    for(size_t i=0;i<surfaces;++i)
        for(size_t j=0;j<scroll_surface_fields;++j)
            if(fields[i][j].present)++found;
    if(found && found!=count)return false;
    *loaded=found!=0;
    if(*loaded){
        for(size_t i=0;i<surfaces;++i)
            colors[i]=(classic_scroll_surface){fields[i][0].value,fields[i][1].value,
                fields[i][2].value,fields[i][3].value,fields[i][4].value};
    }
    return true;
}

static HRESULT palette(HMODULE module,theme_resource map,theme_resource variant,classic_theme *theme)
{
    enum {fill_type, active_face, active_text, inactive_face, inactive_text,
        button_face, button_edge, scroll_face, scroll_edge, first_gradient};
    theme_field fields[]={
        {class_window,WP_CAPTION,0,TMT_FILLTYPE,TMT_ENUM,0,false},
        COLOR_FIELD(class_window,WP_CAPTION,CS_ACTIVE,TMT_FILLCOLOR),
        COLOR_FIELD(class_window,WP_CAPTION,CS_ACTIVE,TMT_TEXTCOLOR),
        COLOR_FIELD(class_window,WP_CAPTION,CS_INACTIVE,TMT_FILLCOLOR),
        COLOR_FIELD(class_window,WP_CAPTION,CS_INACTIVE,TMT_TEXTCOLOR),
        COLOR_FIELD(class_button,BP_PUSHBUTTON,0,TMT_FILLCOLOR),
        COLOR_FIELD(class_button,BP_PUSHBUTTON,0,TMT_BORDERCOLOR),
        COLOR_FIELD(class_scroll_arrow,SBP_ARROWBTN,0,TMT_FILLCOLOR),
        COLOR_FIELD(class_scroll_arrow,SBP_ARROWBTN,0,TMT_BORDERCOLOR),
        COLOR_FIELD(class_window,WP_CAPTION,CS_ACTIVE,TMT_GRADIENTCOLOR1),
        COLOR_FIELD(class_window,WP_CAPTION,CS_ACTIVE,TMT_GRADIENTCOLOR2),
        COLOR_FIELD(class_window,WP_CAPTION,CS_INACTIVE,TMT_GRADIENTCOLOR1),
        COLOR_FIELD(class_window,WP_CAPTION,CS_INACTIVE,TMT_GRADIENTCOLOR2)
    };
    const unsigned push_states[]={PBS_NORMAL,PBS_PRESSED,PBS_DISABLED,PBS_DEFAULTED};
    theme_field push[CLASSIC_PUSH_STATE_COUNT*2];
    for(size_t i=0;i<ARRAYSIZE(push_states);++i){
        push[2*i]=(theme_field)COLOR_FIELD(class_button,BP_PUSHBUTTON,push_states[i],TMT_FILLCOLOR);
        push[2*i+1]=(theme_field)COLOR_FIELD(class_button,BP_PUSHBUTTON,push_states[i],TMT_BORDERCOLOR);
    }
    const unsigned scroll_parts[]={SBP_THUMBBTNHORZ,SBP_THUMBBTNVERT,SBP_LOWERTRACKHORZ,
        SBP_LOWERTRACKVERT,SBP_SIZEBOX};
    const unsigned color_ids[]={TMT_FILLCOLOR,TMT_EDGELIGHTCOLOR,TMT_EDGEHIGHLIGHTCOLOR,
        TMT_EDGESHADOWCOLOR,TMT_EDGEDKSHADOWCOLOR};
    theme_field scroll[ARRAYSIZE(scroll_parts)][ARRAYSIZE(color_ids)];
    for(size_t i=0;i<ARRAYSIZE(scroll_parts);++i)
        for(size_t j=0;j<ARRAYSIZE(color_ids);++j)
            scroll[i][j]=(theme_field)COLOR_FIELD(class_scroll_arrow,scroll_parts[i],0,color_ids[j]);
    theme_field dimensions[]={
        {class_window,WP_CAPTION,0,TMT_HEIGHT,TMT_INT,0,false},
        {class_window,WP_MINBUTTON,0,TMT_WIDTH,TMT_INT,0,false},
        {class_window,WP_MINBUTTON,0,TMT_HEIGHT,TMT_INT,0,false},
        {class_window,WP_SMALLCAPTION,0,TMT_HEIGHT,TMT_INT,0,false},
        {class_window,WP_SMALLCLOSEBUTTON,0,TMT_WIDTH,TMT_INT,0,false},
        {class_window,WP_SMALLCLOSEBUTTON,0,TMT_HEIGHT,TMT_INT,0,false}
    };
    theme_field font_refs[]={
        {class_sysmetrics,0,0,TMT_CAPTIONFONT,TMT_FONT,0,false},
        {class_sysmetrics,0,0,TMT_SMALLCAPTIONFONT,TMT_FONT,0,false},
        {class_sysmetrics,0,0,TMT_MENUFONT,TMT_FONT,0,false}
    };
    msstyles_font *fonts[]={&theme->caption_font,&theme->small_caption_font,&theme->menu_font};
    theme_field frame_colors[]={
        COLOR_FIELD(class_sysmetrics,0,0,TMT_BTNFACE),
        COLOR_FIELD(class_sysmetrics,0,0,TMT_BTNHIGHLIGHT),
        COLOR_FIELD(class_sysmetrics,0,0,TMT_BTNSHADOW),
        COLOR_FIELD(class_sysmetrics,0,0,TMT_WINDOWFRAME)
    };
    theme_field label_colors[]={
        COLOR_FIELD(class_sysmetrics,0,0,TMT_WINDOWTEXT),
        COLOR_FIELD(class_sysmetrics,0,0,TMT_GRAYTEXT)
    };
    theme_field list_colors[]={
        COLOR_FIELD(class_sysmetrics,0,0,TMT_WINDOW),
        COLOR_FIELD(class_sysmetrics,0,0,TMT_HIGHLIGHT),
        COLOR_FIELD(class_sysmetrics,0,0,TMT_HIGHLIGHTTEXT),
        COLOR_FIELD(class_sysmetrics,0,0,TMT_DKSHADOW3D)
    };
    theme_field group_colors[]={
        COLOR_FIELD(class_button,BP_GROUPBOX,GBS_NORMAL,TMT_TEXTCOLOR),
        COLOR_FIELD(class_button,BP_GROUPBOX,GBS_DISABLED,TMT_TEXTCOLOR),
        COLOR_FIELD(class_button,BP_GROUPBOX,0,TMT_EDGESHADOWCOLOR),
        COLOR_FIELD(class_button,BP_GROUPBOX,0,TMT_EDGEHIGHLIGHTCOLOR)
    };
    const unsigned edit_states[]={ETS_NORMAL,ETS_READONLY,ETS_DISABLED};
    theme_field edit[CLASSIC_EDIT_STATE_COUNT][2];
    for(size_t i=0;i<ARRAYSIZE(edit);++i){
        edit[i][0]=(theme_field)COLOR_FIELD(class_edit,EP_EDITTEXT,edit_states[i],TMT_BORDERCOLOR);
        edit[i][1]=(theme_field)COLOR_FIELD(class_edit,EP_EDITTEXT,edit_states[i],TMT_EDGEDKSHADOWCOLOR);
    }
    theme_field combo[]={
        COLOR_FIELD(class_combo,CP_DROPDOWNBUTTON,CBXS_NORMAL,TMT_FILLCOLOR),
        COLOR_FIELD(class_combo,CP_DROPDOWNBUTTON,CBXS_PRESSED,TMT_FILLCOLOR),
        COLOR_FIELD(class_combo,CP_DROPDOWNBUTTON,CBXS_DISABLED,TMT_FILLCOLOR),
        COLOR_FIELD(class_combo,CP_BORDER,CBB_NORMAL,TMT_BORDERCOLOR),
        COLOR_FIELD(class_combo,CP_BORDER,CBB_DISABLED,TMT_BORDERCOLOR)
    };
    theme_field tab[]={
        COLOR_FIELD(class_tab,TABP_PANE,0,TMT_FILLCOLOR),
        COLOR_FIELD(class_tab,TABP_TABITEM,TIS_NORMAL,TMT_FILLCOLOR),
        COLOR_FIELD(class_tab,TABP_TABITEM,TIS_SELECTED,TMT_FILLCOLOR),
        COLOR_FIELD(class_tab,TABP_PANE,0,TMT_BORDERCOLOR)
    };
    theme_field header[]={
        COLOR_FIELD(class_header,HP_HEADERITEM,HIS_NORMAL,TMT_FILLCOLOR),
        COLOR_FIELD(class_header,HP_HEADERITEM,HIS_PRESSED,TMT_FILLCOLOR)
    };
    const unsigned progress_bar[]={PP_BAR,PP_BARVERT};
    const unsigned progress_fill[]={PP_FILL,PP_FILLVERT};
    theme_field progress[2][5];
    for(size_t i=0;i<ARRAYSIZE(progress);++i){
        progress[i][0]=(theme_field)COLOR_FIELD(class_progress,progress_bar[i],0,TMT_FILLCOLOR);
        progress[i][1]=(theme_field)COLOR_FIELD(class_progress,progress_fill[i],PBFS_NORMAL,TMT_FILLCOLOR);
        progress[i][2]=(theme_field)COLOR_FIELD(class_progress,progress_bar[i],0,TMT_BORDERCOLOR);
        progress[i][3]=(theme_field)COLOR_FIELD(class_progress,progress_fill[i],PBFS_ERROR,TMT_FILLCOLOR);
        progress[i][4]=(theme_field)COLOR_FIELD(class_progress,progress_fill[i],PBFS_PAUSED,TMT_FILLCOLOR);
    }
    const unsigned track_part[]={TKP_TRACK,TKP_TRACKVERT},thumb_part[]={TKP_THUMB,TKP_THUMBVERT};
    theme_field trackbar[2][3];
    for(size_t i=0;i<ARRAYSIZE(trackbar);++i){
        trackbar[i][0]=(theme_field)COLOR_FIELD(class_trackbar,track_part[i],0,TMT_FILLCOLOR);
        trackbar[i][1]=(theme_field)COLOR_FIELD(class_trackbar,thumb_part[i],0,TMT_FILLCOLOR);
        trackbar[i][2]=(theme_field)COLOR_FIELD(class_trackbar,track_part[i],0,TMT_BORDERCOLOR);
    }
    theme_field bar[]={
        COLOR_FIELD(class_menu,MENU_BARBACKGROUND,0,TMT_FILLCOLOR),
        COLOR_FIELD(class_menu,MENU_BARITEM,MBI_HOT,TMT_FILLCOLOR),
        COLOR_FIELD(class_menu,MENU_BARITEM,MBI_NORMAL,TMT_TEXTCOLOR),
        COLOR_FIELD(class_menu,MENU_BARITEM,MBI_HOT,TMT_TEXTCOLOR),
        COLOR_FIELD(class_menu,MENU_BARITEM,MBI_DISABLED,TMT_TEXTCOLOR)
    };
    theme_field popup[]={
        COLOR_FIELD(class_menu,MENU_POPUPBACKGROUND,0,TMT_FILLCOLOR),
        COLOR_FIELD(class_menu,MENU_POPUPITEM,MPI_HOT,TMT_FILLCOLOR),
        COLOR_FIELD(class_menu,MENU_POPUPITEM,MPI_NORMAL,TMT_TEXTCOLOR),
        COLOR_FIELD(class_menu,MENU_POPUPITEM,MPI_HOT,TMT_TEXTCOLOR),
        COLOR_FIELD(class_menu,MENU_POPUPITEM,MPI_DISABLED,TMT_TEXTCOLOR),
        COLOR_FIELD(class_menu,MENU_POPUPSEPARATOR,0,TMT_BORDERCOLOR)
    };
    theme_field menu_images[]={
        {class_menu,MENU_POPUPCHECK,0,TMT_IMAGEFILE,TMT_FILENAME,0,false},
        {class_menu,MENU_POPUPSUBMENU,0,TMT_IMAGEFILE,TMT_FILENAME,0,false}
    };
    const uint32_t caption_parts[]={WP_MINBUTTON,WP_MAXBUTTON,WP_RESTOREBUTTON,WP_SYSBUTTON};
    const uint32_t caption_pressed[]={MINBS_PUSHED,MAXBS_PUSHED,RBS_PUSHED,SBS_PUSHED};
    theme_field images[CLASSIC_IMAGE_ROLE_COUNT*2];
    for(size_t i=0;i<CLASSIC_IMAGE_ROLE_COUNT;++i){
        images[2*i]=(theme_field){class_window,caption_parts[i],0,TMT_IMAGEFILE,TMT_FILENAME,0,false};
        images[2*i+1]=(theme_field){class_window,caption_parts[i],caption_pressed[i],TMT_IMAGEFILE,TMT_FILENAME,0,false};
    }
    size_t indices[class_count],class_total;
    HRESULT result=class_indices(map,indices,&class_total);
    if(FAILED(result))return result;
    for(size_t pos=0;pos<variant.size;){
        msstyles_v4_property item;
        size_t used;
        if(!msstyles_v4_property_read(variant.bytes+pos,variant.size-pos,class_total,&item,&used))
            return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        result=read_fields(&item,indices,fields,ARRAYSIZE(fields));
        if(FAILED(result))return result;
        if(item.class_id==indices[class_tab]){
            result=read_fields(&item,indices,tab,ARRAYSIZE(tab));
            if(FAILED(result))return result;
        }
        if(item.class_id==indices[class_header]){
            result=read_fields(&item,indices,header,ARRAYSIZE(header));
            if(FAILED(result))return result;
        }
        if(item.class_id==indices[class_progress])
            for(size_t i=0;i<ARRAYSIZE(progress);++i){
                result=read_fields(&item,indices,progress[i],ARRAYSIZE(progress[i]));
                if(FAILED(result))return result;
            }
        if(item.class_id==indices[class_trackbar])
            for(size_t i=0;i<ARRAYSIZE(trackbar);++i){
                result=read_fields(&item,indices,trackbar[i],ARRAYSIZE(trackbar[i]));
                if(FAILED(result))return result;
            }
        if(item.class_id==indices[class_combo]){
            result=read_fields(&item,indices,combo,ARRAYSIZE(combo));
            if(FAILED(result))return result;
        }
        if(item.class_id==indices[class_button]){
            result=read_fields(&item,indices,push,ARRAYSIZE(push));
            if(FAILED(result))return result;
            result=read_fields(&item,indices,group_colors,ARRAYSIZE(group_colors));
            if(FAILED(result))return result;
            if(!read_choice(&item,&theme->choice))return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }
        if(item.class_id==indices[class_sysmetrics]){
            result=read_fields(&item,indices,font_refs,ARRAYSIZE(font_refs));
            if(FAILED(result))return result;
            result=read_fields(&item,indices,frame_colors,ARRAYSIZE(frame_colors));
            if(FAILED(result))return result;
            result=read_fields(&item,indices,label_colors,ARRAYSIZE(label_colors));
            if(FAILED(result))return result;
            result=read_fields(&item,indices,list_colors,ARRAYSIZE(list_colors));
            if(FAILED(result))return result;
        }
        if(item.class_id==indices[class_edit])
            for(size_t i=0;i<ARRAYSIZE(edit);++i){
                result=read_fields(&item,indices,edit[i],ARRAYSIZE(edit[i]));
                if(FAILED(result))return result;
            }
        if(item.class_id==indices[class_menu]){
            result=read_fields(&item,indices,bar,ARRAYSIZE(bar));
            if(FAILED(result))return result;
            result=read_fields(&item,indices,popup,ARRAYSIZE(popup));
            if(FAILED(result))return result;
            result=read_fields(&item,indices,menu_images,ARRAYSIZE(menu_images));
            if(FAILED(result))return result;
        }
        if(item.class_id==indices[class_window]){
            result=read_fields(&item,indices,dimensions,ARRAYSIZE(dimensions));
            if(FAILED(result))return result;
            result=read_fields(&item,indices,images,ARRAYSIZE(images));
            if(FAILED(result))return result;
        }
        for(size_t i=0;i<ARRAYSIZE(scroll_parts);++i)
            if(item.part_id==scroll_parts[i] && item.class_id==indices[class_scroll_arrow]){
                result=read_fields(&item,indices,scroll[i],ARRAYSIZE(color_ids));
                if(FAILED(result))return result;
                break;
            }
        pos+=used;
    }
    theme->gradient=fields[fill_type].value==FT_HORZGRADIENT;
    unsigned frame_fields=0;
    for(size_t i=0;i<ARRAYSIZE(frame_colors);++i)frame_fields+=frame_colors[i].present;
    if(frame_fields && frame_fields!=ARRAYSIZE(frame_colors))return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    if(frame_fields)theme->frame=(classic_frame_colors){frame_colors[0].value,frame_colors[1].value,
        frame_colors[2].value,frame_colors[3].value,TRUE};
    if(label_colors[0].present && label_colors[1].present)
        theme->label=(classic_text_colors){label_colors[0].value,label_colors[1].value,TRUE};
    if(list_colors[0].present && list_colors[1].present && list_colors[2].present && list_colors[3].present)
        theme->list=(classic_list_colors){list_colors[0].value,list_colors[1].value,
            list_colors[2].value,list_colors[3].value,TRUE};
    if(group_colors[0].present && group_colors[1].present && group_colors[2].present && group_colors[3].present)
        theme->group=(classic_group_colors){{group_colors[0].value,group_colors[1].value,TRUE},
            group_colors[2].value,group_colors[3].value};
    for(size_t i=0;i<ARRAYSIZE(edit);++i){
        if(edit[i][0].present!=edit[i][1].present)return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        if(edit[i][0].present)
            theme->edit[i]=(classic_edit_border){edit[i][0].value,edit[i][1].value,TRUE};
    }
    size_t combo_fields=0;
    for(size_t i=0;i<ARRAYSIZE(combo);++i)combo_fields+=combo[i].present;
    if(combo_fields && combo_fields!=ARRAYSIZE(combo))return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    if(combo_fields)
        theme->combo=(classic_combo_colors){{combo[0].value,combo[1].value,combo[2].value},
            combo[3].value,combo[4].value,TRUE};
    if(tab[0].present && tab[1].present && tab[2].present && tab[3].present)
        theme->tab=(classic_tab_colors){tab[0].value,tab[1].value,tab[2].value,tab[3].value,TRUE};
    if(header[0].present && header[1].present)
        theme->header=(classic_header_colors){header[0].value,header[1].value,TRUE};
    for(size_t i=0;i<ARRAYSIZE(progress);++i){
        const theme_field *p=progress[i];
        if(p[0].present && p[1].present)
            theme->progress[i]=(classic_progress_colors){p[0].value,p[1].value,p[2].value,p[3].value,
                p[4].value,TRUE,p[2].present && p[3].present && p[4].present};
    }
    for(size_t i=0;i<ARRAYSIZE(trackbar);++i){
        const theme_field *t=trackbar[i];
        if(t[0].present && t[1].present && t[2].present)
            theme->trackbar[i]=(classic_trackbar_colors){t[0].value,t[1].value,t[2].value,TRUE};
    }
    for(size_t i=0;i<ARRAYSIZE(menu_images);++i)
        theme->menu_images[i]=(WORD)menu_images[i].value;
    for(size_t i=0;i<ARRAYSIZE(font_refs);++i)
        if(font_refs[i].present && !msstyles_font_read(module,font_refs[i].value,fonts[i]))
            return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    theme_field *menu_fields[]={bar,popup};
    size_t menu_counts[]={ARRAYSIZE(bar),ARRAYSIZE(popup)};
    classic_menu_colors *menus[]={&theme->menu_bar,&theme->menu_popup};
    for(size_t i=0;i<ARRAYSIZE(menus);++i){
        theme_field *rows=menu_fields[i];
        unsigned present=0;
        for(size_t j=0;j<menu_counts[i];++j)present+=rows[j].present;
        if(!present)continue;
        if(present!=menu_counts[i])return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        *menus[i]=(classic_menu_colors){rows[0].value,rows[1].value,rows[2].value,
            rows[3].value,rows[4].value,i?rows[5].value:0,TRUE};
    }
    if(fields[fill_type].value!=FT_SOLID && !theme->gradient)
        return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
    size_t required=theme->gradient?ARRAYSIZE(fields):first_gradient;
    for(size_t i=0;i<required;++i)
        if(!fields[i].present)return HRESULT_FROM_WIN32(ERROR_RESOURCE_DATA_NOT_FOUND);
    theme->active=(classic_caption_colors){fields[active_face].value,fields[active_face].value,fields[active_text].value};
    theme->inactive=(classic_caption_colors){fields[inactive_face].value,fields[inactive_face].value,fields[inactive_text].value};
    if(theme->gradient){
        theme->active.start=fields[first_gradient].value;
        theme->active.end=fields[first_gradient+1].value;
        theme->inactive.start=fields[first_gradient+2].value;
        theme->inactive.end=fields[first_gradient+3].value;
    }
    for(size_t i=0;i<CLASSIC_PUSH_STATE_COUNT;++i)
        theme->push.state[i]=(classic_surface_colors){
            push[2*i].present?push[2*i].value:fields[button_face].value,
            push[2*i+1].present?push[2*i+1].value:fields[button_edge].value};
    theme->scroll_arrow=(classic_surface_colors){fields[scroll_face].value,fields[scroll_edge].value};
    BOOL has_thumb,has_track;
    if(!scroll_colors(scroll,scroll_track_pairs,theme->scroll.thumb,&has_thumb) ||
       !scroll_colors(scroll+scroll_track_pairs,scroll_track_pairs,theme->scroll.track,&has_track) ||
       has_thumb!=has_track ||
       !scroll_colors(scroll+2*scroll_track_pairs,1,&theme->scroll.sizebox,&theme->scroll.own_sizebox))
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    theme->scroll.tracks_and_thumbs=has_thumb;
    for(size_t i=0;i<ARRAYSIZE(dimensions);i+=3){
        if(!dimensions[i].present && !dimensions[i+1].present && !dimensions[i+2].present)continue;
        for(size_t j=0;j<3;++j)
            if(!dimensions[i+j].present || !dimensions[i+j].value || dimensions[i+j].value>UINT16_MAX)
                return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        if(dimensions[i+2].value>dimensions[i].value)return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }
    theme->caption_metrics=(classic_caption_metrics){dimensions[0].value,dimensions[3].value,
        {dimensions[1].value,dimensions[2].value},{dimensions[4].value,dimensions[5].value}};
    for(size_t i=0;i<CLASSIC_IMAGE_ROLE_COUNT;++i)
        for(size_t state=0;state<2;++state)
            theme->caption_images.id[i][state]=(WORD)images[2*i+state].value;
    return S_OK;
}

HRESULT classic_theme_read(HMODULE module,PCWSTR size,PCWSTR color,classic_theme *theme)
{
    if(!module || !size || !color || !theme)return E_INVALIDARG;
    theme_resource version=resource(module,L"PACKTHEM_VERSION",MAKEINTRESOURCEW(1));
    theme_resource extension=resource(module,MAKEINTRESOURCEW(10),L"CLASSIC_STYLE");
    theme_resource variant;
    classic_theme value={0};
    HRESULT result=version.bytes && version.size==2 && version.bytes[0]==4 && !version.bytes[1]?
        variant_resource(module,size,color,&variant):HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
    if(SUCCEEDED(result))result=palette(module,resource(module,L"CMAP",L"CMAP"),variant,&value);
    if(SUCCEEDED(result)){
        enum classic_style_result status=classic_style_read(extension.bytes,extension.size,
            &value.title_buttons,&value.scroll_metrics,&value.menu_metrics);
        if(status!=CLASSIC_STYLE_OK)result=HRESULT_FROM_WIN32(
            status==CLASSIC_STYLE_VERSION || status==CLASSIC_STYLE_REQUIRED_RECORD?
            ERROR_NOT_SUPPORTED:ERROR_INVALID_DATA);
    }
    if(SUCCEEDED(result))*theme=value;
    return result;
}

HRESULT classic_theme_load(PCWSTR path,PCWSTR size,PCWSTR color,classic_theme *theme)
{
    if(!path || !size || !color || !theme)return E_INVALIDARG;
    HMODULE module=LoadLibraryExW(path,NULL,LOAD_LIBRARY_AS_DATAFILE|LOAD_LIBRARY_AS_IMAGE_RESOURCE);
    if(!module)return HRESULT_FROM_WIN32(GetLastError());
    HRESULT result=classic_theme_read(module,size,color,theme);
    FreeLibrary(module);
    return result;
}

BOOL classic_theme_default(HMODULE module,classic_theme *theme)
{
    DWORD prior=GetLastError();
    BOOL loaded=SUCCEEDED(classic_theme_read(module,L"NormalSize",L"NormalColor",theme));
    SetLastError(prior);
    return loaded;
}

int classic_scroll_nominal(classic_scroll_metrics metrics,enum classic_scroll_extent_source fallback,
    const RECT *bar,BOOL vertical,UINT dpi)
{
    unsigned source=metrics.source?metrics.source:fallback;
    if(source==CLASSIC_SCROLL_EXTENT_FIXED)return max(1,MulDiv(metrics.logical_extent,(int)dpi,96));
    if(source==CLASSIC_SCROLL_EXTENT_CROSS_AXIS)return vertical?bar->right-bar->left:bar->bottom-bar->top;
    return GetSystemMetricsForDpi(vertical?SM_CYVSCROLL:SM_CXHSCROLL,dpi);
}
