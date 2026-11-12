# M0 T26 S1: shared UWP global target manager

`lib-style/uwp/global` now owns the existing target observation, deduplication,
identity revalidation, queue/worker coordination, statistics and bounded stop
lifecycle. The product manager supplies the existing ApplicationFrameHost image
filter, x64 host policy, active-session check, Win16 target-change operation and
diagnostics through copied callbacks. The callback context and process policy,
module path and event handles remain borrowed until successful stop. This is a
migration of existing discovery and lifecycle; it does not add hook propagation,
change target permissions or introduce a second process-global engine.

The standalone native test starts its own synthetic frame/core process on the
current desktop, observes only that window and checks apply, duplicate admission,
statistics, stop and removal through test callbacks. Its image filter is test
only; product admission remains the exact Windows ApplicationFrameHost filter.
The synthetic process does not prove actual UWP host styling, cross-style use,
or complete runtime recovery. Those and the broader T26 audit remain open.

The clean x86/x64 MinGW GCC, Ninja and ccache build passed from snapshot
`f88626d8d1dfcc66e041c727302d0cc2fc68822044b5ff36d6aa6a70acdfe194`.
All eight files were published to assets and `O:\winnt` with matching hashes
before testing. The test-owned process initially failed policy inspection with
`ERROR_ACCESS_DENIED` inside the command sandbox; no target was admitted. The
same clean-snapshot x86/x64 test passed outside that sandbox without changing
system security settings, reporting one apply, one remove and no duplicate.
Both widths also passed 17 component tests, two published-launcher probes and
two direct protocol workers: 44 focused checks including the manager test.
The shared manager archives have no unresolved Win16, Win2000 or product-private
symbols. This does not establish real UWP host or full global-mode acceptance.
