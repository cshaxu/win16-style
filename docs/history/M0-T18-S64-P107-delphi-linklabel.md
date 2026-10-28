# M0 T18 S64 P107: Delphi LinkLabel typography

A newly created exact `TLinkLabel` is a candidate for the existing native SysLink typography only when its owner forwards its own WM_PAINT to the registered native SysLink procedure, and the initialized native window provides a bounded ideal height, DLGC_STATIC and sufficient class extra bytes. The existing control adapter changes typography while native SysLink retains markup, hit testing, notification and state. No external code, original graphics or public export was added. This applies to a witnessed path, not every VCL link or custom renderer.

A genuine DelphiVCL4Python 1.0.7 sample at 144 DPI using the actual four-file final package changed from Segoe UI height -18 to `Win16 System 00000007` height 25 after attachment, in both widths; LM_GETITEM preserved ID `link` and enabled state. The old/new real samples each produced the same three OnLinkClick callbacks under the same synthetic mouse, Return and direct disabled-window messages; that comparison does not establish suppression of synthetic messages by a disabled window or physical input. The direct-message test is not evidence of a physical click. All actual target and launcher exits were zero and owned jobs emptied. An initial observer wrapped CallWindowProcW and therefore changed the caller identity; the valid final sample used no such wrapper. An initial candidate required DLGC_WANTTAB and failed binding; the real class reported DLGC_STATIC, and the revised protocol passed.

Project-authored x86/x64 models exercise absence of the native paint witness and a foreign caller (both rejected), successful link font conversion, native visited state, mouse/Return notifications, disabled link item suppression, original font restoration and reentrant removal from NM_CLICK. Both old published DLLs fail the new positive attachment assertion. This fixture models native link forwarding; it is not a substitute for additional VCL versions, actual physical keyboard/mouse, Unicode and complex markup, live layout changes, or general resource leak proof. A first fixture attempt did not show its private desktop root; a subsequent fixture had no source font and a later state read used the wrong mask. These were corrected before the passing runs.

A clean same-snapshot build with C17/MinGW GCC, Ninja and ccache passed source, byte-exact graphics/provenance, governance and dual ABI gates. Snapshot JSON SHA-256 `408383f8325cb22003c0632fa2ba23030c0eeb923fdcf320fc9dafa392f4dc21`; x86/x64 builds took 23.32/27.88 seconds. The actual four-file package passed 16 focused cases in 6.68 seconds, zero skips, including mandatory launcher/propagation, link models and adjacent edit/NumberBox regressions. It is focused coverage, not the complete integration matrix. The same tested files were published to `assets/` and `O:/winnt/system32`; hashes matched all eight destinations and installed x86/x64 link models passed from both.

| File | SHA-256 |
| --- | --- |
| run16s.exe | `bd0e2008105cba06e6a20eebc2ab38cf8f4d887a7da8c8e58cf18de1f8d31c9e` |
| ntw16s32.dll | `14ea8aecc7c3f2d63e5f60fd8daa18497ed6248e83c819a5247d894e28ae60cb` |
| ntw16s64.dll | `2aafae0a01bd96d8462e8a526006056a6a60766a24e6298ef5dedfb4b29e5a69` |
| demo.exe | `0882879ac467368a3458f6d1a14cb78f24217294ec80b148b972f85a85fd0f92` |

The TMediaPlayer button and other visible family gaps remain open. M0/T18/S64 remain open.
