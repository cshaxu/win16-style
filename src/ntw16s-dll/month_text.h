#ifndef NTW16S_MONTH_TEXT_H
#define NTW16S_MONTH_TEXT_H
#include <windows.h>
/* The witnessed western date strings may contain U+200E bidi marks.
 * Other scripts and directional controls retain the native renderer. */
static inline BOOL ntw16s_month_text_length(LPCWSTR text,int count,int *length)
{
    int n=0;if(!text || !length || count < -1 || count > 128)return FALSE;
    if(count==-1){while(n<128 && text[n])++n;if(n==128)return FALSE;}else n=count;
    if(!n)return FALSE;
    for(int i=0;i<n;++i)if((text[i]<32 || text[i]>126) && text[i]!=0x200e)return FALSE;
    *length=n;return TRUE;
}
static inline BOOL ntw16s_month_title_fits(const RECT *r,const RECT *prev,const RECT *next,int width)
{
    if(!r || !prev || !next || width<0 || width>r->right-r->left)return FALSE;
    int left=r->left+(r->right-r->left-width)/2,right=left+width;
    return (right<=prev->left || left>=prev->right) &&
           (right<=next->left || left>=next->right);
}
#endif
