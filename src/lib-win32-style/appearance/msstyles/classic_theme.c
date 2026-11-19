#include "classic_theme.h"
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

enum theme_class { class_window, class_button, class_scroll_arrow, class_count };

static HRESULT class_indices(theme_resource map,size_t indices[class_count],size_t *count)
{
    const PCWSTR names[class_count]={L"Window",L"Button",L"ScrollBar"};
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
    for(unsigned i=0;i<class_count;++i)
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

static HRESULT read_fields(const msstyles_v4_property *item,const size_t indices[class_count],
    theme_field *fields,size_t count)
{
    for(size_t i=0;i<count;++i){
        theme_field *field=&fields[i];
        if(item->class_id!=indices[field->cls] || item->part_id!=field->part ||
           item->state_id!=field->state || item->name_id!=field->name)continue;
        if(field->present || item->type_id!=field->type)
            return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        if(field->type==TMT_FILENAME){
            if(item->data || item->declared_size!=16 || !item->short_value || item->short_value>UINT16_MAX)
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

static HRESULT palette(theme_resource map,theme_resource variant,classic_theme *theme)
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
        if(item.class_id==indices[class_button] && item.part_id==BP_PUSHBUTTON){
            result=read_fields(&item,indices,push,ARRAYSIZE(push));
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
    if(SUCCEEDED(result))result=palette(resource(module,L"CMAP",L"CMAP"),variant,&value);
    if(SUCCEEDED(result)){
        enum classic_style_result status=classic_style_read(extension.bytes,extension.size,
            &value.title_buttons,&value.scroll_metrics);
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
