"""Run real modal file and folder calls using independent packed policy fields."""
import ctypes
from ctypes import wintypes
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import tempfile


def entry(name, result, *arguments):
    function = getattr(kernel, name)
    function.argtypes = arguments
    function.restype = result
    return function


kernel = ctypes.WinDLL("kernel32", use_last_error=True)
load = entry("LoadLibraryExW", wintypes.HMODULE,
             wintypes.LPCWSTR, wintypes.HANDLE, wintypes.DWORD)
find = entry("FindResourceW", wintypes.HANDLE,
             wintypes.HMODULE, wintypes.LPCWSTR, ctypes.c_void_p)
resource = entry("LoadResource", wintypes.HANDLE, wintypes.HMODULE, wintypes.HANDLE)
size = entry("SizeofResource", wintypes.DWORD, wintypes.HMODULE, wintypes.HANDLE)
lock = entry("LockResource", ctypes.c_void_p, wintypes.HANDLE)
free = entry("FreeLibrary", wintypes.BOOL, wintypes.HMODULE)
begin = entry("BeginUpdateResourceW", wintypes.HANDLE, wintypes.LPCWSTR, wintypes.BOOL)
update = entry("UpdateResourceW", wintypes.BOOL, wintypes.HANDLE, ctypes.c_void_p,
               wintypes.LPCWSTR, wintypes.WORD, ctypes.c_void_p, wintypes.DWORD)
end = entry("EndUpdateResourceW", wintypes.BOOL, wintypes.HANDLE, wintypes.BOOL)


def checked(value):
    if not value:
        raise ctypes.WinError(ctypes.get_last_error())
    return value


def set_policy(path, declaration):
    module = checked(load(str(path), None, 0x22))
    try:
        handle = checked(find(module, "CLASSIC_STYLE", 10))
        data = bytearray(ctypes.string_at(
            checked(lock(checked(resource(module, handle)))), checked(size(module, handle))))
    finally:
        checked(free(module))
    assert data[:4] == b"CSTY"
    cursor = 16
    for _ in range(struct.unpack_from("<I", data, 12)[0]):
        kind, flags, length = struct.unpack_from("<HHI", data, cursor)
        cursor += 8
        if kind == 9:
            assert flags == 1 and length == len(declaration)
            data[cursor:cursor + length] = declaration
            break
        cursor += length
    else:
        raise AssertionError("The built theme lacks a dialog policy")
    transaction = checked(begin(str(path), False))
    buffer = ctypes.create_string_buffer(bytes(data))
    written = update(transaction, 10, "CLASSIC_STYLE", 0, buffer, len(data))
    error = ctypes.get_last_error()
    checked(end(transaction, not written))
    if not written:
        raise ctypes.WinError(error)


def verify(package, driver, targets, print_targets):
    settings = (
        ("Mixed Classic", "Win16", bytes((1, 2, 0, 0)),
         (("open", "explorer", "COM_EXPLORER_LAYOUT bridged=1"),
          ("folder", "native", "COM_FOLDER_LAYOUT tree=0"))),
        ("Mixed 2000", "Win2K", bytes((2, 1, 1, 1)),
         (("open", None, "old=1"), ("folder", None, "COM_FOLDER_LAYOUT tree=1"))),
        ("Native Files", "Win2K", bytes((2, 0, 1, 2)),
         (("open", "native", "old=0"), ("folder", None, "COM_FOLDER_LAYOUT tree=1"))),
    )
    with tempfile.TemporaryDirectory(prefix="style-dialog-policy-") as temporary:
        root = Path(temporary)
        for binary in ("run16s.exe", "ntwshk32.dll", "ntwshk64.dll"):
            shutil.copyfile(package / binary, root / binary)
        for stock in ("Win16", "Win2K"):
            shutil.copytree(package / stock, root / stock)
        for folder, stock, declaration, checks in settings:
            shutil.copytree(package / stock, root / folder)
            set_policy(root / folder / f"{stock}.msstyles", declaration)
            for bits, target, printer in zip((32, 64), targets, print_targets):
                for operation, option, marker in checks:
                    command = [driver, "--direct", root / "run16s.exe", folder, target,
                               operation, "cancel"]
                    if option:
                        command.append(option)
                    result = subprocess.run(command, capture_output=True, text=True,
                                            errors="replace", timeout=16)
                    if result.returncode or marker not in result.stdout or "hr=800704c7" not in result.stdout:
                        raise AssertionError((folder, bits, operation, result.returncode,
                                              result.stdout, result.stderr))
                    print(f"DIALOG POLICY bits={bits} theme={folder} operation={operation} {marker}")
                expected = declaration[3]
                arguments, markers = {
                    0: (("--print-ex-default",), ("settings=1 dc=0 pass=1",)),
                    1: (("--print-ex-policy", "1"), ("expected=1 observed=1",)),
                    2: (("--print-ex-frame",), ("attached=1 result=00000000 action=0",)),
                }[expected]
                result = subprocess.run(
                    [driver, "--direct", root / "run16s.exe", folder, printer, *arguments],
                    capture_output=True, text=True, errors="replace", timeout=16)
                if result.returncode == 77 and "SKIP no_default_printer" in result.stdout:
                    print(f"DIALOG POLICY PRINT SKIP bits={bits} theme={folder} no_default_printer")
                    continue
                if result.returncode or not all(marker in result.stdout for marker in markers):
                    raise AssertionError((folder, bits, "print", result.returncode,
                                          result.stdout, result.stderr))
                print(f"DIALOG POLICY PRINT bits={bits} theme={folder} mode={expected} ANSI and Unicode")
    print("DIALOG POLICY PASS")


if __name__ == "__main__":
    package, driver, x86, x64, print_x86, print_x64 = (
        Path(arg).resolve() for arg in sys.argv[1:])
    verify(package, driver, (x86, x64), (print_x86, print_x64))
