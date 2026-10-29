# Native demo

One x64 Windows GUI application. The original main window retains its native
controls, Apply, File/Reset, Help/About and normal message loop. The Controls
menu opens nine independent example windows: buttons and states; editing and
lists; files and scrollbars; containers and application drawing; lists and
toolbars; ranges and dates; rich text and special objects; MDI documents; and common dialogs. The Common dialogs page includes legacy Win32 file/folder, modern COM file/folder, find, replace, font, color, print, page setup, message, task dialog and property sheet examples. The modern-to-classic and modern-retained buttons are distinct comparison paths; modern-to-classic remains under development. Dialogs only choose values or paths; the demo does not open or save files or submit print jobs.

These are native Windows counterparts for the VB5/VB6 audit, not real VB or OCX
controls. The 64-bit demo does not host 32-bit VB runtimes. File listings are
read-only and bounded; the native file dialog only selects a path. Timer and
in-memory record examples do not implement external binding or embedded OLE.
Images, shapes and charts illustrate application content boundaries. Legacy
grids, windowless ActiveX and other unmatched families remain separate work.

Windows owns controls, menus, fonts, focus, input and notifications. The only
dynamic library load is fixed system Msftedit.dll for native RichEdit. The retained-modern examples dynamically use the public ntw16s modern-dialog API when run16s has loaded the DLL; they report its absence when run directly. Other examples use Windows APIs directly. Run the same demo.exe directly for native appearance or through run16s.exe for styling.
External observers live in tests. Native counterparts demonstrate existing
support and remaining visual gaps; they do not prove real VB compatibility.
