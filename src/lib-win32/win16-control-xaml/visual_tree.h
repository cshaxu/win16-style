#ifndef WIN16_XAML_VISUAL_TREE_H
#define WIN16_XAML_VISUAL_TREE_H
#include <windows.h>
#include <inspectable.h>

/* Invoked on the XAML UI thread. Object is borrowed for the call. Return
 * FALSE to stop walking. The first object is at depth zero. */
typedef BOOL (*win16_xaml_ancestor_visitor)(IInspectable *object, unsigned int depth, void *context);
HRESULT win16_xaml_visit_ancestors(IInspectable *start, win16_xaml_ancestor_visitor visitor, void *context);
/* Bounded traversal of an already existing subtree on its owning UI thread. */
HRESULT win16_xaml_visit_descendants(IInspectable *start, win16_xaml_ancestor_visitor visitor, void *context);
#endif
