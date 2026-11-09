#ifndef WIN16_XAML_OWNER_H
#define WIN16_XAML_OWNER_H
#include <windows.h>
#include <inspectable.h>
/* Called on the element's UI thread. Failure leaves *owner NULL. */
HRESULT win16_xaml_element_owner(IInspectable *element,HWND *owner);
#endif
