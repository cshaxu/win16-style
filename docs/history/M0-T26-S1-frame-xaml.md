# M0 T26 S1: shared frame helpers and XAML child detection

The Win16 frame no longer defines pass-through wrappers for frame refresh,
client-screen geometry or client restoration. Its callers directly use the
same `lib-style/common/frame.h` helpers already used by the Win2000 frame.
Existing style-specific activation, DWM, geometry and removal decisions stay
with their owners.

The existing Win16 XAML child detection for `Windows.UI.Composition.DesktopWindowContentBridge`
and `Microsoft.UI.Content.DesktopChildSiteBridge` is now `style_frame_xaml_bridge`
in the shared frame header. Its class matching and optional visibility behavior
are unchanged; Win16 frame dispatch and layout use it directly. It is a detection
primitive only: it does not style Win2000 UWP/WinUI or implement XAML repaint.
The new standalone test registers both names in its own process, creates hidden
child windows, and exercises matching and the visibility gate; it does not show
or switch desktops. Real XAML rendering and the prior UWP lifecycle access-denied
problem remain unverified. T26's wider lifecycle and rendering audit stays open.

The x86/x64 clean MinGW GCC, Ninja and ccache build passed from snapshot
`5ebda72a6a7b09b1746b72c1be53b36e07fc3d6a1cf3e63130febcfdc74c0ad0`.
All eight files were published to assets and `O:\winnt` with matching hashes
before testing. Both widths passed 14 component tests, two published-launcher
probes and two direct control-protocol workers, for 36 focused checks. The
Win16 control link test includes client geometry restoration; the new XAML
case is synthetic and not a real UWP application. Existing UWP lifecycle and
visual limitations remain open.
