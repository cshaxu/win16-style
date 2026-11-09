#include "lib-win32/win16-control-xaml/adapter.h"
#include <stddef.h>
#include <stdio.h>
/* Test-side mirror of the private entry, used only to inspect our own target. */
typedef struct observed_entry {
    void *next;
    IUnknown *identity;
    BOOL installing;
    win16_xaml_button_style style;
} observed_entry;
int main(void)
{
    printf("%zu %zu %zu %zu\n",offsetof(win16_xaml_adapter,session)+offsetof(win16_xaml_session,buttons)+offsetof(win16_xaml_button_set,first),
           offsetof(observed_entry,installing),offsetof(observed_entry,style)+offsetof(win16_xaml_button_style,installed_style),
           offsetof(win16_xaml_adapter,session)+offsetof(win16_xaml_session,thread_id));
    return 0;
}
