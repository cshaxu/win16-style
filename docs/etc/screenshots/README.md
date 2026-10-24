# Screenshot provenance

These eight PNGs illustrate the English project README. They are real application
captures, not mockups. Exact image, application, and product SHA-256 hashes are
in [manifest.json](manifest.json).

## Applications and package

- **demo.exe:** the unchanged x64 executable in [assets](../../../assets/README.md).
- **sol.exe:** Microsoft Windows XP Solitaire, x86, version 5.1.2600.0
  (`xpclient.010817-1148`), using the matching version of `cards.dll`.
- **Win16 Style:** version 0.8.0.0, the four checked-in files from the completed
  menu typography delivery. No product binaries were rebuilt for these images.

The Solitaire files came from a pre-existing local copy. Neither `sol.exe` nor
`cards.dll` is redistributed in this repository. Solitaire's images and game
content remain Microsoft's material; the project license does not relicense them.
The project's original embedded artwork retains its separate
[resource provenance](../../../src/ntw16s-dll/resources.md).

## Capture method

A temporary C17 controller, compiled with MinGW-w64 GCC, starts only its own
applications on an owned private desktop. It verifies job ownership, the exact
matching styling DLL path for injected launches, and the absence of these DLLs
for direct launches. The same checked-in application executable is used for each
before/after pair.

Each application is captured with native cross-process `PrintWindow`. Visible
menus and dialogs are printed separately and composited at their real window
coordinates. Dialogs are centered within the owner window before capture.
These are application-window captures, not full-desktop compositor screenshots.
No computer-use, physical keyboard/mouse input, generated artwork, or retouching
is involved. PNG conversion preserves the captured pixel dimensions.

The demo reports **144 DPI**. The DPI-unaware Windows XP Solitaire reports
**96 DPI**. The native appearance on another desktop or Windows configuration
can differ. Solitaire launches deal independently, and animated card backs can
change between captures. The cards, board, and Score/Time line are painted by
Solitaire itself.

## Gallery

| Image | Capture |
| --- | --- |
| [demo-native.png](demo-native.png) | Direct native demo, 144 DPI. |
| [demo-styled.png](demo-styled.png) | Styled demo, 144 DPI. |
| [demo-menu.png](demo-menu.png) | Styled demo's real File menu. |
| [demo-about.png](demo-about.png) | Styled demo's real About message box. |
| [solitaire-native.png](solitaire-native.png) | Direct Windows XP Solitaire, 96 DPI. |
| [solitaire-styled.png](solitaire-styled.png) | Styled Windows XP Solitaire, 96 DPI. |
| [solitaire-menu.png](solitaire-menu.png) | Styled Solitaire's real Game menu. |
| [solitaire-options.png](solitaire-options.png) | Styled Solitaire's real Options dialog. |

Only these selected documentation images and provenance are retained here.
Temporary capture programs, original BMPs, application copies, and diagnostic
logs are task scratch; their relevant results and hashes are recorded before
cleanup. The screenshots demonstrate the pictured cases, not a claim of complete
Solitaire functional testing or universal application compatibility.
