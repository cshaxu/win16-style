#include "lib-style/common/classic_com_show.h"
#include "lib-style/common/classic_browse.h"
static BOOL inactive(void) { return FALSE; }
static HRESULT native(void *owner, HWND window)
{ return owner && window==(HWND)(ULONG_PTR)42 ? E_ABORT : E_FAIL; }
int main(void)
{
    int owner=1; BOOL completed=TRUE; IShellItem *item=NULL; IShellItemArray *items=NULL;
    style_classic_com_show_state state={.owner=&owner,.result=&item,.results=&items,
        .completed=&completed,.session_active=inactive,.show_native=native};
    if(style_classic_com_show(&state,(HWND)(ULONG_PTR)42)!=E_ABORT || completed) return 1;
    UINT flags=BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE;
    style_classic_browse_plan p=style_classic_plan_browse(flags,FALSE);
    if(p.mode!=STYLE_CLASSIC_BROWSE_REWRITE || p.flags!=BIF_RETURNONLYFSDIRS) return 2;
    p=style_classic_plan_browse(flags,TRUE);
    if(p.mode!=STYLE_CLASSIC_BROWSE_NATIVE || p.flags!=flags) return 3;
    return 0;
}
