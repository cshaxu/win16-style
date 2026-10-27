# M0 T18 S27 — VCL spin-button frame

S26 established a real `TSpinEdit`/`TSpinButton` pair in 32-bit DoubleKiller,
its `DrawFrameControl` arguments, DC window origins and clipping, as well as
normal and pressed message-driven input. S27's initial implementation is a
working-tree candidate only. It restricts attachment to same-thread,
same-process application-owned exact `TSpinButton` under an application-owned
`TSpinEdit` with an EDIT superclass. A nested per-thread message scope limits
the overlay to the attached window's drawing; the caller module, frame flags,
DC owner, logical clip and transformed client bounds must match. The original
primitive is forwarded, its adjusted rectangle, return and last error are
preserved, and only a one-pixel black frame is added after success. The native
application retains the arrow and input. Unknown identity, flags, mapping and
scope retain native drawing. This is not yet a claim of safety against every
same-name impostor, all nested callbacks, all DPI or Delphi versions.

Both incremental MinGW x86 and x64 DLL builds succeeded. A self-owned isolated
DoubleKiller instance loaded the candidate x86 DLL, reported its TSpinButton
attached, and synthetic upper/lower clicks changed the value `0 -> 10 -> 0`;
the checkbox, disabled state and capture were restored and the job exited
empty. A bounded comparison of the driver captures against the earlier current
package found 77 changed RGB pixels in the visible 15x19 button rectangle,
bounding box `(612,178)-(626,196)`; these two captures are separate runs and
do not establish pressed, disabled, keyboard, or all surrounding pixels. No
test-only frame overlay was loaded during this candidate run. This candidate is
not yet package-tested, published or committed. The required dual-width
positive/negative, passive, reentrant, clipping and DPI tests remain to be
implemented before delivery; T18 and S27 stay open.

The candidate now rejects a same-name `DefWindowProc` class in addition to a
wrong parent. A self-contained C model reproduces the two mapped/clipped native
frame calls and application-owned content. Both x86 and x64 passed at actual
96 and 144 DPI: 77 and 96 RGB pixels respectively changed, no changed pixels
were outside the frame, and native primitive result, last error and adjusted
rectangles matched after removal. The old published x86 package failed this
model with `SPIN_CONTROL_UNATTACHED`, establishing a relevant negative. The
new candidate also rejected wrong-parent and same-name DefWindowProc windows;
both widths remained unattached on passive DLL loading. After adding the
impostor guard, a new real DoubleKiller run still attached the target and
preserved the `0 -> 10 -> 0` interaction and job cleanup. The four focused
model cases and two passive cases are registered with the package runner, but
have not yet been run as an actual clean package. Model state coverage,
reentrant destruction/removal, clipping variations, other impostors and
broader non-target regressions remain open.

The self-contained model now exercises removal and destruction during paint:
the removed instance's subsequent pixels match native output, the destroyed
window is gone, and both x86 96-DPI and x64 144-DPI runs complete normally.
A nested unrelated message draws the same primitive into the same window DC
and is compared pixel-for-pixel to a native memory-DC reference; both tested
widths preserve it unchanged. A partial update-region comparison also passed:
x86 changed 24 pixels and x64 37, with zero differences outside the requested
clip. Its first draft reported outside differences because a newly created
control retained an earlier pending update region. Clearing that update region
before setting the test clip made the setup deterministic; the initial failure
is not represented as a product pass. These are focused model results, not yet
the final same-snapshot package or complete non-target regression.

The extended fake-class test found a genuine hole in the first guard: an ANSI
`DefWindowProcA` class was attached even though the Unicode fake was rejected.
The subclass machinery made `IsWindowUnicode` unsuitable for choosing a single
class-procedure view. The guard now queries both ANSI and Unicode registrations
and rejects either default procedure. On both x86 at 96 DPI and x64 at 144 DPI,
the ANSI and Unicode fakes are now rejected while nested, clip and reentrant
checks pass. A new owned real DoubleKiller run still attached its ANSI
TSpinButton and preserved `0 -> 10 -> 0` and job cleanup. This is not a claim
that arbitrary custom procedures mimicking the public contract are identifiable.

The first clean-build attempt correctly stopped at the source-manifest gate:
the two adapted-source receipts still contained their pre-S27 hashes. The
receipts were updated only for the two modified files, retaining their source
blob and original provenance, and the manifest self-test then passed. The
failed build is retained as a failed attempt rather than a package result.

The corrected same-snapshot clean GCC 16.1 x86/x64 build passed the source,
original-graphic identity, governance and ABI checks. Its actual four-file
package passed 30 focused cases in 10.38 seconds, zero skipped; the remaining
87 standard cases and separate full integration matrix were not run. The
focused set includes all six spin cases, both checklist and superclass cases,
12 status variants and the required launch/propagation baselines. A separate
self-owned DoubleKiller run loaded this candidate's actual x86 DLL, attached
the target, preserved `0 -> 10 -> 0`, and exited with zero active job processes;
its 96-DPI normal capture differed from the prior released capture by 77 RGB
pixels within the 15x19 spin rectangle. Different-run capture alone does not
establish other states or whole-window equivalence.

The tested batch was published to `assets/` and `O:/winnt/system32`; all eight
destination SHA-256 hashes matched. The installed launcher at each location
then passed x86 and x64 spin model tests, across actual 96/144 DPI, including
clip, nested, reentrant and impostor checks. The four batch SHA-256 values are
`b6a27dc4b0957c3e6fd59a7df0522f3ac8b7a2365294cdfda4979c6fe2b23e8c` (demo),
`2dfdf58b2d668848f0da962a80d79e09ccc89e0128bf24ae36803a66ef990c8d` (x86 DLL),
`9957ef73188558306bab81c2212d5e5b97e79e28a805235f7a5d58c7d8fa96be` (x64 DLL),
and `c6a78c9d150852977bf4c9290ab57b40f89d73ad2e5bb76233d66a3c9683b2b1`
(launcher). Real keyboard and physical input, other application implementations,
later-created or unusual styles, and broad T18 integration remain unverified.
Product source and test-only model remain pure C; no original graphics or
provenance bytes changed. No fifth product or public ABI export was added.

Source sweep `rg -n DrawFrameControl src/ntw16s-dll -g '*.c'` found the existing
startup-hook path as the production primitive interception owner; S27 composes
that path after native drawing, without installing another hook. The existing
checklist and status scopes remain separately owned and were included in the
focused package. The staged product-source diff is 70 additions and eight
deletions across two C files; the new native test is 153 lines and the build
and package-runner additions total ten lines. The old generic custom-control
fallback remains in place for all windows that fail the exact new gates.
