# M0 T18 S41 — VCL date-picker investigation

Self-owned DelphiVCL4Python 1.0.7 x86 and x64 instances at 144 DPI expose a
runtime-owned `TDateTimePicker`, currently unattached. Its class module
forwards WM_PAINT and WM_NCPAINT to the registered native date-picker
procedure. `DTM_GETDATETIMEPICKERINFO` succeeds and, for a 130x24 client,
reports button rectangle (104,0)-(130,24). This is one runtime and one style;
there is no existing project date-picker renderer.

Synthetic mouse messages opened a visible `SysMonthCal32` owned by the same
process and then closed it. The original date survived; dropdown and close-up
notifications were each observed once. A focused synthetic VK_UP changed
2026-10-27 to 2026-11-27 and produced one date-change notification; the
original value was restored through the public date API. This is not physical
input coverage or proof of every date field and format.

A temporary probe reused the existing Win3.1 combo glyph with proportional
DPI coordinate scaling. Normal and disabled states changed 552 pixels, and
the open state changed 555 pixels, all within the public button rectangle, on
both widths. The open button reported public state 8. Native repaint restored
the probe drawing; the popup and original date were then restored. This is a
scratch experiment, not product support. Checkbox, custom-draw variants,
calendar internals, other DPI, clipping, nested messages and resource
lifecycle remain unverified. One x64 open-state probe returned exit 1 after
writing its report without normal capture/close confirmation; a separate run
passed, but the first failure's exact cause is unknown and retry is not a fix.
The [evidence](../etc/evidence/delphi-s41-date-audit.json) records the bounded
observations. All probes run under self-owned jobs.

No product code or original graphic resource changed. The independent clean
GCC 16.1.0/Ninja/ccache snapshot SHA-256 was
`ec0acebbc69959c443ef40b124a996bbb08eea2f645bc8ce3ca3e72dc1eadf87`.
The actual same-batch package passed 12 focused cases with no skips in 10.41s;
other package cases and physical input were not run. The rebuild changed the
binary hashes, so all four tested files were published to assets and system32;
both system32 x86/x64 installed-entry probes passed. Both locations matched:

| File | SHA-256 |
| --- | --- |
| demo.exe | `3c90e6d44d070ea17f8ade154af62537a95b87ae3b876c53cbafa4f47596fab5` |
| ntw16s32.dll | `fee7d9f1cd133f4b080e31dc86f60d1f0900bd372c40a47619adab2860f885c3` |
| ntw16s64.dll | `75d89a697a64b3764c19cf5fe63036a6c8f114c2b931458b3af561905427f3a1` |
| run16s.exe | `44c40ad4887138bbfec749b44bebfc079753d50f3a6af66110587d7ffa5a1ea2` |

P1 `a85346e` was pushed and all seven changed files were retrieved from the actual GitHub commit and matched byte for byte; the remaining T18
inventory and the screen mechanism from S40 remain open.
