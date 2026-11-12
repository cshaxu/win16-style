#include "lib-style/common/modern_file_dialog.h"

int main(void)
{
    WCHAR path[2];
    if (style_dialog_show_modern(NULL, 1, NULL, 0, NULL) != E_INVALIDARG) return 1;
    if (style_dialog_show_modern(NULL, 1, path, 0, NULL) != E_INVALIDARG) return 2;
    if (style_dialog_show_modern(NULL, 0, path, 2, NULL) != E_INVALIDARG) return 3;
    if (style_dialog_show_modern(NULL, 4, path, 2, NULL) != E_INVALIDARG) return 4;
    return 0;
}
