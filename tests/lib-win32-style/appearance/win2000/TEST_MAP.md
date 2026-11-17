# Win2000 appearance test ownership

| Production source | Contract validation |
| --- | --- |
| common/palette.c, common/edge.c | common/edge.c: source/original-image edge color samples, standard scheme, default/pressed differences, mono window slot and caller DC restoration |
| control/button_paint.c | control/button_paint.c: actual product ABI/WM_PRINTCLIENT pixels and input/resource contracts; v5/v6 manifests, pressed text offset, multiline, disabled emboss, focus/UI-cue messages, borrowed bitmap, modern image-list native preservation and DC state |
| control/control.c | control/button_paint.c and tests/ntw2ks-dll/control_theme.c: actual install/remove/retire, reentry and shared theme ownership; integration tests, not independent product-free library tests |

Tests retain source component paths. An actual consumer test can cover multiple
source files; no duplicate constant-only tests are added to manufacture counts.
