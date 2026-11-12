# M0 T26 S1: shared control protocol recognition

`lib-style/common/control_protocol` and the `style-control-common` target own
existing native/VB/VCL/OCX role recognition and GUI-thread binding lifecycle.
The Win16 control component and product attachment callers directly use the
shared API; the optional OCX fixture links the same target. No Win16 palette,
renderer or product-private header is required. Existing exact admission and
owner checks, reference counting and error paths are preserved.

The private window property was renamed from `ntw16s.private.control-protocol.v1`
to `style.private.control-protocol.v1`, with all in-tree checks updated. This is
an observable internal property-name change: old already-loaded binaries do not
share the new name. The registry is thread-local and belongs to each linked
module; a foreign value under the new property prevents binding, so simultaneous
independent modules on the same window are not claimed to cooperate. Normal
style handoff must retire the old attachment before a new owner binds. No
Win2000 control consumer is implemented by this extraction.

The standalone common test uses an owned hidden window and checks rejection of
an unsupported class. The existing direct-worker protocol fixture covers
identity, reference cleanup, destroyed windows, allocation failure and a
foreign property without creating a private desktop. Other control rendering,
real OCX integration, concurrent style modules and full visual behavior are
outside these focused tests. T26's wider extraction and audits remain open.

The x86/x64 clean MinGW GCC, Ninja and ccache build passed from snapshot
`6950318d7a6d19e5e9be6cd66a752fd91ca3dd7b0aa5ace74e321c6168a9088c`.
All eight artifacts were published to assets and `O:\winnt` with matching
hashes before testing. Both widths passed 13 component executables, two
published-launcher probes, the direct protocol lifetime worker and the
control-extension worker loading the published DLL: 34 focused checks. The
extension worker reported 60 protocol cases per width, no early queries and
owned images freed. Both direct workers use self-created windows and bypass
their private-desktop launcher. The clean shared archives have no unresolved
Win16, Win2000 or product-private references. These results do not prove
simultaneous independent module ownership or real Win2000 control rendering.
