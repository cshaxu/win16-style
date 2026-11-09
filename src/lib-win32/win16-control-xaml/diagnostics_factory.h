#ifndef WIN16_XAML_DIAGNOSTICS_FACTORY_H
#define WIN16_XAML_DIAGNOSTICS_FACTORY_H
#include "diagnostics_site.h"

/* The DLL owner keeps its module and context alive while COM objects and
 * subscription workers exist; the factory does not implement server locks. */
HRESULT win16_xaml_get_class_object(REFCLSID requested,REFCLSID supported,
                                    const win16_xaml_site_handlers *handlers,void *context,
                                    REFIID iid,void **object);
#endif
