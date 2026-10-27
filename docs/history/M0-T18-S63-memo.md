# M0 T18 S63 — ordinary VCL TMemo

The preceding S62 observer checked immediately while its own temporary IAT
observer still occupied the runtime CallWindowProcW slot. The hook was restored
and a subsequent paint established native forwarding; that earlier timing was
not sufficient evidence of a second product defect. Diagnostic logging in a
scratch-only DLL established the sequence: candidate creation, runtime native
paint, protocol binding, and successful install. The released product does
not include the logger. The S62 baseline did not recognize TMemo. On x86 the
actual unlimited EM_GETLIMITTEXT bit pattern 0xffffffff also failed the former
signed-positive check; x64 returned 4294967295. The limited fix recognizes an
exact runtime-owned multiline TMemo after its native EDIT paint witness, then
uses the existing edit protocol and renderer. The limit is interpreted as the
unsigned documented value. It does not infer support from the class name alone.

Actual DelphiVCL4Python 1.0.7 samples at 144 DPI, x86 and x64, attached under
the installed assets DLLs. The owned sample accepted two lines, returned line
count 2 and selection (1,3), preserved text, restored original text and
closed; the observer's import was restored. A self-contained C model covered
fake-paint rejection, selection, readonly, multiline scroll, reentrant removal
and cleanup at both widths. The same model against the previously published
S62 package failed at positive attachment in both widths, exit 1. Real physical
input, IME, long text, other versions and DPI remain unverified. Native text,
input, callback, scroll state and data remain owned by the application/system.
No original graphics or ABI changed.

Same-snapshot GCC/Ninja/ccache clean x86/x64 build and PE/ABI gates passed;
snapshot manifest SHA-256
`164a8668a31e0eed8a9235e1ba8c24016560aae3fc98bcc59c2d2cbc17dd6169`.
Fourteen focused actual-package cases passed in 6.41 seconds at four jobs,
zero skips; other cases were not run. Both installed launcher paths passed
both bit-width models. Four tested files were deployed to assets root and
O:/winnt/system32 with identical hashes:

| File | SHA-256 |
| --- | --- |
| demo.exe | `f308d874ec6081bf5b6cd37d08ba2a4d2439b5ce0870a3a1beed0dcbc44e03f1` |
| ntw16s32.dll | `8a6fbd7a4ec3087eff7d4352c24ec9adb9f870fb9467361bf217e4350afb4a3f` |
| ntw16s64.dll | `3094b24dbf4497735eec81728f65943a84112879062e419cadde2a8043512570` |
| run16s.exe | `b595dee056a35da5e7236b5112ca3a34b6e04034209760b0c0326e412f5ae164` |

[Interim 74-family accounting](../etc/evidence/delphi-s63-count.json) records
26 families with at least one bounded implementation, 2 native-preserved,
and 46 without evidenced implementation. These are family classifications,
not a count of remaining coding tasks; bounded families may still contain
unsupported variants. T18 and M0 remain open.

Production diff: 4 additions/4 deletions, net 0; test/build wiring: 24
additions/5 deletions, net 19. Searches used `rg -n -C 2 'TEdit|TMemo'
src/ntw16s-dll` and `rg -n -C 4 'edit_candidate|verified_role'
src/ntw16s-dll/attachment.c`; the same verified edit path owns the renderer,
without replacing the old edit path. Expanded protocol uses actual native
paint witness and preserves native edit text and selection semantics.
