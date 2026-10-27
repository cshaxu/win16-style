# M0 T18 S57 — bounded VCL text toolbar

S56 observed that a real DelphiVCL4Python 1.0.7 TToolBar under TControlBar
forwards WM_PRINTCLIENT to the system toolbar during its own WM_PAINT, has a
text-only button with command id 0, and emits native custom-draw notifications.
The implementation admits only that same-thread/same-runtime parent-child
identity after observing that forwarding and validating the public toolbar
protocol. The existing custom-draw painter is reused. A zero command is
accepted only for this witnessed VCL role; TB_COMMANDTOINDEX must map each ID
to its own position, so duplicate IDs fall back. Image, separator and other
unsupported variants retain native painting. The VCL role caps the scan at
64 buttons; the cap itself has not been separately stress tested. Existing
OCX zero-command handling was not widened. On parent/role mismatch the exact
attachment is retired.

In real x86/x64 144 DPI samples, a full-size button generated item-prepaint
and the product returned CDRF_SKIPDEFAULT after its own drawing. Synthetic
mouse messages produced WM_COMMAND with id 0; removal succeeded. Same-window
client images before/after removal changed 1,214 pixels per width. An earlier
sample had a 33px button in a 22px toolbar: attachment existed but the drawing
remained native because the button rectangle exceeded the client. Attachment
alone is therefore not proof of styling. Tests do not assert physical input.

A self-owned C module verifies false forwarding, zero ID, duplicate ID,
image fallback, synthetic click, application default and skip callbacks,
reparent and cleanup on both widths. Both older released DLLs fail the new
positive attachment check. The first clean all-target build failed because an
existing standalone toolbar test lacked the new runtime-query stub; that test
was updated and the next independent clean build passed. It did not alter the
old toolbar reader's behavior. Original graphics and public ABI are unchanged.
Images, large lists, additional runtime versions/DPI and full T18 integration
remain unverified; parent ControlBar styling stays separately in TODO.

The clean GCC 16.1.0/Ninja/ccache 4.13.6 snapshot manifest SHA-256 was
`b12182c60d7eccf8255596ebea475009f71324b524e95f8e0f5219a1229a42f2`.
PE/ABI checks passed; 18 focused same-package cases passed in 6.78s at four
jobs, no skips. Other cases and physical input were not run. Tested files were
published to assets root and O:/winnt/system32; installed x86/x64 model tests
passed and both destinations matched:

| File | SHA-256 |
| --- | --- |
| demo.exe | `cd7a7bab74c2abab4fc7b4d21fe065305784f27642b09d17f8cec79c84255447` |
| ntw16s32.dll | `258d681e34369604fe60ca9134c56d8d9ea93a68b5662d266e02584d443d760b` |
| ntw16s64.dll | `4e48e988efec3ceba092a3af06ea6564fbaf969e455b79ca2f962f5f8778698a` |
| run16s.exe | `32d4bee99dc6b34c79254643d79b9d0dbd79a0be8ec3868d9a41bf1ef5f8abef` |

Search `rg -n 'runtime_toolbar|vcl_toolbar|TOOLBAR_MODULE' src tests`
identifies witness, binding, bounded reader and test. M0/T18 remain open.
