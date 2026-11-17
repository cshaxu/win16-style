#ifndef WIN2K_CONTROL_EDGE_H
#define WIN2K_CONTROL_EDGE_H
#include "palette.h"

/* Source-derived rectangular USER edge. BF_SOFT/FLAT/MONO select the original
 * layer colors; BF_ADJUST publishes the inset. Diagonal/partial sides are
 * outside this operation. The caller retains the DC and palette. */
BOOL win2k_control_edge(HDC,RECT *,UINT edge,UINT flags,const win2k_control_palette *);
BOOL win2k_control_fill(HDC,const RECT *,COLORREF);
#endif
