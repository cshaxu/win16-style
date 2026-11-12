# M0 T26 S1: shared nested control redraw suppression

`lib-style/common/control_redraw.h` now provides the existing redraw bracket:
visibility gating, nesting, native dispatch and conditional reenable. Win16
basic, list, combo and edit message paths directly use the same functions;
the former state-specific helpers and the edit path's duplicate bracket are
removed. Message selection, painting and restoration decisions stay in each
control. Callers must keep the supplied depth and flags alive through native
reentrancy; the helper does not own their state or install a subclass.

The standalone test uses an owned hidden window and a simulated native
subclass callback to exercise nested calls, detach, application redraw-off,
visibility and result propagation. Existing product integration checks remain
separate. This does not implement Win2000 controls or prove full visual fidelity;
the broader T26 migration and Win2000 consumers remain open.

The clean x86/x64 MinGW GCC, Ninja and ccache build passed from snapshot
`53bb5d7a969c56ce82673e5065775c2b10e91fb01aae2d34904001ce94993f24`.
Eight files were published to assets and `O:\winnt` with identical hashes
before testing. Both widths passed 15 component checks, two published-launcher
probes, the direct protocol lifetime and published-DLL extension workers, and
the published-launcher list-report worker: 40 focused checks. The list worker
reported 18 variants, preserved native data and callback behavior, and cleaned
up. Direct worker entry points bypass private desktop creation. The redraw unit
uses a simulated native callback; full interactive redraw and Win2000 consumer
coverage remain unverified.
