# M0 T26 S1: shared popup arrow selection

The shared `style_menu_bar_arrow` now owns the popup arrow decision used by
both `win16-window/menu_bar.c` and `win2k-window/frame.c`. Win16 opts into
its existing multi-column rectangle and nested-popup rules; Win2000 keeps its
root-popup restriction and scans highlighted items before choosing a bar
transition. The old Win16 implementation and duplicate Win2000 scan were
removed. The two tracking loops remain separate: they use different native
hooks, focus and system-menu behavior, and have not been proven interchangeable.

A clean x86/x64 MinGW GCC 16.1, Ninja and ccache build passed; source snapshot
SHA-256 `08374fa8bd6306c65681452c33011b62c982b88ad81e773905e06d5dfd6babad`.
The eight files were published to assets and O:\winnt before 18 focused checks.
Both architectures passed the shared input and Win16 standalone fixtures, the
published Win16 mnemonic menu and Win2000 mnemonic, F10, selection, hover,
arrow and nested-popup scenarios. These are message-driven tests, not physical
input or full visual acceptance. Both destinations matched the candidate:

| File | SHA-256 |
| --- | --- |
| demo.exe | `3e4c894fa6d35b9b05b941d6dc354a34dd34594155ff9391229cb219cfe8c8cb` |
| ntw16s32.dll | `0e9ae425a96821b1f9528826f66977f3a2c4f032e6ba1737eb769275fceabde6` |
| ntw16s64.dll | `21cfe9db25e10aa24716c917b5c46390ae9bbb175aedfc2477068b5bdea2928c` |
| ntw2ks32.dll | `4758d6cebe45d7d0100c4db0275bd524919c63367ae65c306ca0db128da2a449` |
| ntw2ks64.dll | `f81c5278b0561e5a4eec704faeced7327ad2528551669d3167f67c464b62f17d` |
| ntwshk32.dll | `808649f566acba96b4b72a13d038fd5819bd55d2da87e4decf1a17d97e494d62` |
| ntwshk64.dll | `278487652403980b15e6ec944f6beee96691f8966c1bcb7dd7d860ca4ce2c548` |
| run16s.exe | `2251b2042e4a53c5b4fdd0c498ab6d2b102bc124a8f6c5c6d4699fa42c74d1de` |

The wider cross-style extraction and audit remain open; this result does not
establish Win2000 dialog or control coverage, WinUI coverage, or DWM restoration.
