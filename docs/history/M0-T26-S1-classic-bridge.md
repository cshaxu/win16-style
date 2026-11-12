# M0 T26 S1: shared classic dialog bridge

The classic file-dialog bridge now resides in `lib-style/common`: COM show and
result conversion, Shell folder browse planning/compatibility, and live file
name/filter synchronization. The Win16 proxy dispatch and product adapters use
these interfaces directly. The implementation retains its existing classic
control IDs, capability checks, native fallback and opt-in behaviors; these are
explicit classic-dialog contracts rather than a universal modern-dialog API.
The caller owns the state and provides session policy, event policy, native
fallback and display scope; no Win16 drawing or product headers are included.

The standalone native test exercises inactive-session native fallback and Shell
browse planning. Existing Win16 dialog integration tests cover the adapter's
other mapped paths. Neither proves an interactive COM dialog, another style's
end-to-end use, or full UWP behavior. Further dialog helpers and wider T26
window/control audit remain open.

The x86/x64 clean MinGW GCC, Ninja and ccache build passed using snapshot
`a30bab9701f496ea736e4d330ec6bbd6e7b042f0577c85b572a7ac83566ffb82`.
All eight files were published to assets and `O:\winnt` with matching hashes
before tests. Both widths passed ten component tests (including classic bridge,
COM adapter, Win16 link and chrome) and two published-launcher menu and Win2000
geometry probes: 24 focused checks. Public process-library binary boundary
checks passed both widths. No interactive file selection or actual alternate
style consumer was tested.
