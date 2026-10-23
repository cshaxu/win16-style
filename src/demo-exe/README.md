# Native demo

One x64 Windows GUI application, adapted from the frozen W16Style entry and
the native menu/control examples. It presents standard STATIC, BUTTON, EDIT,
LISTBOX, COMBOBOX and SCROLLBAR windows with normal application notifications.
File/Reset restores sample data; Apply displays the entered text; Help/About
opens a standard message box. Windows owns rendering, menus, focus and input.

There is no style ABI, DLL loading, appearance switch, test counter or capture
code. Run this same demo.exe directly for native Windows appearance, or through
run16s.exe for the injected appearance. External observers live in tests.
The original gallery's explicit installation, private font, owner-drawn menu,
palette controls and 28-page controller are intentionally omitted.
