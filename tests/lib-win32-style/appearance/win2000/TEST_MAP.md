# Win2000 appearance test ownership

| Production source | Contract validation |
| --- | --- |
| common/palette.c, common/paint.c | common/paint.c: source/original-image edge color samples, standard scheme, default/pressed differences, mono window slot, four genuine 13px glyph hashes, wrong-dark negative, mixed/disabled/flat rules, original down-arrow shape/non-soft scroll edge/pressed/disabled and caller DC/resource restoration |
| control/button_paint.c | control/button_paint.c: actual product ABI/WM_PRINTCLIENT pixels and input/resource contracts; v5/v6 manifests, 84 choice/pushlike state renders per mode, native auto3/radio groups, odd-offset dither consistency, pressed text offset, multiline, disabled emboss, focus/UI-cue messages, borrowed bitmap, modern image-list native preservation and DC state |
| control/control.c | control/button_paint.c and tests/ntw2ks-dll/control_theme.c: actual install/remove/retire, reentry and shared theme ownership; integration tests, not independent product-free library tests |

Tests retain source component paths. An actual consumer test can cover multiple
source files; no duplicate constant-only tests are added to manufacture counts.

| control/background.c | control/static_paint.c and button_paint.c: actual product parent query, custom brush and reentrant remove/destroy; shared background and font order |
| control/static_paint.c | control/static_paint.c: 14 drawn STATIC roles ×2 enable states, alignment/default colors/interior preservation, native bitmap/icon handle/DC preservation, group etched caption/interior,32 ANSI resource cycles; hooked modal/modeless initialized descendants |

| control/edit_paint.c | control/edit_paint.c: actual DLL/source-image NC color samples; native EDIT7 roles/text/undo/selection/font/password/custom brush, RichEdit20A/W/50W and HotKey native clients/formats/values, parent retirement/reparent32 ANSI resources and initialized dialog children; common optional edit protocol test at application/control/edit.c |

| control/list_combo.c | control/list_combo.c: five list and three combo modes, original selection and edge colors, native tabs and owner draw, editable child clipping, owned popup paint and detach; v5/v6, DPI awareness and normal startup |

| control/scroll_layout.c, control/scroll.c | scroll_layout.c verifies the shared layout geometry. scroll.c verifies attached standalone and nonclient bars, list wheel, drag notifications, borrowed GDI, custom groove and resize grip. |

| control/tab_paint.c, control/header_paint.c | control/tab_header.c verifies original edge colors and selected corners, tab image/font borrowing, native selection and disable, header press/click/reorder, application custom draw and detach. The same probe runs with both common-control manifests, DPI awareness and regular startup. |

| control/report_paint.c | control/report_tree.c verifies four native ListView modes, inactive selection, column sizing through Header, tree expand/collapse glyphs and notifications, application custom draw, borrowed images, native owner-data fallback and detach. Both common-control manifests, DPI awareness and standard startup. |

| control/progress_paint.c | control/progress_paint.c verifies original block rounding, period, terminal clearance, both axes, signed-range math and borrowed DC. control/progress.c verifies v5 edge-only and v6 client rendering, native values, caller colors, disabled appearance, removal and standard startup. |

| control/trackbar_paint.c | control/trackbar_paint.c verifies the original groove, square and four pointed directions, disabled dither, keyboard state and notifications, child custom draw, DPI awareness and detach. |

| control/updown_paint.c | control/updown_paint.c verifies borrowed native image roles, horizontal and vertical odd seams, disabled and pressed pixels, notification veto, edit buddy identity/data, DPI awareness and detach. |

| control/toolbar_paint.c, control/rebar_paint.c, control/status_paint.c | control/bars.c verifies regular and flat toolbar edges, separators, image list and application drawing, band gripper and child identity, status pane edges, native simple text and owner draw, custom background and size hit test, both manifests, DPI awareness, removal and standard startup. |
