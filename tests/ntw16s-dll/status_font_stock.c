#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
static void *watched;static unsigned freed_watched;
static void watched_free(void *p);
#define free watched_free
#include "../../src/ntw16s-dll/font.c"
#undef free
static void watched_free(void *p){if(p&&p==watched)++freed_watched;free(p);}
#define CHECK(x) do{if(!(x)){fprintf(stderr,"STOCK_FAIL line=%d expr=%s\n",__LINE__,#x);return 1;}}while(0)
int main(void){HFONT source=CreateFontW(-18,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,0,0,L"Arial");CHECK(source);HFONT shared=win16_style_system_acquire(source);CHECK(shared&&shared!=source&&variants);watched=variants;HFONT stock=(HFONT)GetStockObject(SYSTEM_FONT);CHECK(stock);HFONT borrowed=win16_style_system_acquire_isolated(stock);CHECK(borrowed==stock);CHECK(freed_watched==0);CHECK(variants==watched);CHECK(win16_style_system_source(shared)==source);CHECK(win16_style_system_try_retain(shared));win16_style_system_release(shared);win16_style_system_release(shared);CHECK(DeleteObject(source));puts("STOCK_VARIANT_OWNERSHIP_OK");return 0;}
