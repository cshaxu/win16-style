#ifndef RUN16S_LEGACY_SHELL_H
#define RUN16S_LEGACY_SHELL_H
#include "elevation.h"
BOOL run16s_legacy_image(PCWSTR image);
/* Only the rejected legacy-image path delegates to Shell. Native PE targets
 * must keep the suspended creation/injection transaction, including UAC. */
DWORD run16s_legacy_shell(PCWSTR image,PCWSTR arguments,run16s_shell_execute execute);
#endif
