# M0 T21 S3: additive global hook ABI

The public DLL exports add `Ntw16sGlobalHook` at ordinal 34 and
`Ntw16sGlobalHost` at ordinal 35 on both architectures. The former is a
Win32 `HOOKPROC` (`CALLBACK`, three arguments); the latter is a rundll32-style
`CALLBACK` entry point (four arguments). The x86 DEF explicitly aliases their
stdcall-decorated implementations while exporting the same undecorated names
as x64. Existing ordinals 1–33 and the ABI version are unchanged. These are
controller/host integration entry points, not new styling interfaces; their
protocol is private to the matching four-file package. No fifth project
artifact is added. The frozen import ABI decisions remain historical input;
package and ABI probes separately verify these additive exports.
