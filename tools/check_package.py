"""Verify a four-file package against the source, PE and Windows version data."""
import argparse
import ctypes
import hashlib
import importlib.util
import json
import re
from pathlib import Path
import struct
import sys

sys.dont_write_bytecode = True


def application(path):
    raw = path.read_bytes()
    u16 = lambda at: struct.unpack_from('<H', raw, at)[0]
    u32 = lambda at: struct.unpack_from('<I', raw, at)[0]
    assert raw[:2] == b'MZ'
    pe = u32(60)
    assert raw[pe:pe+4] == b'PE\0\0' and u16(pe+4) == 0x8664
    opt = pe+24
    assert u16(opt) == 0x20b and not u16(pe+22) & 0x2000
    sections = opt+u16(pe+20)

    def offset(rva):
        for index in range(u16(pe+6)):
            size, base, length, start = struct.unpack_from('<IIII', raw, sections+40*index+8)
            if base <= rva < base+max(size, length):
                return start+rva-base
        raise AssertionError('unmapped RVA')

    assert not u32(opt+112) and not u32(opt+112+13*8), 'unexpected export/delay import'
    imports = []
    at = offset(u32(opt+112+8))
    while any(raw[at:at+20]):
        start = offset(u32(at+12))
        imports.append(raw[start:raw.index(b'\0', start)].decode('ascii').lower())
        at += 20
    return u16(opt+68), imports


def version(path, task):
    api = ctypes.WinDLL('version', use_last_error=True)
    api.GetFileVersionInfoSizeW.argtypes = [ctypes.c_wchar_p, ctypes.POINTER(ctypes.c_uint32)]
    api.GetFileVersionInfoSizeW.restype = ctypes.c_uint32
    api.GetFileVersionInfoW.argtypes = [ctypes.c_wchar_p, ctypes.c_uint32, ctypes.c_uint32, ctypes.c_void_p]
    api.VerQueryValueW.argtypes = [ctypes.c_void_p, ctypes.c_wchar_p, ctypes.POINTER(ctypes.c_void_p), ctypes.POINTER(ctypes.c_uint32)]
    ignored = ctypes.c_uint32()
    size = api.GetFileVersionInfoSizeW(str(path), ctypes.byref(ignored))
    assert size
    data = ctypes.create_string_buffer(size)
    assert api.GetFileVersionInfoW(str(path), 0, size, data)
    value, length = ctypes.c_void_p(), ctypes.c_uint32()
    assert api.VerQueryValueW(data, '\\', ctypes.byref(value), ctypes.byref(length)) and length.value >= 52
    fixed = ctypes.cast(value, ctypes.POINTER(ctypes.c_uint32))
    assert fixed[0] == 0xfeef04bd and (fixed[2], fixed[3], fixed[4], fixed[5]) == (task, 0, task, 0)
    assert api.VerQueryValueW(data, '\\StringFileInfo\\040904b0\\FileVersion', ctypes.byref(value), ctypes.byref(length))
    assert ctypes.wstring_at(value) == 'M0 T'+str(task)
    assert api.VerQueryValueW(data, '\\StringFileInfo\\040904b0\\ProductVersion', ctypes.byref(value), ctypes.byref(length))
    assert ctypes.wstring_at(value) == '0.'+str(task)+'.0.0'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('package', type=Path)
    parser.add_argument('--source-root', type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument('--built-root', type=Path)
    parser.add_argument('--task', type=int, required=True)
    args = parser.parse_args()
    package, root = args.package.resolve(), args.source_root.resolve()
    expected = {'run16s.exe': 'x64', 'ntw16s32.dll': 'x86', 'ntw16s64.dll': 'x64', 'demo.exe': 'x64'}
    assert args.task > 0 and {p.name for p in package.iterdir()} == set(expected), 'package must contain exactly four files'
    spec = importlib.util.spec_from_file_location('core_pe', root/'tools/check_core_binary.py')
    core = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(core)
    abi = json.loads((root/'docs/etc/import/abi-decisions.json').read_text(encoding='utf-8'))
    names = sorted({e['symbol'] for e in abi['exports'] if e['decision'] == 'retain-public'})
    license_bytes = (root/'LICENSE').read_bytes()
    minhook_license = (root/'src/ntw16s-dll/injection/minhook/LICENSE.txt').read_bytes()
    # Independently read the retained C atlas rows and require those exact
    # compiled tables, plus provenance, in both DLLs. No visual substitute.
    glyph_tables = {}
    for header in ('frame_buttons.h', 'control_glyphs.h', 'combo_glyph.h', 'menu_glyphs.h', 'scroll_glyphs.h'):
        text = (root/'src/ntw16s-dll'/header).read_text(encoding='utf-8')
        arrays = re.findall(r'static const char\s+(\w+)[^=]*=\s*\{(.*?)\n\};', text, re.S)
        assert arrays, 'missing resource tables: '+header
        for symbol, body in arrays:
            rows = re.findall(r'"([0-3]+)"', body)
            assert rows and symbol not in glyph_tables
            glyph_tables[symbol] = b''.join(row.encode('ascii')+b'\0' for row in rows)
    assert len(glyph_tables) == 18, 'incomplete original resource atlas'
    provenance = (root/'src/ntw16s-dll/resources.md').read_bytes()
    result = {}
    for name, arch in expected.items():
        path = package/name
        assert path.is_file() and not path.is_symlink()
        raw = path.read_bytes()
        version(path, args.task)
        assert license_bytes in raw, 'project license missing from '+name
        if name.endswith('.dll'):
            assert minhook_license in raw, 'MinHook/HDE license missing from '+name
            assert provenance in raw, 'original resource provenance missing from '+name
            for symbol, data in glyph_tables.items():
                assert data in raw, 'original resource table missing/changed: '+name+':'+symbol
            machine, public, imports, digest, ordinals, live = core.inspect(path)
            pe = struct.unpack_from('<I', raw, 60)[0]
            assert struct.unpack_from('<H', raw, pe+22)[0] & 0x2000, 'DLL flag missing'
            assert machine == (0x14c if arch == 'x86' else 0x8664)
            assert public == names and ordinals == {n: i+2 for i, n in enumerate(names)} and live == set(range(1, 33))
            assert {n.lower() for n in imports if not core.system_crt(n)} == {'kernel32.dll', 'user32.dll', 'gdi32.dll', 'comctl32.dll', 'dwmapi.dll', 'uxtheme.dll'}
        else:
            subsystem, imports = application(path)
            assert subsystem == (2 if name == 'demo.exe' else 3)
            assert {n for n in imports if not core.system_crt(n)} == ({'kernel32.dll', 'user32.dll', 'gdi32.dll', 'comctl32.dll', 'comdlg32.dll'} if name == 'demo.exe' else {'kernel32.dll', 'shell32.dll', 'ole32.dll', 'advapi32.dll'})
            digest = hashlib.sha256(raw).hexdigest()
        if args.built_root:
            assert hashlib.sha256((args.built_root/arch/name).read_bytes()).hexdigest() == digest, 'mixed build'
        result[name] = dict(architecture=arch, version='M0 T'+str(args.task), sha256=digest, imports=imports)
    print(json.dumps(dict(status='PACKAGE PASS', artifacts=result, original_resource_tables_per_dll=len(glyph_tables)), indent=2))


if __name__ == '__main__':
    main()
