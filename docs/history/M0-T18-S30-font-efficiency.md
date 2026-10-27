# M0 T18 S30 — font-layout test efficiency 

The S29 actual-package run spent 79.06 seconds in x86 and 63.51 seconds in
x64 `font-layout` cases; 14 cases completed in 80.27 seconds with four jobs.
To locate the cost without changing the test contract, a task-scratch x86 copy
of the existing test added only timing and GDI-count logs, then ran against the
published assets DLL on an owned isolated desktop. The original eight full
warmup cycles and all assertions remained in place, and the test passed.

In context 0, warmup iterations took 1.91, 2.16, 4.03, 4.20, 4.31, 4.00,
4.25 and 4.05 seconds; GDI counts were 22, 45, then 43 for the last six
iterations. In context 1, each iteration took 4.03–4.30 seconds and the count
was 45 throughout. Menu plus reentrant checks took 0.09 and 0.16 seconds;
final before/after GDI counts were 43/43 and 45/45. This one x86 profile shows
that repeated full warmup dominates this run; it does not prove every platform
stabilizes at those counts, nor that a single unchanged count is a safe
universal stopping rule. No product or source-controlled test has been changed
yet. A candidate optimization must retain the full tested variants, independent
glyph and layout oracles, both contexts and meaningful resource-leak negative
checks, and must be timed and validated on both architectures.

An initial test-only candidate replaces the fixed eight warmup rounds per
context with up to eight full rounds, stopping only after at least three rounds
and two consecutive non-increasing post-cleanup GDI counts. The subsequent
independent before/after resource assertion is unchanged. Every warm round
still executes all dialog and control variants; the final pass still checks the
same glyph pixels, per-character advances, DPI, geometry, reentry, font changes,
detach and resource count. A self-test rejects synthetic monotonically growing
counts and injects four real held GDI DCs, checking each increment is rejected
and that cleanup restores the baseline. Both incremental x86/x64 runs passed;
the x86 warm counts were 22,43,43,43 then 45,45,45. The first two attempts at
real fault injection using brushes failed because those brush allocations did
not reliably increase `GetGuiResources` on this system; replacing the witness
with actual retained DCs produced observed counts 3,4,5,6,7, then cleanup.
These preliminary runs are not the required clean-package timing comparison;
the detector remains bounded and is not a proof against all intermittent
resource growth patterns.

The first S30 clean-build attempt stopped at the source-manifest gate because
the adapted test file's import receipt still had its old digest. The receipt
was updated for that test file only, preserving its fixed source identity and
notice; the manifest self-test then passed. The stopped build is not counted
as a package pass.

## Final validation and delivery

The final clean x86/x64 MinGW/Ninja/ccache build and source, resource, ABI
and governance gates passed. The actual four-file candidate passed 14 focused
cases in 46.32 seconds, with zero skips; 103 other cases were not selected.
The x86 and x64 font cases took 45.02 and 32.85 seconds respectively, versus
79.06 and 63.51 in S29. The final pass retains both DPI contexts, all control
variants, glyph and layout comparisons and strict before/after resource checks.
The adaptive plateau check is bounded to eight cycles and cannot prove freedom
from intermittent or delayed resource growth. Its self-test detects increasing
synthetic counts and real retained DCs.

The former eight-cycle test remains available as `--stress` and is included
only in `-FullIntegration`; x86 passed separately in 69.67 seconds. The x64
stress run and remaining full integration are deferred to T18 final acceptance.
The `--menu-only` path also passed on x86. No production source, graphics or
public ABI changed. The same tested four files were published to assets and
O:/winnt/system32, with all eight deployed SHA-256 values matching the tested
package. Test changes update only the corresponding adapted-source receipt.
