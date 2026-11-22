#include "v3_theme.h"
#include <stdint.h>
#include <wchar.h>

static const void *resource(HMODULE module,PCWSTR name,PCWSTR type,DWORD *length)
{
    HRSRC item=FindResourceW(module,name,type);
    if(!item)return NULL;
    *length=SizeofResource(module,item);
    if(!*length)return NULL;
    HGLOBAL loaded=LoadResource(module,item);
    return loaded?LockResource(loaded):NULL;
}

static msstyles_v3_text text_resource(HMODULE module,PCWSTR name,PCWSTR type)
{
    DWORD length=0;
    const WCHAR *data=resource(module,name,type,&length);
    return data && !(length%sizeof(*data))?
        (msstyles_v3_text){data,length/sizeof(*data)}:(msstyles_v3_text){0};
}

static BOOL equal(msstyles_v3_text text,PCWSTR name)
{
    return text.length==wcslen(name) && !_wcsnicmp(text.data,name,text.length);
}

static HRESULT table(msstyles_v3_text text,PCWSTR wanted,size_t index,
    size_t *count,size_t *selected,PCWSTR *name)
{
    if(!text.data)return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    size_t pos=0,total=0,match=SIZE_MAX;
    PCWSTR entry=NULL;
    while(pos<text.length && text.data[pos]){
        size_t first=pos;
        while(pos<text.length && text.data[pos])++pos;
        size_t length=pos-first;
        if(pos==text.length || length>=64)return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        if(wanted && equal((msstyles_v3_text){text.data+first,length},wanted)){
            if(match!=SIZE_MAX)return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            match=total;
        }
        if(total==index)entry=text.data+first;
        ++total;
        ++pos;
    }
    if(!total || pos==text.length)return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    while(pos<text.length)
        if(text.data[pos++])return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    if(wanted && match==SIZE_MAX)return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    *count=total;
    if(selected)*selected=match;
    if(name)*name=entry;
    return S_OK;
}

HRESULT msstyles_v3_read(HMODULE resources,PCWSTR size,PCWSTR color,msstyles_v3_theme *theme)
{
    if(!resources || !size || !*size || !color || !*color || !theme)return E_INVALIDARG;
    DWORD length=0;
    PCWSTR root=MAKEINTRESOURCEW(1);
    const BYTE *version=resource(resources,root,L"PACKTHEM_VERSION",&length);
    if(!version || length!=2)return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    if(version[0]!=3 || version[1])return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
    size_t colors,sizes,color_index,size_index,variants;
    HRESULT status=table(text_resource(resources,root,L"COLORNAMES"),color,0,&colors,&color_index,NULL);
    if(FAILED(status))return status;
    status=table(text_resource(resources,root,L"SIZENAMES"),size,0,&sizes,&size_index,NULL);
    if(FAILED(status))return status;
    if(colors>SIZE_MAX/sizes)return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    PCWSTR variant=NULL;
    status=table(text_resource(resources,root,L"FILERESNAMES"),NULL,color_index*sizes+size_index,
        &variants,NULL,&variant);
    if(FAILED(status))return status;
    if(variants!=colors*sizes || !variant)return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    msstyles_v3_text ini=text_resource(resources,variant,L"TEXTFILE");
    if(!ini.length)return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    *theme=(msstyles_v3_theme){resources,variant,ini};
    return S_OK;
}

static BOOL space(WCHAR c)
{return c==L' ' || c==L'\t' || c==L'\r';}

static msstyles_v3_text trim(const WCHAR *start,const WCHAR *end)
{
    while(start<end && space(*start))++start;
    while(end>start && space(end[-1]))--end;
    return (msstyles_v3_text){start,(size_t)(end-start)};
}

BOOL msstyles_v3_property(const msstyles_v3_theme *theme,PCWSTR section,PCWSTR key,
    msstyles_v3_text *value)
{
    if(!theme || !theme->properties.data || !section || !*section || !key || !*key || !value)return FALSE;
    const WCHAR *line=theme->properties.data,*end=line+theme->properties.length;
    if(line<end && *line==0xfeff)++line;
    BOOL match=FALSE;
    while(line<end && *line){
        const WCHAR *next=line;
        while(next<end && *next && *next!=L'\n')++next;
        const WCHAR *content=line;
        while(content<next && *content!=L';')++content;
        msstyles_v3_text text=trim(line,content);
        if(text.length>=2 && text.data[0]==L'[' && text.data[text.length-1]==L']'){
            match=equal(trim(text.data+1,text.data+text.length-1),section);
        }else if(match && text.length){
            const WCHAR *separator=wmemchr(text.data,L'=',text.length);
            if(separator && equal(trim(text.data,separator),key)){
                *value=trim(separator+1,text.data+text.length);
                return TRUE;
            }
        }
        line=next;
        if(line<end && *line==L'\n')++line;
    }
    return FALSE;
}

BOOL msstyles_v3_bitmap(const msstyles_v3_theme *theme,msstyles_v3_text image,
    const void **data,DWORD *length)
{
    WCHAR key[128];
    if(!theme || !theme->resources || !image.data || !image.length ||
       image.length>=ARRAYSIZE(key) || !data || !length)return FALSE;
    for(size_t i=0;i<image.length;++i){
        WCHAR c=image.data[i];
        if(c>=L'a' && c<=L'z')c-=L'a'-L'A';
        if(c==L'\\' || c==L'/' || c==L'.')c=L'_';
        if(!(c>=L'A' && c<=L'Z') && !(c>=L'0' && c<=L'9') && c!=L'_' && c!=L'-')return FALSE;
        key[i]=c;
    }
    key[image.length]=0;
    DWORD bytes=0;
    const void *bitmap=resource(theme->resources,key,MAKEINTRESOURCEW((ULONG_PTR)RT_BITMAP),&bytes);
    if(!bitmap || bytes<sizeof(BITMAPINFOHEADER))return FALSE;
    *data=bitmap;*length=bytes;
    return TRUE;
}