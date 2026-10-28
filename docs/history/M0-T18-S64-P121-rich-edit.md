# M0 T18 S64 P121: witnessed Delphi Rich Edit chrome

The real x86 DoubleKiller Help view used an application-owned `TRichEditURL` that forwarded its own WM_PAINT to the registered ANSI RichEdit20A procedure. P120 left it unattached, with modern client edge and scrollbar. This change adds an ANSI CallWindowProc witness, restricted to the same new control, UI thread, class/parent owner and a fresh paint witness. It reuses existing frame and scrollbar rendering, without changing rich text, character formatting, fonts or application-owned content. Role or parent changes retire the owned attachment. No new product file or public export was added.

The real isolated 96 DPI sample, launched with the actual same-batch package, preserved all client pixels in a symmetric redraw comparison: 1898 outer-edge pixels and 4573 other nonclient pixels changed. Character format, selection, length, event mask and scroll state stayed equal; selection 2..8 was restored, the down arrow moved position 0 to 16 and released capture, and line scrolling matched native 0 -> 3 -> 0. The target and launcher exited zero and the owned job emptied. This is one x86 application, not a blanket claim for other Delphi versions or RichEdit classes.

Self-owned x86/x64 RichEdit20A models passed wrong owner, wrong caller, absent and stale paint witness, formatting and selection, application link notifications compared with native (2 each, no URL opened), exception unwind, reparent and client-edge removal. Pixel comparisons changed 3096 nonclient pixels with zero client changes. A 130000-character, 10001-line test reached the observed native scroll maximum of 65535, dragged to 65496 and released capture in both widths. This does not establish behavior for larger reported ranges or all RichEdit versions. For efficient test execution, only the isolated test thread calls ImmDisableIME(0); earlier long-test cleanup waited in system input components without this test isolation and its root cause remains unproven.

P120 old-package dual-width negatives failed at the expected attachment assertion. Removing fresh-paint witness reset deliberately caused the dual-width stale-witness negative to fail. The historical startup-hook capacity test expected 48 although the implementation already allowed 56; its independent test now confirms 56 slots, rejection of the 57th and rollback, on both architectures.

A clean C17 MinGW GCC/Ninja/ccache x86/x64 build passed source, byte-exact resource and 31-export ABI checks. The same four-file actual package passed 23 focused cases in 22.17 seconds without skips, including the core nested-cmd, VB, menu, scrollbar and hook capacity cases. It was published to assets and O:/winnt/system32; all eight hashes matched and both locations passed x86/x64 installed RichEdit models. Other rich edit classes, additional runtime versions/DPI, physical input, full IME semantics and the full T18 matrix are not covered. M0/T18/S64 remain open.

| File | SHA-256 |
| --- | --- |
| demo.exe | `1985e9460da2b1c6b4020fe3fce8f8bdb6ce1a6c0e75793a869c579bbc467830` |
| ntw16s32.dll | `909be99f8fd45d748ffaee1c59b2e8316b5258482e239d96ea69086e7c172d05` |
| ntw16s64.dll | `aa797e88de8a7c92ae9fd08555fd59ad321d24dab0e1acebf0b2916870514280` |
| run16s.exe | `e4b31fbc41998a4880f2191255b2aeacee95534bd096bedc8730b1881948df07` |
