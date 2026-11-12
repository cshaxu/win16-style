# win16-control test map

| Source family | Native test adapter |
| --- | --- |
| control protocol, state, install/remove/lifetime | `control_link.c`; `control_scope.c` |
| basic, choice, list, combo, scroll, range and special painters | `control_link.c`; `list_paint.c` (native tab-stop forwarding); focused `tests/ntw16s-dll/*` probes |
| VB/VCL identity and text | `control_link.c`; focused `tests/ntw16s-dll/vb_*.c` and `vcl_*.c` probes |
| native presentation | `tests/lib-style/common/present.c`; `tests/ntw16s-dll/win16_style_control_present.c` |
