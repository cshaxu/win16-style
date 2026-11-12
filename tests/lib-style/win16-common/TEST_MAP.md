# win16-common test map

Each source family has one focused native adapter. `common_link.c` exercises
the common palette, font and menu-layout contract; the focused DLL probes
retain behaviour-specific coverage. Window-library linkage is checked by the
separate `tests/lib-style/win16-window/window_link.c` adapter.

| Source family | Native test adapter |
| --- | --- |
| `layout`, `font`, `palette` | `common_link.c`; `tests/ntw16s-dll/font_layout.c` |
