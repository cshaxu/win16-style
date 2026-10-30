#include "print_ranges.h"
#include <stdint.h>
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
