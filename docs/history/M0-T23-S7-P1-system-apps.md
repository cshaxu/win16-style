# M0 T23 S7 P1: system application window inventory

Paint and Snipping Tool use captioned no-redirection composited windows on this
machine. Their observed `MSPaintApp` and `XamlWindow` classes now reuse the
existing composed non-client surface and its removal path. A separate shared
geometry correction intersects a maximized client area with the visible work
area. The previous behavior hid the surface when Paint's client extended beyond
that area. No client-control styling or original graphics were changed.

| Target | Evidence and boundary |
| --- | --- |
| Paint 11.2609.401.0 | The original package attached but showed a native title. Candidate screen capture showed a Win16 title, with one maximize/restore and surface removal observed. The final clean package loaded and its owned surface was removed successfully. |
| Snipping Tool 11.2608.41.0 | Candidate screen capture showed a Win16 title. One maximize/restore and removal were observed. The final package observer reported attachment and removal success; its outer PowerShell driver exited 1 without additional error, so the entire driver run is not a pass. |
| Settings | After the earlier shared host was closed to release the deployed DLL, a newly activated Settings frame loaded the published DLL and showed a visible Win16 title surface. Foreground activation failed and the screenshot was partly obscured; full visual and interaction verification remains pending. |
| Device Manager | MMC declares `highestAvailable`; the system elevation attempt returned cancellation 1223, and no target window was observed. This is not a pass. |

The short `run16s mspaint` and `run16s SnippingTool` commands both failed with
1920 because their Windows App Execution Alias paths could not be opened as
ordinary files. The verified full package executable paths work for the two
desktop applications. Generic alias handling remains a TODO; reparse-data
internals are not treated as a stable public contract.

The x64 maximize overflow regression failed before the geometry fix; the
corrected x86 and x64 incremental and clean-tree versions passed. The C17
MinGW GCC 16.1.0, Ninja and ccache 4.13.6 clean x86/x64 build, ABI, source,
resource and library-boundary gates passed. The actual four-file package passed
29 focused cases with zero skips; this is not the full matrix. Snapshot manifest
SHA-256: `584449eb5b333bb583635fd82edf3deefa4584ce19b6e5bbeac79475913483b3`.

The first publish was interrupted by a mapped DLL. Under the owner's standing
authorization, the exact ApplicationFrameHost process was identified and
closed normally; windows in that shared host may have closed. The second publish
copied and verified the same tested files to assets and O:/winnt:

| File | SHA-256 |
| --- | --- |
| run16s.exe | bb070bec3d193f59fe1e9c2701d0bba64966822062e628bf8fb1b3fac0abb486 |
| ntw16s32.dll | c2e3a5b9d254b880aedced6ce753ab5ec3f885695a6ac6b7bf77b445d5252716 |
| ntw16s64.dll | 54a73b1189db0860c2bcb9008aa5e98f0f287856d3071727504f3b666e1ac5d4 |
| demo.exe | 198736803c261e58d1d777cba9c1e9f9b521d215014086e05503062fb13c5387 |

S7 remains open for user acceptance. Codex remains skipped by prior owner request.
