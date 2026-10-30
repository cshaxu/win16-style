#include <windows.h>
#include <stdio.h>
#include "ntw16s-dll/injection/print_ranges.h"
#define CHECK(x) do {if(!(x)){printf("PRINT RANGES FAIL line=%d\n",__LINE__);return 1;}} while(0)
int main(void)
{
    PRINTPAGERANGE ranges[4];DWORD count=99;
    CHECK(ntw16s_parse_print_ranges(L"4-5,2,2,4-7",1,1000,ranges,4,&count));
    CHECK(count==4 && ranges[0].nFromPage==4 && ranges[0].nToPage==5 &&
          ranges[1].nFromPage==2 && ranges[2].nFromPage==2 && ranges[3].nToPage==7);
    const WCHAR *bad[]={L"",L"1,",L",1",L"3-2",L"0",L"1001",L"1-",L"1,,2",L"4294967296",L"1 2",L"1,2,3,4,5"};
    for(unsigned i=0;i<sizeof(bad)/sizeof(bad[0]);++i){ranges[0].nFromPage=77;count=88;
        CHECK(!ntw16s_parse_print_ranges(bad[i],1,1000,ranges,4,&count));
        CHECK(ranges[0].nFromPage==77 && count==88);}
    CHECK(ntw16s_parse_print_ranges(L"4294967295",0,0xffffffffu,ranges,4,&count));
    CHECK(count==1 && ranges[0].nFromPage==0xffffffffu);
    CHECK(!ntw16s_parse_print_ranges(NULL,1,1000,ranges,4,&count));
    CHECK(!ntw16s_parse_print_ranges(L"1",2,1,ranges,4,&count));
    puts("PRINT RANGES PASS order duplicates overlap bounds overflow atomic");return 0;
}
