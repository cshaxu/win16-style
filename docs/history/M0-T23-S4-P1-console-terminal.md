# M0 T23 S4 P1: Terminal and console candidate

S4 remains open. This delivery enables a bounded candidate for direct Windows Terminal,
Terminal-hosted `run16s cmd`, and `run16s conhost`; it does not claim complete WinUI support.
The existing user-owned QUEUE and XAML proposal edits are outside this delivery.

The launcher identifies a Terminal window through the calling pseudo-console or a new
window snapshot; only the selected window receives the existing GUI-thread attachment.
For an explicitly started system conhost, its child console window and host threads are
identified before attachment. Existing frame drawing is composited over nonredirected
Terminal windows without clearing WS_EX_NOREDIRECTIONBITMAP. Generic layered windows
remain excluded; constant-alpha ConsoleWindowClass windows preserve their attributes.

A self-created Terminal shared-process test observed the new window attached and the
pre-existing test window unattached. Real-screen capture showed the Win16 title and the
original tab row; a real mouse click opened the system menu and Escape closed it.
A separate same-package conhost test checked attachment, echo input/output and removal.
Dual-width frame tests and the x64 launcher selection tests passed. The clean package
passed 29 focused actual-package cases, no skips, including all four cmd nested bitness
combinations. First package run failed one cmd-nested case even though its child and
marker appeared in the eventual log; its cause remains unknown and the failure is kept
in s4-terminal-package1. This is not claimed as a fixed or fully explained failure.

Known limitations: Terminal's XAML title row still shows its native minimize/maximize/
close buttons in addition to the Win16 buttons. This task does not alter XAML controls.
All Terminal variants, transparency modes, rapid resize, visual flicker, shared-host
races, and package-version differences remain unverified. User acceptance remains
pending. The original goal and T23 remain open.

Same-snapshot clean x86/x64 Ninja/GCC/ccache package, actual-package results and publish
backups: build/t23-package/s4-terminal-clean3, s4-terminal-package2 and
s4-terminal-publish1. Published to assets root and O:/winnt with matching SHA-256:

| File | SHA-256 |
| --- | --- |
| run16s.exe | 938a98de4cd94481eb0ca36f8943424dac06ebdc58b235ad10ec558392d4d0d8 |
| ntw16s32.dll | 0da81bed45ecb1dc31e806071488c141cd65d7ac58394312e5d5c1baf05b857c |
| ntw16s64.dll | 977b789936e6f818e1ab4d5376f308964d25b950f90acbdc8591d96728ca6d81 |
| demo.exe | a23b5ac657a28bbcf217477aa5616359978e6bb290becb895b58621bdb3950aa |

Explorer PID 22072 was confirmed as the holder of the old deployed DLL, stopped under
the owner's standing authorization, and Explorer was restarted. Existing Explorer
folder windows may have closed. No other process was intentionally terminated.
