# M0 T21 S6: multi-GUI-thread and bounded resource validation

A native private-desktop probe starts one x86 and one x64 owned target. Each target runs two independent GUI threads; each thread creates a root and Button before the controller starts, then creates a second root and Button while the global session is active. Three successive global sessions verify all four pre-existing and four new roots and their Buttons attach, then detach on normal stop. Each process and window is owned by the test job and is closed afterwards.

The preliminary run passed (`widths=2 threads=4 old=4 new=4 cycles=3`). After the first warm-up and after the third cycle, the x86 process handle counts were 479/479 and GDI object counts 55/53; x64 handle counts were 216/216 and GDI counts 22/22. One notification timed out or failed in the first initial sweep; the targeted windows still passed their state checks. The probe rejects unreadable or zero resource counts and fails on a rise greater than two in either counter. This is a bounded comparison across three cycles, not proof of no leaks under long-running stress; USER objects, heap size, forced termination and other application behavior are not covered by these counters.

S6 does not change product code or claim extra controls, DPI or higher-integrity support. Final build, actual-package results, deployment hashes and remote review are recorded below before closing this S.

The final `s6-threads-verified` clean x86/x64 build and ABI/source/resource gates passed. Its actual package passed 18 focused cases, no skips (`s6-threads-check`). In that final run x86 handles were 487/487 and GDI objects 53/53, and x64 handles were 222/222 and GDI objects 22/22. The first and third start sweeps reported two and three timed-out/failed window notifications respectively, while every targeted window passed attachment and removal checks. This distinction is retained: notification failures were not investigated individually and the results do not prove every desktop window was reached.

The same rebuilt four-file package was published to assets root and O:/winnt root:

- run16s.exe x64: `e6c19d58ac81df4457fd8ce3c80542ad391c9e7d257f5711b14513aeabdbab14`
- ntw16s32.dll x86: `ae58776fc5f004ecf23193ddc604a1c692975d7657f62f6fb020588aeb204d51`
- ntw16s64.dll x64: `0c5fd0eab3ea26329058e1a156bd87510fff406755178e79ba84edcfbb4e999a`
- demo.exe x64: `e947fada1d3b3d345ca7f3b1a959eebcbb28f53ef6d528ed43782a96e35c20e1`

S6 changed tests and build/test registration only. The new binary hashes differ from S5's clean batch; both deployment locations were therefore updated as required. Remote review follows the delivery commit.
The staged diff before this accounting line covered 11 files, 130 insertions and 2 deletions, primarily the two native test fixtures and documentation. No production source changed; target processes, GUI threads and all handles in this probe have test-owned cleanup paths.
