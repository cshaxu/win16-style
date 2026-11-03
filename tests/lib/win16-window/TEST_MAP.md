# win16-window test map

`window_link.c` whole-archives the window library, so every window source
object must resolve at link time. The focused native probes below exercise the
behavioural contracts that require a window or message pump.

| Source family | Native test adapter |
| --- | --- |
| `menu_*` | `window_link.c`; `tests/ntw16s-dll/win16_style_menu_*.c` |
| `frame_*` | `window_link.c`; `tests/ntw16s-dll/frames.c`; `win16_style_dc_failure.c` |
