# M0 T18 S64 P111: runtime-owned Delphi push button

DelphiVCL4Python 1.0.7's `TNotebook` and `TPage` are containers with no independently styled chrome. Its ordinary `TButton` was not attached because its class belongs to the Delphi runtime module rather than the process EXE. In both real widths its own WM_PAINT forwarded to the registered native BUTTON procedure; its native button protocol and ideal size were valid. The scoped change accepts a same-runtime child only after this paint witness and protocol validation, and only for plain or default text push buttons. Bitmap, icon, owner-draw, RTL, foreign-owner and unwitnessed cases retain native treatment. No public export or product file was added.

At 144 DPI the real button is 120 x 28 physical pixels, while its existing Win16 System font measures 91 x 25. The earlier candidate's generic padding clipped the label severely. This delivery gives only this witnessed button a smaller vertical inset and, if necessary, fits the complete bitmap text image proportionally into its face. A captured real 32-bit label was visually inspected. The application's font and layout remain unchanged; fitting affects only this new renderer path. In each real width, actual package testing recorded attachment, Win16 System font height 25, one posted BM_CLICK callback, successful notebook page changes and target/launcher exit 0 with the owned job empty. These are synthetic input observations, not physical input.

The project-authored double-width fixture covers missing witness, foreign caller, wrong parent owner, bitmap exclusion, native protocol, click, space key, reparent, owner-draw transition, font restoration and cleanup. The P110 package failed the new attachment assertion in each width. Existing group/radio, spin and SpinEdit focused models also passed. The first sandboxed package test skipped window cases and is not counted as a pass; an authorized native rerun passed all 20 selected cases in 7.25 seconds, with zero skips. Full integration, other DPI values, VCL versions, long or localized labels, fault injection in the new text fitting code, physical input and all buttons' possible visual states remain unverified. Existing title, menu and original resource source files were not changed. Original five resource headers and provenance passed byte-exact verification.

C17 MinGW GCC 16.1.0, Ninja and ccache 4.13.6 clean builds passed x86/x64 ABI checks. Clean snapshot SHA-256: `18ab3369b8a0d4a93d37f9f2c4194b92bb9177e533c2fc6648e6cd97ff80c871`. The same four-file package was published to both assets root and O:/winnt/system32; the native push model passed both widths from both destinations. M0, T18 and S64 remain open; nonvisual and application-self-drawn controls remain outside scope.

| File | SHA-256 |
| --- | --- |
| run16s.exe | `eb119b16eb1564af19326c20710e793a5b0d1b59a78616bd41bfab155c46fbf8` |
| ntw16s32.dll | `b54cf45c4ca1a95d8454ea23e5daa2400c0b154bdea3adb50a19d8d1b649e367` |
| ntw16s64.dll | `e4b48ea5c159928812abad5be70f98a1cc8b7739ce0034a379435029c3dc2cde` |
| demo.exe | `88e544cc2742c97823485446a33c9890f2a684fa0f32855cae5eff52aba976c7` |
