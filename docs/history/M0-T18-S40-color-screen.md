# M0 T18 S40 — Color-combo screen comparison

S39's expanded-color-combo screen captures sometimes differed outside the
button by 1718 pixels. The same change also occurred after a plain redraw with
no removal. This investigation used self-owned, isolated DelphiVCL4Python
1.0.7 windows and native probes; it did not modify existing user processes,
the product, system settings, or original assets.

Old S38 and new S39 packages both reproduced the phenomenon at 144 DPI on
x86 and x64. In the captured item rectangle (3,3)-(101,19), affected samples
changed 1396 pixels. A parent observer recorded the application's last
WM_DRAWITEM DC before the screen sample and its DC after the next redraw.
Across 32 paired samples, both app DC images were identical and matched the
post-redraw screen pixels. In seven affected samples, the pre-redraw screen
sample differed from both application DCs by the same 1396 pixels. The DC was
a memory DC rather than the target window DC; its clip was the item rectangle.
This places the observed discrepancy after that sampled app drawing, but does
not distinguish delayed blit, composition, occlusion or screen-DC sampling.
It does not establish whether S39 changes the probability of the discrepancy.

Without a redraw, five short 100 ms x86 samples for each package did not
change. In a separate small x86 experiment, a successful DwmFlush before
capture was followed by no item discrepancy in five old and five new samples.
This is not proof that DWM is the cause, a general fix, or a reason to add a
production wait. No global settings were changed. All owned instances and
temporary observers closed. Raw per-sample data and boundaries are summarized
in the [diagnostic evidence](../etc/evidence/delphi-s40-screen-diagnostic.json).
Other DPI, runtime versions, physical input and direct inspection outside the
item rectangle remain unverified. The mechanism and any difference in rates
stay open rather than being represented as resolved.

No product source or graphics changed. The clean GCC 16.1.0/Ninja/ccache
snapshot SHA-256 was
`dc1457a10e7adc47fa18bd1f5b1335c95130951a64a155afcbfa86d4616d4406`.
The same-batch package passed 12 focused cases without skips in 10.94 seconds;
other package cases and physical input were not run. Independent rebuild hashes
changed, so all four tested files were published to assets and system32; both
system32 x86/x64 installed-entry probes passed. Both locations matched:

| File | SHA-256 |
| --- | --- |
| demo.exe | `81c10e36df2440399dab96bbd500a853c9c086a6ed60535c38c2aba0332bd59a` |
| ntw16s32.dll | `a740e9f87557fe0d26ae41a382556f8f9249916f377e70f2823672d390d53ffb` |
| ntw16s64.dll | `bd8287ff4641ba17d0221eda388733cd2e91e344c7cb5c6b841067b13737e25b` |
| run16s.exe | `a4a10c9780f26d1b01462e5aeec8ca6e7650f0e0ff6a7a03bdd94fbc6d566ff6` |

P1 commit and actual remote verification are pending. The remaining T18
control inventory is not complete; the screen mechanism is unresolved.
