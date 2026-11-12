#include "lib-style/common/text_metrics.h"
#include <stdio.h>
int main(void)
{
    HDC dc=CreateCompatibleDC(NULL);SIZE extent;int average=-1;
    if(!dc || !GetTextExtentPoint32W(dc,L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",52,&extent) ||
       !style_text_alphabet_average(dc,&average) || average!=((extent.cx/26)+1)/2 ||
       style_text_alphabet_average(NULL,&average) || !DeleteDC(dc))return 1;
    puts("TEXT_METRICS_OK average=1 invalid_dc=1");return 0;
}
