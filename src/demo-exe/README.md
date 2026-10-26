# Native demo

One x64 Windows GUI application. The original main window retains its native
controls, Apply, File/Reset, Help/About and normal message loop. The Controls
menu opens eight independent example windows: buttons and states; editing and
lists; files and scrollbars; containers and application drawing; lists and
toolbars; ranges and dates; rich text and special objects; and MDI documents.

These are native Windows counterparts for the VB5/VB6 audit, not real VB or OCX
controls. The 64-bit demo does not host 32-bit VB runtimes. File listings are
read-only and bounded; the native file dialog only selects a path. Timer and
in-memory record examples do not implement external binding or embedded OLE.
Images, shapes and charts illustrate application content boundaries. Legacy
grids, windowless ActiveX and other unmatched families remain separate work.

Windows owns controls, menus, fonts, focus, input and notifications. The only
dynamic library load is fixed system Msftedit.dll for native RichEdit. There
is no style ABI, appearance switch, test counter or capture code. Run the same
demo.exe directly for native appearance or through run16s.exe for styling.
External observers live in tests. Native counterparts demonstrate existing
support and remaining visual gaps; they do not prove real VB compatibility.
