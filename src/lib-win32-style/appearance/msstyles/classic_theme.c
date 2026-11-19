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
    size_t indices[class_count],class_total;
    HRESULT result=class_indices(map,indices,&class_total);
    if(FAILED(result))return result;
    for(size_t pos=0;pos<variant.size;){
        msstyles_v4_property item;
        size_t used;
        if(!msstyles_v4_property_read(variant.bytes+pos,variant.size-pos,class_total,&item,&used))
            return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        for(size_t i=0;i<ARRAYSIZE(fields);++i){
            theme_field *field=&fields[i];
            if(item.class_id!=indices[field->cls] || item.part_id!=field->part ||
               item.state_id!=field->state || item.name_id!=field->name)continue;
            if(field->present || item.type_id!=field->type || !item.data || item.declared_size!=4)
                return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            field->value=msstyles_v4_read_word(item.data);
            field->present=true;
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
    theme->button=(classic_surface_colors){fields[button_face].value,fields[button_edge].value};
    theme->scroll_arrow=(classic_surface_colors){fields[scroll_face].value,fields[scroll_edge].value};
    return S_OK;
}

HRESULT classic_theme_load(PCWSTR path,PCWSTR size,PCWSTR color,classic_theme *theme)
{
    if(!path || !size || !color || !theme)return E_INVALIDARG;
    HMODULE module=LoadLibraryExW(path,NULL,LOAD_LIBRARY_AS_DATAFILE|LOAD_LIBRARY_AS_IMAGE_RESOURCE);
    if(!module)return HRESULT_FROM_WIN32(GetLastError());
    theme_resource version=resource(module,L"PACKTHEM_VERSION",MAKEINTRESOURCEW(1));
    theme_resource extension=resource(module,MAKEINTRESOURCEW(10),L"CLASSIC_STYLE");
    theme_resource variant;
    classic_theme value={0};
    HRESULT result=version.bytes && version.size==2 && version.bytes[0]==4 && !version.bytes[1]?
        variant_resource(module,size,color,&variant):HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
    if(SUCCEEDED(result))result=palette(resource(module,L"CMAP",L"CMAP"),variant,&value);
    if(SUCCEEDED(result)){
        enum classic_style_result status=classic_style_read(extension.bytes,extension.size,&value.title_buttons);
        if(status!=CLASSIC_STYLE_OK)result=HRESULT_FROM_WIN32(
            status==CLASSIC_STYLE_VERSION || status==CLASSIC_STYLE_REQUIRED_RECORD?
            ERROR_NOT_SUPPORTED:ERROR_INVALID_DATA);
    }
    FreeLibrary(module);
    if(SUCCEEDED(result))*theme=value;
    return result;
}
