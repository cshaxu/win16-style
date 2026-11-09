# M0 T23 S6 P1: Chrome candidate

The published launcher previously started an owned Chrome window, but its GPU
process failed while acquiring a memory-backed menu font and the browser exited
with `0x80000003`. The candidate uses the stock System font when that menu
font cannot be obtained, and uses the existing non-client surface for
captioned `Chrome_WidgetWin_1` windows with `WS_EX_NOREDIRECTIONBITMAP`.
The normal font path and original graphics are unchanged.

An owned Chrome with a private profile loaded the candidate and stayed alive
for four seconds; a separate incremental candidate test observed the owned
surface visible and maximize/restore states 3/1. The final tested package
removed the surface, cleared attachment, restored the client top and retained
the original composition flag in an owned Chrome instance. `PrintWindow` could not
capture the independent surface, and the input desktop was unavailable for
physical-screen verification. These observations do not establish correct
pixels, mouse interactions, restoration or all Chromium versions. User
acceptance remains pending. Codex Desktop was skipped at the user's request.

The same-snapshot C17 MinGW GCC 16.1.0, Ninja and ccache 4.13.6 x86/x64 clean
build and ABI, source, resource and library boundary checks passed. Snapshot
manifest SHA-256: `1b8dd446473e6c6891aca2b102dc6d4c769f73f980a8cbf3af633828b4051958`.
The same four-file package passed 29 focused cases with no skips, serially.
Two earlier package attempts failed private desktop release checks; their
cause remains unknown and their [output](M0-T23-S6-package-failures.md) is
preserved. Initial focused CTest invocations failed before executing tests due
to missing capture-directory setup; the corrected x86 and x64 runs passed four
tests each. Physical input and full matrix were not tested.

An earlier interim package was published before review identified a missing
Chrome surface removal path. The final package fixes and tests that path and
replaces the interim four files at both destinations.

The tested four files were published to assets and O:/winnt and matched:

| File | SHA-256 |
| --- | --- |
| run16s.exe | c27e41925b41e00432dc26869f8c73f7ce98e3e1406499cfbef8576900e3606e |
| ntw16s32.dll | e198ed42595533bce9879b951d7b75376161bb31328f7fa7eb6f246f88ae8ef6 |
| ntw16s64.dll | 2eabbc5db5629960d070a64e29afc9a1fccc8245b8fd12df3b186762456c8338 |
| demo.exe | 8044dc552aa8d06c4cc954ccfa63e2377edd2f9a8019d5ee4dc84b1beed64e56 |
