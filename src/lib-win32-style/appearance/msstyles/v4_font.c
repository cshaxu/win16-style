#include "v4_font.h"
#include <wchar.h>
#include <string.h>

static WCHAR *next_field(WCHAR **remaining)
{
    WCHAR *word=*remaining;
    if(!word)return NULL;
    WCHAR *separator=wcschr(word,L',');
    *remaining=separator?separator+1:NULL;
    if(separator)*separator=0;
    while(*word==L' ' || *word==L'\t')++word;
    WCHAR *end=word+wcslen(word);
    while(end>word && (end[-1]==L' ' || end[-1]==L'\t'))--end;
    *end=0;
    return word;
}

static bool font_quality(PCWSTR field,BYTE *quality)
{
    static const struct {PCWSTR label;BYTE quality;} known[]={
        {L"Default",DEFAULT_QUALITY},{L"Draft",DRAFT_QUALITY},{L"Proof",PROOF_QUALITY},
        {L"NonAntialiased",NONANTIALIASED_QUALITY},{L"Antialiased",ANTIALIASED_QUALITY},
        {L"ClearType",CLEARTYPE_QUALITY},{L"ClearTypeNatural",CLEARTYPE_NATURAL_QUALITY}
    };
    for(unsigned i=0;i<ARRAYSIZE(known);++i)
        if(!_wcsicmp(field,known[i].label)){
            *quality=known[i].quality;
            return true;
        }
    return false;
}

bool msstyles_font_read(HMODULE module,UINT string_id,msstyles_font *font)
{
    if(!module || !string_id || !font)return false;
    PCWSTR source=NULL;
    int length=LoadStringW(module,string_id,(PWSTR)&source,0);
    WCHAR copy[160];
    if(length<=0 || (size_t)length>=ARRAYSIZE(copy))return false;
    memcpy(copy,source,(size_t)length*sizeof(*source));
    copy[length]=0;
    if(wcslen(copy)!=(size_t)length)return false;

    WCHAR *remaining=copy;
    PCWSTR face=next_field(&remaining),size=next_field(&remaining);
    if(!*face || wcslen(face)>=LF_FACESIZE || !size || *size<L'0' || *size>L'9')return false;
    WCHAR *end;
    long points=wcstol(size,&end,10);
    if(*end || points<1 || points>128)return false;
    msstyles_font value={.logical={.lfWeight=FW_NORMAL,.lfCharSet=DEFAULT_CHARSET},.points=(unsigned)points};
    wcscpy(value.logical.lfFaceName,face);
    PCWSTR option;
    while((option=next_field(&remaining))){
        if(!_wcsicmp(option,L"Bold"))value.logical.lfWeight=FW_BOLD;
        else if(!_wcsicmp(option,L"Italic"))value.logical.lfItalic=TRUE;
        else if(!_wcsicmp(option,L"Underline"))value.logical.lfUnderline=TRUE;
        else if(!_wcsicmp(option,L"Strikeout"))value.logical.lfStrikeOut=TRUE;
        else if(_wcsnicmp(option,L"Quality:",8) || !font_quality(option+8,&value.logical.lfQuality))return false;
    }
    *font=value;
    return true;
}
