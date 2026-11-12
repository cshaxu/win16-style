# M0 T25 S8 P13: shared frame redraw correction

Solitaire's native menu could overwrite the custom menu immediately after a
non-client redraw. P11's later client invalidation did not prevent the user
from seeing it again. The Win16 path already suppresses host themed caption
and menu messages and handles activation without forwarding native painting.
Both styles now share that message policy in `style-common/frame.h`.
Win2000 handles non-minimized activation itself and restricts native
non-client painting to a region outside the client, retaining native scrollbar
fallback until a Windows 2000 scrollbar renderer exists. The existing Win16
non-client DC/region painter is also moved into the common header with
style-supplied painting callbacks; Win2000 does not yet use that entry.

The existing common global-window transition, DWM policy, geometry restoration
and menu hit routing remain shared. The two styles still have separate menu
tracking and system-menu implementations; this is not a complete S9 merge.
Win16 scrollbar geometry, rendering and input are style-specific and cannot be
presented as a Windows 2000 implementation. No configurable Windows 2000
scrollbar painter was completed by this delivery.

An in-process controlled window reproduced the P11 defect immediately after
WM_NCPAINT (`immediate=0`, `click_clean=0`), before any WM_PAINT. With the
candidate, x86 and x64 reported clean pixels immediately, after activation and
after six message-driven client clicks. Repeated focused runs were clean in
both architectures. Native horizontal and vertical scrollbar paint, menu hover,
geometry and 24 resize steps passed on both architectures. One earlier x86
run observed `footer=0` (14-pixel difference); later repeats passed. It remains
an unresolved intermittent observation, not proof of a fixed status bar.

The clean C17 MinGW/Ninja/ccache x86 and x64 builds and package identity gate
passed; all eight files were published first to assets and O:/winnt with equal
SHA-256. Already mapped old ntw16s64 and ntwshk64 images were retained while
the paths were replaced. Post-publication tests used the published launcher and
matching built test binaries. The focused package result reports skips: its
driver attempted private desktop creation and Windows returned error 5. This
violated the user's no-private-desktop instruction; the driver was stopped and
the equivalent listed cases were run directly without that driver. No claim is
made for real mouse input, full integration, or user visual acceptance.

The real mouse probe was previously rejected by automatic approval review as
conflicting with the computer-use restriction; it was not rerun or bypassed.
The newly published version and remaining shared-mechanism work require further
validation. S8 and T25 remain open.
