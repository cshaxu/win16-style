#include "print_ranges.h"
#include <stdint.h>
#include <wchar.h>
static BOOL read_number(const WCHAR **cursor,DWORD *result)
{
    const WCHAR *p=*cursor;DWORD value=0;
    if(*p<L'0' || *p>L'9')return FALSE;
    do {DWORD digit=(DWORD)(*p-L'0');if(value>(UINT32_MAX-digit)/10)return FALSE;
        value=value*10+digit;++p;
    } while(*p>=L'0' && *p<=L'9');
    *cursor=p;*result=value;return TRUE;
}
static BOOL parse_pass(const WCHAR *text,DWORD minimum,DWORD maximum,
                       PRINTPAGERANGE *output,DWORD capacity,DWORD *count)
{
    DWORD length=0;const WCHAR *p=text;
    for(;;){DWORD first,last;if(!read_number(&p,&first))return FALSE;last=first;
        if(*p==L'-'){++p;if(!read_number(&p,&last))return FALSE;}
        if(first<minimum || last<first || last>maximum || length>=capacity)return FALSE;
        if(output){output[length].nFromPage=first;output[length].nToPage=last;}
        ++length;if(!*p){*count=length;return TRUE;}
        if(*p!=L',')return FALSE;
        ++p;
    }
}
BOOL ntw16s_parse_print_ranges(const WCHAR *text,DWORD minimum,DWORD maximum,
                               PRINTPAGERANGE *output,DWORD capacity,DWORD *count)
{
    DWORD length=0;if(!text || !output || !count || !capacity || minimum>maximum)return FALSE;
    if(!parse_pass(text,minimum,maximum,NULL,capacity,&length))return FALSE;
    return parse_pass(text,minimum,maximum,output,capacity,count);
}

BOOL ntw16s_format_print_ranges(const PRINTPAGERANGE *ranges,DWORD count,DWORD minimum,DWORD maximum,
                                WCHAR *output,DWORD capacity)
{
    if(!ranges || !count || !output || !capacity || minimum>maximum)return FALSE;
    DWORD needed=0;
    for(DWORD i=0;i<count;++i){WCHAR item[32];DWORD first=ranges[i].nFromPage,last=ranges[i].nToPage;
        if(first<minimum || last<first || last>maximum)return FALSE;
        int length=first==last?swprintf(item,32,L"%lu",first):swprintf(item,32,L"%lu-%lu",first,last);
        if(length<0 || needed>capacity-1 || (DWORD)length>capacity-1-needed)return FALSE;
        needed+=(DWORD)length;
        if(i+1<count){if(needed==capacity-1)return FALSE;++needed;}}
    DWORD pos=0;
    for(DWORD i=0;i<count;++i){int n=ranges[i].nFromPage==ranges[i].nToPage?
            swprintf(output+pos,capacity-pos,L"%lu",ranges[i].nFromPage):
            swprintf(output+pos,capacity-pos,L"%lu-%lu",ranges[i].nFromPage,ranges[i].nToPage);
        if(n<0)return FALSE;
        pos+=(DWORD)n;if(i+1<count)output[pos++]=L',';}
    output[pos]=0;return TRUE;
}
