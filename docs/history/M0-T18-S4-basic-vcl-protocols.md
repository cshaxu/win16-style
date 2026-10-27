# M0 T18 S4 basic VCL public protocols

S4 implements the first bounded VCL wrapper group without relying on private
VCL object memory or a component-name prefix. Exact `TEdit` and `TListBox`
registered classes are accepted only when executable-module/native-class
identity and their documented USER32 message contracts all agree. `TEdit`
uses the existing edit lifecycle path; `TListBox` uses the existing standard
list path. Owner-draw, virtual and protocol-impostor controls remain native.

`control_protocol_owner` now creates app-owned, native-extra-byte `TEdit` and
`TListBox` impostors using `DefWindowProc`; both are rejected. This is the
specific negative that an old name-and-owner-only implementation would miss.
`superclass` adds exact native-superclass fixtures for the positive attachment
and teardown route. There is no new ABI, component, product file or graphic
resource; all five original graphic headers remain byte-identical.

The clean x86/x64 core matrix passed. The four-file M0 T18 candidate passed
its package identity gate and the actual 87-case package run completed
`passed-with-skips`; the session's private desktop error 5 skipped GUI cases,
including the new runtime fixtures. This is not a VCL-app integration pass.
The tested hashes were published together to `assets/` and
`O:\winnt\system32` with backups and post-copy hash verification.

Real Delphi startup, injection, displayed rendering, keyboard/notification
behavior and cleanup remain deliberately deferred to the final T18 integration
audit under the user's approved sequencing. The machine-readable evidence is
[delphi-basic-protocols.json](../etc/evidence/delphi-basic-protocols.json).
