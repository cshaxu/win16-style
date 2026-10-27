# M0 T18 S47 — icon-bearing last status pane

On self-owned x86/x64 ComCtl32 6.16 status controls and DelphiVCL4Python
1.0.7, native DrawIconEx precedes native DrawThemeText for the last pane with
a sizing grip. The observed icon extends outside the pane vertically, so
framing at the later text call could overwrite icon pixels. The product now
frames before a native icon call only when the current status paint scope,
registered system caller, same-thread window, final pane geometry, ordinary
pane type, icon handle and unchanged mutation epoch all match. The native icon
call retains its arguments and entry last-error state; unsupported/owner-drawn
cases remain native. The added hook fits the existing 40-slot transaction; no
public ABI or original resource changed.

Self-owned x86/x64 96-DPI module tests cover normal/empty panes, icon and
owner-drawn fallback, 16/32/48 input icons, an alpha icon, and reentrant
removal during icon query. The system drew the tested input sizes as 32x32;
this is not proof of all icon sizes or DPI. Whole-canvas comparisons found no
changes away from the two pane edges. A separate alpha-icon restricted-clip
probe changed 79 pixels within clip and zero outside. The prior released x64
DLL failed the new icon positive assertion. Tests destroy the owned alpha icon.

The final package on a self-owned DelphiVCL4Python 1.0.7 x86/x64 sample at
144 DPI showed screen changes of 440 first-pane and 732 last-pane edge pixels,
and WM_PRINTCLIENT changes of 440 and 731, all within pane edges. The one
pixel screen/DC count difference is recorded, not explained; it does not
establish the unrelated S40 screen issue. Text and owner-draw data were
restored. [Evidence](../etc/evidence/delphi-s47-icon-pane.json) is limited to
this runtime and sample; physical input and full T18 integration remain open.

Clean GCC/Ninja/ccache dual-width builds and ABI checks passed. Actual package
first-run x86 date module failed because its process GDI count fell from 27 to
25 across the exception test. A separate same-package run had 27 to 27; a
serial 26-case package retry passed in 25.21 seconds with zero skips. This
retry does not explain the first failure. The unresolved transient count is
recorded in TODO. Both publication locations match the same-batch hashes below,
and installed x86/x64 pane probes passed. Source and original-resource gates
passed.

| File | SHA-256 |
| --- | --- |
| demo.exe | `9dd0c39e7f0712ab8d740d5d20dc934bcffffe76125a90c45edf679201f576d6` |
| ntw16s32.dll | `fb562a0989a060661b4f70623096b6b1139f0f9889ed02181b7997b9c0aeb46b` |
| ntw16s64.dll | `808dddd4dbc3090f512e005b34ceb3be2838e2973af2fc8c08c2e521af40017f` |
| run16s.exe | `42714712d4d3b4e70b723bbf6aac2fbc6c96aa25f1e131bb4ef6ddd82246d1c9` |

T18 and M0 remain open; owner-drawn last-pane styling is still native.
