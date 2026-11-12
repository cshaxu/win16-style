# M0 T26 S1: shared classic print contract

The ANSI and Unicode PrintDlg parameter-copy and result-restoration mechanism
now lives in `lib-style/common/classic_print_contract`, part of
`style-dialog-common`. Its caller supplies a supported-flag mask and temporary
hook. Win16's original allowed mask and no-op hook remain at the product call
site; unsupported contracts still take the native path. The old Win16-only
implementation was removed. The public helper does not depend on a renderer,
printer driver, product header or specific style.

The standalone test covers both character widths, caller-defined flag policy,
invalid input, hook replacement, result data and restoring an originally set
or unset hook flag. The existing dialog-link test also calls the common API.
These tests do not operate a printer or establish interactive dialog behavior;
other style consumers and full PrintDlg/PrintDlgEx parameter coverage remain
open.

The clean x86/x64 MinGW GCC / Ninja / ccache build passed from snapshot
`1d7f6b0be47445922b98626837ced3e68b4d4f413c6632f2956e547d4b7c1eb9`.
Eight files were published to assets and `O:\winnt` before testing and their
hashes matched the candidate. The print contract, dialog-link and remaining
selected component tests passed on both architectures, as did four published
launcher menu probes. The x64 `core-style-uwp-dispatch` failed with exit 6:
its hook operations returned error 126. The x86 equivalent passed. A retry
outside the sandbox and the unchanged incremental x64 test had the same
failure. An independent x64 probe loaded the fixture DLL and found its export,
but `SetWindowsHookExW` returned 126; the same error occurred with a system
`user32` callback both inside and outside the sandbox. No security settings or
private desktop were changed. This is a failed regression, not a passing or
skipped result; the cause remains unconfirmed and delivery is not complete.

On a later retry both the x86 and x64 incremental dispatch tests returned exit
6 and hook error 126. A read-only desktop query found the test thread on
`Default` and the input desktop named `Screen-saver`; this observation does not
establish the cause. An input-based wake attempt was denied, and the requested
unsandboxed retry was rejected by automatic approval as computer use. No
further input or desktop switching was attempted. Both architectures therefore
remain unverified in this later environment.

Further same-thread comparison: `WH_CALLWNDPROC` with an executable callback
worked both with a null module and with the executable module on x86 and x64.
On both architectures a separate native probe successfully loaded the actual
test fixture DLL with the production loader flags and found `fixture_hook`, but
`SetWindowsHookExW` with that DLL module returned 126. This narrows the observed
difference to the DLL-backed case; it does not establish why Windows rejected
it or prove a product regression. Probe source and executables were removed.

An x64 path comparison loaded the same fixture from the workspace and from a
temporary local directory. Both loads and export lookups succeeded, while both
DLL-backed hook installations returned 126. This excludes neither all path nor
environment causes; it shows that this particular directory change did not
resolve the failure. The temporary copy was removed.
