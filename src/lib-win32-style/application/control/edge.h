#ifndef STYLE_CONTROL_EDGE_H
#define STYLE_CONTROL_EDGE_H
#include "../lifecycle/instance.h"
typedef enum style_control_edge_kind {STYLE_EDGE_DATA,STYLE_EDGE_USER,STYLE_EDGE_GRID} style_control_edge_kind;
typedef struct style_control_edge_state {
    BOOL candidate,attempted,installed;
    HWND parent;
    HMODULE owner;
} style_control_edge_state;
typedef struct style_control_edge_ops {
    void *context;
    BOOL (*eligible)(HWND,HWND,style_control_edge_kind,void *);
    BOOL (*install)(HWND,void *,const BOOL *);
    BOOL (*remove)(HWND,const void *);
    BOOL (*scroll_info)(HWND,int,LPSCROLLINFO);
} style_control_edge_ops;
/* Caller pins instance/state throughout native queries and complete policy
 * operations. Delayed roles publish their parent/edge before installation;
 * the grid checks its native scroll protocol before publishing success. */
void style_control_edge_settle(HWND,UINT,style_instance *,style_control_edge_state *,style_control_edge_kind,const style_control_edge_ops *);
#endif
