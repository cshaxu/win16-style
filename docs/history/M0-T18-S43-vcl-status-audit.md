# M0 T18 S43 — real VCL status-bar audit

Self-owned DelphiVCL4Python 1.0.7 x86 and x64 instances at 144 DPI created
runtime-owned `TStatusBar` windows. Both were unattached by the current DLL.
An import observer in the runtime module saw WM_PAINT and WM_NCPAINT forwarded
to the registered native status-bar procedure; observer slots were restored.
Using the public native messages, two ordinary panes read back `Ready` and
`Count: 42` with rectangles (0,2)-(200,24) and (202,2)-(584,24). Simple mode
could be enabled and disabled. An owner-draw pane sent its original
`0x12345678` data to the parent twice; the temporary parent subclass and pane
contents were restored. These are two instances of one runtime, not proof for
all Delphi versions or actual application-owned painter implementations.

A scratch-only frame drawn over the two public pane rectangles changed 1244
pixels on each width, all on pane edges. The native redraw was restored; it
is not a product implementation or evidence that painting after application
content is always safe. The v6 comctl32 DrawEdge import observation installed
but saw no calls in this experiment. The DrawThemeBackground import observer
did not install; a dynamic or other theme path is not excluded. The existing
native status renderer's pre-content boundary therefore cannot yet be claimed
to work for this superclass. Further implementation needs a real native-call
witness, the correct drawing entry and unchanged app ownership, clipping,
icons, mutation, reentry, lifetime and simple/owner-draw boundaries.
[Structured evidence](../etc/evidence/delphi-s43-status-audit.json) records the
limited observations. The first x86 attempt failed because the scratch probe
DLL used decorated exports; correcting the probe export allowed the subsequent
run. This was a test setup failure, not a product result.

No product source or original resource changed. Independent clean GCC/Ninja/
ccache builds and both ABI checks passed; resource identity and provenance
remain byte-exact. The same-batch four-file package passed 18 focused cases
with no skips in 6.56 seconds, including both native status versions and DPIs,
date-picker tests and required launch/propagation checks. Full integration,
physical input, other runtimes and variants remain untested. Since the clean
build changed hashes, the tested four files were published to both locations;
installed x86/x64 v6 status probes at 144 DPI passed and all hashes matched:

| File | SHA-256 |
| --- | --- |
| demo.exe | `8416636c7c67f1b9b76e853f1068eafcebb1186c450341a6547651ab43efb8a7` |
| ntw16s32.dll | `f3764e6cd51a7997c9b36fa50ae369096de6207de682275375a01aee29f3b4a9` |
| ntw16s64.dll | `856a228320f79233ec1c9a8a28c78c1906a4e54ca9e8c07fa506ed680ace8cd6` |
| run16s.exe | `5927d76ba128a691d53f324f0caca2f4b629261d7467100ca532a7c05c2abae2` |

T18 and M0 remain open. The existing S40 screen discrepancy and S41 first
probe exit remain unresolved; no conclusion is drawn from retry alone.
