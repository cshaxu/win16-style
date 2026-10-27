# M0 T18 S62 — VCL radio and memo

Real DelphiVCL4Python 1.0.7 runtime-owned x86 and x64 ordinary TRadioButton
and TMemo samples at 144 DPI were observed on owned processes. Both forwarded
WM_PAINT to the corresponding USER BUTTON or EDIT procedure from their own
runtime module. The original package did not attach these samples. The bounded
change admits ordinary TRadioButton only when the parent has the same class
owner and the runtime actually forwards paint to native BUTTON; it reuses the
existing verified radio protocol and renderer. Text, selection and callbacks
remain native. The installed assets DLLs attached both actual samples; this is
not proof of every VCL version, picture/owner-draw variant or physical input.

A self-contained native module model verifies fake-paint rejection, actual
native forwarding, selection, keyboard, font restoration, reparenting,
owner-draw transition and cleanup for both bit widths. The original group and
edit models are also tested. Real TMemo stayed unattached: it reports an
EM_GETLIMITTEXT value represented as -1, but changing the signed comparison
alone did not establish attachment. That experimental code was removed; the
remaining failure gate is not yet established. No claim is made that the
unsigned interpretation alone is the root cause. More interaction variants,
DPI, physical input and task-wide integration remain outstanding.

The same-snapshot clean GCC/Ninja/ccache x86/x64 build passed PE and ABI gates;
snapshot manifest SHA-256 is
`455e6ba7b1f61dfba89b19b3e017680b976f5265d2d2680afc9141e911609575`.
Fourteen focused actual-package cases passed in 15.32 seconds at four jobs,
zero skips. Both installed launchers passed both radio models. Real sample
observer confirmed attachment and native paint forwarding under both installed
DLLs, then restored its import and closed owned processes. Resource identity and
source manifest self-tests passed; no original graphics or ABI changed.
The four tested files were published to assets root and O:/winnt/system32,
with identical hashes:

| File | SHA-256 |
| --- | --- |
| demo.exe | `631926c0c9c880d9bef8b5508f82e3f06c057cd311e27bc079959640527d36b6` |
| ntw16s32.dll | `15c7ddd5cb07e5d1bbf6326338c50ff62a342fbdd6238bf804d3c6f058014c45` |
| ntw16s64.dll | `12f65fa747ae5c60776e8088d61f8142e026d08f9e535acac6f1200614cceeee` |
| run16s.exe | `698e64cadd972c0fe4dac26876ec511dd5cacdf99c7ea691125a70e493981baf` |

[Interim accounting](../etc/evidence/delphi-s62-count.json): 25 families with
at least one bounded implementation, 2 native-preserved, 47 without an
evidenced implementation. These are family classifications, not a count of
remaining coding tasks; bounded families may still contain unsupported variants.
M0 and T18 remain open.
The identical x86/x64 radio model run against the previous S61 package failed
at the positive attachment assertion with exit code 1 in both widths; the
current package passed. This is an old-implementation negative, not a claim
about every possible lookalike class.
The production diff is 4 added/4 removed lines across attachment and protocol
(net 0); test/build wiring is 16 added/3 removed lines (net +13). Search:
`rg -n -C 2 'TEdit|TMemo' src/ntw16s-dll` and
`rg -n -C 4 'edit_candidate|group_button_identity|verified_role' src/ntw16s-dll/attachment.c`.
The radio role, admission and parent lifetime guard share the existing native
selection owner; the old group path remains. The unproven memo candidate was
removed and deferred, rather than changing the existing edit path.
