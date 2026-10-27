# M0 T18 S26 — control audit (in progress)

The frozen S1 catalogue contains 74 unique VCL family names. The S2 demo
mapping is a native-model inventory, not proof that a Delphi wrapper is
supported. A name search of production source, native tests and T18 history
found 12, 18 and 21 names respectively; these are only leads for individual
protocol review. The 74-row disposition and the next implementation receiver
are not yet complete. S26 and T18 remain open.

A [provisional ledger](../etc/evidence/delphi-s26-current-ledger.json) joins
the frozen set, S1 receiver, S2 model inventory, exact source-name leads and
subsequent evidence without treating a name match as proof. It contains 74
unique rows, including 24 bounded assessments and 50 rows without an
established real runtime owner and content-neutral contract in these inputs.
All gaps remain assigned to T18 final integration or a specifically admitted
earlier implementation; no unsupported control is counted as complete. One
candidate is the real DoubleKiller `TAnimate`: the
current probe observes an application-owned window, style `54000003`, extended
style `00000000`, and `attached=0`. That observation alone does not establish
an appearance defect or justify interception. The Microsoft animation-control
[overview](https://learn.microsoft.com/en-us/windows/win32/controls/animation-control-overview)
describes application-owned AVI frames and playback messages; its
[styles](https://learn.microsoft.com/en-us/windows/win32/controls/animation-control-styles)
also define transparent-background interaction with the parent. Actual class
ownership, playback, surrounding frame, DPI and lifecycle therefore require
separate evidence before selecting this as an implementation receiver.

The same DoubleKiller observation exposes a more concrete next candidate outside
the 74 frozen names: `TSpinEdit` is an EDIT superclass and attached, but its
15-by-19 child `TSpinButton` has an application-owned class procedure, no native
window extra bytes, and is unattached. The Options-page capture shows the
associated up/down arrows in their native appearance. Existing T15 S11 covers
native UpDown and explicitly does not claim this custom control. Embarcadero's
[SpinEdit reference](https://embarcadero.github.io/DelphiVCL4Python/reference/delphivcl.SpinEdit.html)
describes a composite edit and spin button, consistent with this observed
window relationship; it does not prove the legacy build's drawing contract. The next
candidate should inspect the actual drawing and input boundary on an owned
instance before determining whether a content-neutral adaptation is feasible;
class name and screenshot alone are insufficient to implement it safely. This
is a separate discovered variant, not an extra member silently added to the
frozen 74-row denominator.

A subsequent scratch-only x86 observer, derived from the existing
DoubleKiller driver/probe, ran the current assets package on an owned isolated
desktop. On the Options page it confirmed that the visible 15-by-19
`TSpinButton` is parented by the attached `TSpinEdit`, whose text was `0` and
whose enabled state was false. The child reported its own enabled style, had no
child HWNDs, and produced no nonzero pixels when directly sent
`WM_PRINTCLIENT` into a zeroed DIB. The latter is an observation for this
disabled sample, not proof that it does not draw normally: the normal window
capture visibly contains the arrows. The probe did not click or alter the
application, and its owned job reached zero processes. This identifies a
concrete test-design requirement: capture normal painting and explicitly
exercise enabled, pressed and released states before selecting a renderer.

A second owned run briefly activated the `fil&es` checkbox, which enabled the
parent edit, then sent an upper and lower mouse-message click to the actual
`TSpinButton`. The displayed value changed `0 -> 10 -> 0`, the checkbox and
parent returned to their initial disabled state, mouse capture was released,
and the job again exited with no active processes. No disk scan was started.
This is an observed input contract for that sample, not a proof of physical
input, a public state-query API, pressed-state drawing, other Delphi versions,
or a completed style adaptation.

An additional test-only import observer on the same owned target temporarily
observed `DrawFrameControl` and `DrawEdge`, forwarded both, then restored both
import slots. When the enabled spin was explicitly redrawn, its own HWND was
reported by `WindowFromDC` for two `DrawFrameControl` calls with type 4 and
state `0x2010`; their reported rectangles were 15x9 and 15x11. During an upper
mouse-message press the first state became `0x2210` while the second remained
`0x2010`; during a lower press the second became `0x2210` and the first remained
`0x2010`. Both returned to `0x2010` after release. The two calls both reported
zero viewport origin and rectangles beginning at zero, so their actual clip and
coordinate mapping must be examined before painting; do not infer physical
position from these numbers. Both import observers installed and restored, the
`0 -> 10 -> 0` input and initial checkbox state were preserved, capture was
released, and the job exited empty. No DrawEdge call was observed within this
bounded sequence; that is not proof of its absence in all states. This provides
a concrete candidate public-primitive boundary, not a finished implementation or
a claim covering arbitrary VCL controls.

A follow-up trace distinguished the mapping: the first call had window origin
`(0,0)` and clip `(0,0,15,9)`; the second had window origin `(0,-8)` and clip
`(0,0,15,11)`, both with viewport origin `(0,0)`. The local logical rectangles
therefore cannot be compared as untransformed window coordinates. Pressed-state
changes, `0 -> 10 -> 0`, import restoration and job cleanup repeated. Any
candidate painter must respect each caller's DC mapping, clip and
`DFCS_ADJUSTRECT` contract, and must retain the application's arrow content.

A task-scratch feasibility experiment forwarded the original
`DrawFrameControl`, saved its result and error, and framed the pre-call
rectangle only for the observed target HWND and push-button flags. It ran on
the owned application's normal painting path, without changing product code.
For the enabled 15x19 control, direct client-DC captures before and after the
experiment reported 77 changed RGB pixels and zero changes within the tested
interior regions; this is a bounded pixel comparison, not full visual fidelity
or proof that all arrow pixels and states are preserved. With the experimental
frame still enabled, mouse-message clicks again produced `0 -> 10 -> 0`;
checkbox and capture state were restored, the import observers were removed,
and the owned job exited empty. This shows a possible bounded frame path and
identifies a concrete next implementation candidate; disabled/pressed pixel
comparisons, owner/class spoofing, DPI, reentry, non-target preservation and
real application keyboard behavior remain unverified.

The feasibility observer was extended to compare native and experimental
client pixels while the upper button was held, the lower button was held and
the parent edit was disabled. In each state both captures and redraws
succeeded; all 77 changed pixels lay on the two measured 15x9/15x11 frame
perimeters (shared row 8), with zero changes in the remaining interior.
Input still returned `0 -> 10 -> 0`, the disabled checkbox state and capture
were restored, both temporary imports were restored and the owned job emptied.
The disabled sample also received the frame; its arrow and interior pixels
matched the corresponding native capture. These are three states of one x86
sample at 96 DPI, observed with synthetic messages; they do not establish
keyboard, other DPI, focus, reentry, lifecycle, spoof resistance or general
Delphi compatibility. The experiment remains scratch-only.

The four fixed, SHA-identified local Delphi candidates were searched read-only
for complete ASCII identifiers and UTF-16 name leads. Among the 74 frozen
families, 48 have at least one name match and 26 have none in those four
files; 25 of the rows without an established runtime owner have at least one match. Raw names,
counts, paths and file hashes are retained in the S26 scratch ledger and scan.
These are bounded search leads only: a binary string can be type metadata,
code or an unused resource and does not establish a live control, ownership,
paint contract, supported style or absence from another application. A
single-pass ASCII matcher replaced the original repeated regex scan and produced
byte-identical JSON on the same inputs.

The S3 desktop prerequisite was rechecked against the currently deployed
assets on 2026-10-27. In the sandbox both x86 and x64 superclass drivers
returned `PRIVATE_DESKTOP_CREATE_FAIL error=5` before starting a target. The
same self-owned private-desktop probes outside the sandbox succeeded. Both
superclass drivers passed their styled and unstyled 96/144 DPI scenarios,
including bounded button, edit, list and choice protocols, negative cases,
detach pixels, and `SUPER_CLEANUP_OK owned_jobs_empty=4`. Both ownership
drivers returned `PROTOCOL_OWNER_OK`, including invalid and destroyed cases
and zero remaining allocations and bindings. These are native protocol models,
not Delphi runtime tests or physical input acceptance.

The existing self-owned DoubleKiller driver also ran outside the sandbox using
`assets/run16s.exe`, the original x86 target SHA-256
`cdd83ab909db749992d11ace2dff671493c0568d7a14697f8bef4b06d8647ca8`, and
the assets DLL SHA-256
`735f7a050a94f88f6da39872bb1e2db07d5f2fd4977445a8b64ac30d0a443e04`.
It identified the actual target and loaded DLL path, observed all four pages,
and each page reported 28 buttons, three edits and two combos, all 33 attached
with zero failures. Its page-zero detach comparison reported 1,843 changed
pixels. The owned job exited normally with zero remaining processes. This
establishes the driver's limited real-application observations, not that all
VCL families, states or physical interaction have been validated. The native
screenshots and raw logs remain in the registered S26 scratch while the
audit is active. HD Tune elevation remains skipped per user instruction.

The existing x86 `vb-actor-observe` was also run against the actual assets
launcher and `O:/winnt/actor.exe` on an owned isolated desktop. It identified
the loaded assets DLL and `SysWOW64/msvbvm60.dll`; each of the two real VB6
checkboxes matched 182 original glyph pixels and four message-driven clicks
returned it to its original state. `ACTOR_PASS styled=1 count=2 active=0`
confirms this observer's cleanup. This is a limited regression for the two
observed controls at 96 DPI, not a renewed T17 closure or all-VB claim.

An interim clean x86/x64 build and same-snapshot four-file package succeeded
with source, original-resource and governance checks. Its focused actual-package
run passed 12 cases with zero skips in 7.86 seconds, including the two
superclass and two ownership probes and the required launch and propagation
baselines. The remaining 99 standard cases and the full integration matrix
were not run in that focused pass. This interim package has not been published:
S26 is still in progress, and its clean-build bytes differ from the installed
batch. Its hashes and the complete selected/omitted lists are retained in
`build/t18-package/s26-focus1/result.json` until S26 delivery.

The subsequent S26 ledger snapshot was clean-built for x86 and x64 using
MinGW GCC 16.1, Ninja and ccache; source, byte-exact original resources,
governance and ABI checks passed. Its actual four-file package passed 12 focused
cases in 8.48 seconds with zero skips; 99 standard cases, the separate full
integration matrix and physical input were not tested in that run. Because its
four file hashes differed from the installed batch, the tested batch was
published to assets root and `O:/winnt/system32` with all eight hashes matching.
The installed assets x86 and system32 x64 superclass entry points then passed
their 96/144-DPI owned-desktop checks and cleanup. The batch SHA-256 values are
`20295e420b044ca3fb31502d49f3401571dcac4993971cfcf6d05a3220099f7b` (demo),
`962c70a2b020ac32c064c3615674789f9984cafd8838fded566f978b0970ff9a` (x86 DLL),
`0b2a7ca3a94fe343398f204e8501240aff49e013d5d7850056f7f2d333c84449` (x64 DLL),
and `4787e2b0f39729afe2cbbc4b895989fffff7f47ebc6b80815a3526a9317ca27c`
(launcher). The source implementation is unchanged in this S26 delivery;
the probe-only `TSpinButton` experiment is not part of the published product.
An installed-assets real VB6 Actor repeat initially stopped with Win32 error 80
because its screenshot names already existed in the shared scratch folder;
it had loaded the new DLL but did not complete the assertions. A fresh owned
output folder fixed that test-input collision: both checkbox glyph comparisons
passed at 182 source pixels and `ACTOR_PASS styled=1 count=2 active=0` confirmed
normal cleanup. This repeat does not enlarge the VB coverage claim.
