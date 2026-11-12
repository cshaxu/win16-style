# M0 T26 S1: shared UWP hook message handling

`lib-style/uwp/hook` owns the existing hook-message recognition, caller-owned
thread-local reentrancy guard, frame/core association and thread/process checks,
and error-plus-one result property. The product hook retains its exported hook
entry, `CallNextHookEx`, Win16 action selection, style application/removal,
menus and module pinning. Protocol names, accepted actions and the action
callback are provided by the caller. This is not a new hook installation or
propagation mechanism; the existing thread dispatcher remains responsible.

The native test uses owned hidden synthetic frame/core windows and a dedicated
fixture DLL. The real thread hook exercises success, target error, nested
message suppression, unrecognized action, invalid core, machine/identity
mismatch and result cleanup. The fixture does not implement Win16 drawing or
represent a real UWP process. Real UWP host interaction and cross-style runtime
consumers remain unverified; T26's wider migration and final audit remain open.

The clean x86/x64 MinGW GCC, Ninja and ccache build passed from snapshot
`ba223a078a955635d66f3afe6b0997cab0982af4633ea7a081e5500ed5703edc`.
All eight files were published to assets and `O:\winnt` with matching hashes
before testing. Both widths passed the synthetic target/hook round trip, 16
component checks, two published-launcher probes and two direct protocol
workers: 42 focused checks. The fixture runs the real shared hook handler on
a test-owned window, not the product hook inside a real UWP host. Clean shared
hook archives have no unresolved Win16, Win2000 or product-private symbols.
