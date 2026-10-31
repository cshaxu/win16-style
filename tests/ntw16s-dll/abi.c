/* Real-DLL C ABI smoke; the full runtime/visual boundary follows separately. */
#include <windows.h>
#include <stdio.h>
#include "ntw16s-dll/api.h"
#define CHECK(x) do {if(!(x)){fprintf(stderr,"ABI FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
static const char *const names[]={
    "Ntw16sCreationBegin",
    "Ntw16sCreationEnd",
    "Ntw16sGetAbiVersion",
    "Ntw16sGlobalHook",
    "Ntw16sGlobalHost",
    "Ntw16sScrollChanged",
    "Ntw16sShowModernFileDialog",
    "Ntw16sSetWindowTheme",
    "Ntw16sWindowAttached",
    "Ntw16sWindowRemove",
    "Ntw16sWindowTerminal",
    "win16_style_control_install",
    "win16_style_control_remove",
    "win16_style_controls_begin",
    "win16_style_controls_end",
    "win16_style_default_palette",
    "win16_style_edit_install",
    "win16_style_edit_scrollbars_enable",
    "win16_style_font_acquire",
    "win16_style_font_release",
    "win16_style_frame_scrollbars_install",
    "win16_style_frame_scrollbars_palette",
    "win16_style_frame_scrollbars_remove",
    "win16_style_get_scroll_info",
    "win16_style_hit_test",
    "win16_style_install",
    "win16_style_list_scrollbar_enable",
    "win16_style_menu_measure",
    "win16_style_menu_text",
    "win16_style_remove",
    "win16_style_scrollbars_paint",
    "win16_style_set_palette",
    "win16_style_set_scroll_info",
    "win16_style_tabbed_list_install"
};
int wmain(int argc,WCHAR **argv)
{
    HMODULE module;unsigned i;
    typedef DWORD (__cdecl *version_fn)(void);
    version_fn version;
    CHECK(argc==2);
    module=LoadLibraryExW(argv[1],NULL,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
    CHECK(module);
    for(i=0;i<sizeof(names)/sizeof(names[0]);++i)CHECK(GetProcAddress(module,names[i]));
    CHECK(!GetProcAddress(module,"Ntw16sBindNativeApi"));
    CHECK(!GetProcAddress(module,"Ntw16sBindComboInfoApi"));
    CHECK(!GetProcAddress(module,"ntw16s_bind_native_api"));
    version=(version_fn)(void *)GetProcAddress(module,"Ntw16sGetAbiVersion");
    CHECK(version && version()==3 && version()==NTW16S_ABI_VERSION);
    CHECK(FreeLibrary(module));
    printf("ABI PASS width=%u version=3 public=34 private-bind-exports=0\n",(unsigned)(sizeof(void *)*8));
    return 0;
}
