#include "control_lifetime.h"
#include "spin_identity.h"
#include "font.h"
#include <commctrl.h>

HWND win16_control_spin_before(HWND w,UINT m,RECT *before)
{
    HWND button=NULL;DWORD error=GetLastError();
    if(m==WM_SETFONT && (button=win16_control_spinedit_button(w))!=NULL)
        SendMessageW(w,EM_GETRECT,0,(LPARAM)before);
    SetLastError(error);return button;
}
void win16_control_finish_message(HWND w,control_style *s,UINT m,LPARAM b,HWND spin_button,const RECT *spin_before,BOOL combo_paint,HFONT previous,HFONT outbound,HFONT pinned,SUBCLASSPROC callback)
{
    if(spin_button && !s->detached && IsWindow(w) && win16_control_spinedit_button(w)==spin_button){
        DWORD error=GetLastError();RECT button,after;
        if(GetWindowRect(spin_button,&button)){
            MapWindowPoints(NULL,w,(POINT*)&button,2);
            SendMessageW(w,EM_GETRECT,0,(LPARAM)&after);
            if(!s->detached && spin_before->right>spin_before->left && spin_before->right<=button.left &&
               after.right>button.left && spin_before->right>after.left){
                after.right=spin_before->right;SendMessageW(w,EM_SETRECTNP,0,(LPARAM)&after);
            }
        }
        SetLastError(error);
    }
    if(m==WM_SETFONT && s->detached && IsWindow(w)){
        /* A native setter can finish AFTER its callback removed this leaf,
         * overwriting the font restored during removal with our retired
         * variant. Settle only after that native call has returned. A new
         * independent owner, if installed reentrantly, keeps its own source. */
        DWORD_PTR successor;HFONT source=s->original_font;
        if(GetWindowSubclass(w,callback,(UINT_PTR)callback,&successor))
            source=((control_style *)successor)->original_font;
        SendMessageW(w,WM_SETFONT,(WPARAM)source,b);
    }
    if(combo_paint)--s->combo_paints;
    /* Native processing (including nested paint/remove/setfont) has finished
     * with the previous lease. Never delete the application's borrowed font. */
    win16_style_system_release(previous);
    win16_style_system_release(outbound);win16_style_system_release(pinned);
    if(!--s->calls && s->detached)win16_control_state_dispose(s);
}
