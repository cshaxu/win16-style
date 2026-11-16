# win16-common test map

Each source family has one focused native adapter. `common_link.c` exercises
the common palette, font and menu-layout contract; the focused DLL probes
retain behaviour-specific coverage. Window-library linkage is checked by the
separate `tests/lib-win32-style/appearance/win3x/window/window_link.c` adapter.

| Source family | Native test adapter |
| --- | --- |
| `layout` | `layout.c` (geometry and hit testing); `tests/lib-win32-style/appearance/win3x/window/modern_frame_layout.c` |
| `font` | `common_link.c`; `tests/ntw16s-dll/font_layout.c` |
| `palette` | `palette.c` (defaults, current contrast mode and dialog DC); `common_link.c` |
