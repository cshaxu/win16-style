# win16-control test map

| Source family | Native test adapter |
| --- | --- |
| control protocol, state, install/remove/lifetime | `control_link.c`; `control_scope.c` |
| basic, choice, list, combo, scroll, range and special painters | `control_link.c`; focused `tests/ntw16s-dll/*` probes |
| VB/VCL identity and text | `control_link.c`; focused `tests/ntw16s-dll/vb_*.c` and `vcl_*.c` probes |
| print and native presentation | `control_print.c` |
