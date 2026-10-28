# M0 T18 S64 P55 — VB6 checklist at 144 DPI

The released renderer already replaced the observed runtime-owned 11×11
checkbox bitmap at 96 DPI. A private DPI-aware copy of the previously studied
CBMXfer VB6 sample showed the native and P52 checklist images were identical at
144 DPI. The real `ThunderRT6ListBox` item was 22 pixels high; the VB6 runtime
called `BitBlt` with a 44×11 source, an 11×11 destination at item offset (2,5),
and source offsets 0 and 11 for unchecked and checked states. This is one x86
runtime/sample, not a general claim about other layouts.

The renderer now accepts that exact 144-DPI VB6 geometry, while retaining the
existing runtime ownership, bitmap, public list state, DC, enabled state and
reentrancy checks. It uses the existing original checkbox resource at the
observed 11×11 size. VB5 at 144 DPI, other item heights and other DPI remain
native. The 96-DPI rule, input, selection, text, resources and ABI are not
intentionally changed.

In the final four-file package, the two real 144-DPI sample captures matched
the unchecked and checked original glyph at 121/121 pixels each; relative to
the native captures, 49 and 73 pixels changed, all within the 11×11 glyph.
Selection was read back as 0 then 1. Disabled/reenabled states were also
observed, but their complete pixel and physical-input semantics were not
certified. At 96 DPI, after equivalent explicit repaint, old and new package
captures for both states were byte-identical. Before aligning repaint timing,
one candidate capture was native while the old capture was styled; it was not
used as a regression conclusion.

The real sample observer timed out on shutdown with a hidden Options form and
the runtime main window remaining. This occurred with both the old and new
package at 96 DPI and with both at 144 DPI; the probe's self-owned job was
reclaimed, but this is not a normal-exit pass. The original unstyled 144-DPI
sample did exit normally. This pre-existing styled lifecycle issue remains
unresolved and is not explained by the new glyph rule.

The new self-contained 144-DPI model covers both PE widths, VB6 checked and
unchecked enabled/disabled states, wrong source and geometry, wrong height,
and VB5 native fallback. Both old-package VB6 models failed as expected. All
four final-package 144-DPI model combinations passed; the two actual published
directories also passed the x86 and x64 VB6 models. The same-batch package
passed 12 focused cases at two jobs in 39.37 seconds, including both VB5/VB6
96-DPI models and initial injection/propagation, with no skips. A previous
four-job attempt timed out in the x86 checklist child and was not counted as a
pass. The two registered x64 CTest entries also passed in 10.37 seconds after
providing the required owned capture directory; an initial invocation without
that environment failed before executing either test. Physical input, other
DPIs, VB5 144-DPI real samples and full integration remain uncovered.

The clean GCC/Ninja/ccache build, ABI, source and resource identity gates passed.
The four published SHA-256 values are: run16s
`d1af4afe23c67cc0f644a2be7a750dbe544a81ef97cb64f36c17abc0be52ad3a`,
ntw16s32 `ee5e312f382347e9774e563ecb329c2a238ffca725abd382a0c83a9b0e506efc`,
ntw16s64 `ddfdd6157a1b6335def3f2f8adad656cd804f56780ef065adfa66ac13339774f`,
and demo `eb24ed4326fad719f25279c55a3bd0721f6c7e8133012aea86d129f95c586b1f`.
Both `assets` and `O:\winnt\system32` match the tested files. Production source
changed by 6 additions and 3 removals in `controls.c`; the existing fixture
changed one line, and the 144-DPI test is project-authored. The native runtime
continues to own item data, selection and notifications. No original graphic
or provenance file was changed.
