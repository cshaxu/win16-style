#include "lib-style/common/classic_print_contract.h"
#include <stdio.h>

static UINT_PTR CALLBACK original(HWND w,UINT m,WPARAM a,LPARAM b)
{(void)w;(void)m;(void)a;(void)b;return 0;}
static UINT_PTR CALLBACK replacement(HWND w,UINT m,WPARAM a,LPARAM b)
{(void)w;(void)m;(void)a;(void)b;return 0;}
#define CHECK(x) do{if(!(x)){fprintf(stderr,"print contract line %d\n",__LINE__);return 1;}}while(0)
#define TEST(type,suffix) do { \
    type in={0},out={0};in.lStructSize=sizeof(in);in.Flags=PD_PAGENUMS;in.lpfnPrintHook=original; \
    CHECK(!style_classic_print_prepare_##suffix(NULL,&out,PD_PAGENUMS,replacement)); \
    CHECK(!style_classic_print_prepare_##suffix(&in,NULL,PD_PAGENUMS,replacement)); \
    CHECK(!style_classic_print_prepare_##suffix(&in,&out,PD_PAGENUMS,NULL)); \
    CHECK(!style_classic_print_prepare_##suffix(&in,&out,0,replacement)); \
    CHECK(style_classic_print_prepare_##suffix(&in,&out,PD_PAGENUMS,replacement)); \
    CHECK((out.Flags&PD_ENABLEPRINTHOOK) && out.lpfnPrintHook==replacement); \
    out.nCopies=7;out.Flags|=PD_COLLATE;style_classic_print_result_##suffix(&in,&out); \
    CHECK(in.nCopies==7 && (in.Flags&PD_COLLATE) && !(in.Flags&PD_ENABLEPRINTHOOK) && in.lpfnPrintHook==original); \
    in.Flags=PD_ENABLEPRINTHOOK;CHECK(style_classic_print_prepare_##suffix(&in,&out,PD_ENABLEPRINTHOOK,replacement)); \
    style_classic_print_result_##suffix(&in,&out);CHECK((in.Flags&PD_ENABLEPRINTHOOK) && in.lpfnPrintHook==original); \
    in.lStructSize=1;CHECK(!style_classic_print_prepare_##suffix(&in,&out,~0u,replacement)); \
} while(0)
int main(void)
{
    TEST(PRINTDLGA,a);TEST(PRINTDLGW,w);
    puts("CLASSIC_PRINT_CONTRACT_OK ansi=1 unicode=1 policy=1 restoration=1");return 0;
}
