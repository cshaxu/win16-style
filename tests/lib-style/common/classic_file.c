#define COBJMACROS
#include <windows.h>
#include <stdio.h>
#include "lib-style/common/classic_file_contract.h"
#include "lib-style/common/classic_shell_item.h"
#define CHECK(x) do { if (!(x)) { printf("CLASSIC FILE FAIL line=%d\n", __LINE__); return 1; } } while (0)
int main(void)
{
    OPENFILENAMEW source={0}, legacy={0};
    source.lStructSize=sizeof(source); source.Flags=OFN_EXPLORER|OFN_FILEMUSTEXIST;
    CHECK(style_classic_file_prepare_w(&source,&legacy,0,NULL));
    CHECK(!(legacy.Flags&OFN_EXPLORER) && (legacy.Flags&OFN_ENABLEHOOK));
    legacy.nFileOffset=4; style_classic_file_result_w(&source,&legacy);
    CHECK(source.nFileOffset==4 && (source.Flags&OFN_EXPLORER));
    source.Flags=OFN_ALLOWMULTISELECT;
    CHECK(!style_classic_file_prepare_w(&source,&legacy,0,NULL));
    CHECK(!style_classic_com_can_bridge(S_OK,FOS_ALLOWMULTISELECT,FALSE,FALSE,FALSE,FALSE,FALSE));
    COMDLG_FILTERSPEC spec={L"Text",L"*.txt"}; WCHAR *filter=NULL;
    CHECK(style_dialog_format_filters(1,&spec,&filter));
    CHECK(!lstrcmpW(filter,L"Text") && !lstrcmpW(filter+5,L"*.txt") && filter[11]==0);
    CoTaskMemFree(filter);
    HRESULT init=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);
    CHECK(SUCCEEDED(init));
    WCHAR windows[MAX_PATH]; IShellItem *item=NULL;
    BOOL ok=GetWindowsDirectoryW(windows,MAX_PATH)>0 && SUCCEEDED(style_classic_open_item(windows,FALSE,&item)) && item;
    if(item)IShellItem_Release(item);
    CoUninitialize();
    CHECK(ok);
    puts("CLASSIC FILE PASS independent contract filters shell");
    return 0;
}
