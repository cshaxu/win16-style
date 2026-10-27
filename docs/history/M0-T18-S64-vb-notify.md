# M0 T18 S64 P4 — real VB notification observation

Two previously audited original x86 VB5/VB6 intrinsic samples were run with a
scratch-only diagnostic build of the existing style DLL. It records WM_COMMAND
immediately before and after the existing DefSubclassProc call for VB runtime
child windows. It does not install a global or cross-process hook and it is
not the published DLL. Diagnostic source and DLL SHA-256, PIDs, original
sample identity and per-class/code counts are in [VB evidence](../etc/evidence/vb-current-s64.json).

Each sample at 96 DPI produced 27 matched before/after observations including
four option-button and eight checkbox notifications, as well as textbox and
combo notifications. The sequence includes application setup; it is not a
count exclusively caused by the observer's inputs. The existing sample tests
still passed glyph checks, control state/selection and cleanup. This shows the
instrumented subclass invoked and returned from the next window procedure for
those messages; it does not independently establish that every application
handler executed, or cover physical input, scroll/list notifications or IME.

The VB6 diagnostic screenshot matched the preceding released-DLL screenshot.
The first VB5 diagnostic screenshot had identical dimensions/BMP headers but
12,676 different pixel-payload bytes. One further release and diagnostic run
both matched the original release screenshot; the diagnostic repeat again
recorded 27 paired notifications. The first discrepancy was not reproduced
and its cause is unknown. This is not a proof that instrumentation cannot
change rendering or timing.

No product source, test source or release binary changed in P4. The S64 P2
same-batch clean x86/x64 build, 14 focused tests and installed hashes remain
the baseline. P3 commit 77c684f and its three remote files were verified.
The cross-process hook error 126 and S64's separate VCL work remain open.
