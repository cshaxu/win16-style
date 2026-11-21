"""Verify the product package against source, PE and Windows version data."""
import argparse
import configparser
import ctypes
import hashlib
import importlib.util
import json
import os
import re
from pathlib import Path
import stat
import struct
import subprocess
import sys
import zlib

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


def verify_layout(package, names):
    directories = {parent.as_posix() for name in names for parent in Path(name).parents if parent != Path(".")}
    actual = set()
    for directory, subdirs, files in os.walk(package):
        for child in subdirs + files:
            path = Path(directory)/child
            relative = path.relative_to(package).as_posix()
            mode = path.lstat()
            assert not mode.st_file_attributes & stat.FILE_ATTRIBUTE_REPARSE_POINT, relative
            if child in subdirs:
                assert relative in directories, f"unexpected package directory: {relative}"
            else:
                assert stat.S_ISREG(mode.st_mode), relative
                actual.add(relative)
    assert actual == names, f"package file mismatch: missing {names-actual}, extra {actual-names}"


def verify_theme(path):
    if path.suffix == ".theme":
        descriptor = configparser.ConfigParser(interpolation=None)
        descriptor.read_string(path.read_text(encoding="ascii"))
        assert dict(descriptor["VisualStyles"]) == {
            "path": path.with_suffix(".msstyles").name, "colorstyle": "NormalColor", "size": "NormalSize"
        }, path
        assert descriptor["Theme"]["DisplayName"].strip(), path
    else:
        raw = path.read_bytes()
        assert raw[:2] == b"MZ", path
        pe = struct.unpack_from("<I", raw, 60)[0]
        assert raw[pe:pe+4] == b"PE\0\0", path
        assert struct.unpack_from("<H", raw, pe+22)[0] & 0x2000, path
        optional = pe+24
        magic = struct.unpack_from("<H", raw, optional)[0]
        assert magic in (0x10b, 0x20b), path
        directories = optional+(112 if magic == 0x20b else 96)
        assert struct.unpack_from("<I", raw, optional+16)[0] == 0, path
        exports, _, imports, _, resources, size = struct.unpack_from("<6I", raw, directories)
        assert resources and size, path
        sections = optional+struct.unpack_from("<H", raw, pe+20)[0]
        def mapped(rva, length):
            for index in range(struct.unpack_from("<H", raw, pe+6)[0]):
                _, base, size, start = struct.unpack_from("<4I", raw, sections+40*index+8)
                if base <= rva and rva-base+length <= size:
                    offset = start+rva-base
                    assert offset+length <= len(raw), path
                    return raw[offset:offset+length]
            raise AssertionError(f"unmapped theme directory: {path}")
        if exports:
            assert struct.unpack_from("<II", mapped(exports, 40), 20) == (0, 0), path
        if imports:
            assert mapped(imports, 20) == bytes(20), path
        assert "PACKTHEM_VERSION".encode("utf-16-le") in raw, path


def png_pixels(data, width, height):
    assert data[:8] == b'\x89PNG\r\n\x1a\n'
    assert struct.unpack_from('>I4sIIBBBBB', data, 8) == (13, b'IHDR', width, height, 8, 2, 0, 0, 0)
    chunks = []
    offset = 8
    while offset < len(data):
        length, kind = struct.unpack_from('>I4s', data, offset)
        end = offset+8+length
        assert end+4 <= len(data)
        checksum, = struct.unpack_from('>I', data, end)
        assert zlib.crc32(data[offset+4:end]) == checksum
        if kind == b'IDAT':
            chunks.append(data[offset+8:end])
        offset = end+4
        if kind == b'IEND':
            assert not length and offset == len(data)
            break
    else:
        raise AssertionError('PNG lacks IEND')
    stride = width*3
    size = (stride+1)*height
    inflater = zlib.decompressobj()
    decoded = inflater.decompress(b''.join(chunks), size+1)
    assert len(decoded) == size and inflater.eof and not inflater.unused_data
    last = bytearray(stride)
    pixels = bytearray()
    for offset in range(0, len(decoded), stride+1):
        mode = decoded[offset]
        assert 0 <= mode <= 4
        row = bytearray(decoded[offset+1:offset+1+stride])
        for x in range(stride):
            left = row[x-3] if x >= 3 else 0
            up = last[x]
            corner = last[x-3] if x >= 3 else 0
            predictor = 0
            if mode == 1:
                predictor = left
            elif mode == 2:
                predictor = up
            elif mode == 3:
                predictor = (left+up)//2
            elif mode == 4:
                estimate = left+up-corner
                predictor = min((left, up, corner), key=lambda value: abs(estimate-value))
            row[x] = (row[x]+predictor) & 255
        pixels.extend(row)
        last = row
    return pixels


def verify_original_theme_art(path, tables):
    # Map the PE as data. These bytes are the resources the renderer consumes.
    api = ctypes.WinDLL('kernel32', use_last_error=True)
    pointer, word = ctypes.c_void_p, ctypes.c_uint32
    functions = (
        ('LoadLibraryExW', pointer, [ctypes.c_wchar_p, pointer, word]),
        ('FindResourceW', pointer, [pointer, pointer, pointer]),
        ('LoadResource', pointer, [pointer, pointer]),
        ('SizeofResource', word, [pointer, pointer]),
        ('LockResource', pointer, [pointer]),
        ('FreeLibrary', ctypes.c_int, [pointer]),
    )
    for name, result, arguments in functions:
        function = getattr(api, name)
        function.restype, function.argtypes = result, arguments

    module = api.LoadLibraryExW(str(path), None, 0x22)
    assert module, path

    def key(value):
        if isinstance(value, int):
            return pointer(value)
        return ctypes.cast(ctypes.c_wchar_p(value), pointer)

    def resource(kind, name):
        entry = api.FindResourceW(module, key(name), key(kind))
        assert entry, f'missing {kind}:{name} in {path}'
        data = api.LoadResource(module, entry)
        length = api.SizeofResource(module, entry)
        assert data and length, name
        address = api.LockResource(data)
        assert address, name
        return ctypes.string_at(address, length)

    def image(image_id, rows, palette):
        height, width = len(rows), len(rows[0])
        expected = bytes(palette[int(role)] for row in rows for role in row for _ in range(3))
        assert png_pixels(resource('IMAGE', image_id), width, height) == expected, f'IMAGE:{image_id}'

    try:
        normal, pressed = (0, 255, 128, 192), (255, 0, 192, 128)
        frame = tables['frame_buttons']
        for index in range(6):
            image(501+index, frame[18*index:18*(index+1)], normal)
        system = tables['system_menu_bitmap']
        for index in range(2):
            rows = [row[18*index:18*(index+1)] for row in system]
            normal_id, pressed_id = ((507, 508), ('CLASSIC_CHILD_SYSTEM', 'CLASSIC_CHILD_SYSTEM_PRESSED'))[index]
            image(normal_id, rows, normal)
            image(pressed_id, rows, pressed)
        image(511, tables['menu_check'], normal)
        image(512, tables['menu_arrow'], normal)
        checks = tables['control_checks']
        for index in range(12):
            top, left = (index//4)*13, (index%4)*14
            image(513+index, [row[left:left+14] for row in checks[top:top+13]], normal)

        combo = bytes(sum((value == '0') << x for x, value in enumerate(row))
                      for row in tables['combo_arrow'])
        assert resource(10, 'CLASSIC_COMBO_ARROW') == b'CAR\x01'+combo
        arrows = resource(10, 'CLASSIC_SCROLL_ARROWS')
        assert arrows[:5] == b'SAR\x01\x01' and len(arrows) == 5+12*73
        for index, suffix in enumerate(stem+state for state in ('', 'd', 'i')
                                       for stem in ('uparrow', 'dnarrow', 'lfarrow', 'rgarrow')):
            expected = ''.join(tables['scroll_'+suffix])
            packed = arrows[5+index*73:5+(index+1)*73]
            actual = ''.join(str((packed[pixel//4] >> (2*(pixel%4))) & 3)
                             for pixel in range(17*17))
            assert actual == expected and packed[-1] <= 3, suffix
    finally:
        assert api.FreeLibrary(module)
    return 24, 13


def verify_runtime_symbols(path, tree, architecture, standalone):
    cache = (tree/'CMakeCache.txt').read_text(encoding='utf-8')
    match = re.search(r'^CMAKE_NM:FILEPATH=(.+)$', cache, re.M)
    assert match, f'missing build symbol reader for {tree}'
    output = subprocess.check_output([match[1], '--defined-only', str(path)], text=True)
    symbols = {line.split()[-1] for line in output.splitlines() if line.strip()}
    if architecture == 'x86':
        symbols = {re.sub(r'@[0-9]+$', '', symbol.removeprefix('_')) for symbol in symbols}
    # GCC can keep a retired static function as a specialized local clone.
    symbols = {symbol.split('.', 1)[0] for symbol in symbols}
    required = {'classic_theme_read', 'classic_theme_open_folder'}
    missing = required - symbols
    assert not missing, f'missing external theme loader in {path}: {sorted(missing)}'
    retired = {
        'classic_theme_open_packaged', 'legacy_common_control_profiles', 'legacy_profile',
        'open_embedded', 'open_legacy_packaged', 'open_frame_profile',
        'compatibility_dialog_policy', 'classic_sources',
        'win16_style_measure', 'win2k_caption_system_buttons', 'win2k_caption_render',
        'win16_style_tab_supported', 'win2k_tab_supported',
        'win16_style_tab_paint', 'win2k_tab_paint',
    } | standalone
    if path.suffix.lower() == '.exe':
        retired |= {'style_caption_render', 'style_caption_button_paint'}
    found = retired & symbols
    assert not found, f'retired implementation linked into {path}: {sorted(found)}'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('package', type=Path)
    parser.add_argument('--source-root', type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument('--built-root', type=Path)
    parser.add_argument('--task', type=int, required=True)
    args = parser.parse_args()
    assert not args.package.lstat().st_file_attributes & stat.FILE_ATTRIBUTE_REPARSE_POINT
    package, root = args.package.resolve(), args.source_root.resolve()
    contract = json.loads((root/'docs/etc/product-contract.json').read_text(encoding='utf-8'))
    expected = {artifact['name']: artifact['architecture']
                for component in contract['components'] for artifact in component['artifacts']}
    assert expected == {'run16s.exe': 'x64', 'ntwshk32.dll': 'x86', 'ntwshk64.dll': 'x64'}
    themes = set(contract['themes'])
    standalone = set(contract['standalone_only_exports'])
    assert themes == {f'{name}/{name}.{extension}'
                      for name in ('Win16', 'Win2K') for extension in ('theme', 'msstyles')}
    assert args.task > 0
    verify_layout(package, set(expected) | themes)
    spec = importlib.util.spec_from_file_location('core_pe', root/'tools/check_core_binary.py')
    core = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(core)
    license_bytes = (root/'LICENSE').read_bytes()
    minhook_license = (root/'src/lib-win32/process-hook/minhook/LICENSE.txt').read_bytes()
    # The original atlas is shipped through the theme resources. Provenance
    # remains in the runtime's legal notices resource.
    glyph_tables = {}
    glyph_paths = {
        'frame_buttons.h': 'src/lib-win32-style/appearance/win3x/resources/frame_buttons.h',
        'control_glyphs.h': 'src/lib-win32-style/appearance/win3x/resources/control_glyphs.h',
        'combo_glyph.h': 'src/lib-win32-style/appearance/win3x/resources/combo_glyph.h',
        'menu_glyphs.h': 'src/lib-win32-style/appearance/win3x/resources/menu_glyphs.h',
        'scroll_glyphs.h': 'src/lib-win32-style/appearance/win3x/resources/scroll_glyphs.h',
    }
    for header, source in glyph_paths.items():
        text = (root/source).read_text(encoding='utf-8')
        arrays = re.findall(r'static const char\s+(\w+)[^=]*=\s*\{(.*?)\n\};', text, re.S)
        assert arrays, 'missing resource tables: '+header
        for symbol, body in arrays:
            rows = re.findall(r'"([0-3]+)"', body)
            assert rows and symbol not in glyph_tables
            glyph_tables[symbol] = rows
    assert len(glyph_tables) == 18, 'incomplete original resource atlas'
    provenance = (root/'src/ntw16s-dll/resources.md').read_bytes()
    result = {}
    for name, arch in expected.items():
        path = package/name
        assert path.is_file() and not path.is_symlink()
        raw = path.read_bytes()
        version(path, args.task)
        assert license_bytes in raw, 'project license missing from '+name
        for symbol, data in glyph_tables.items():
            compiled = b''.join(row.encode('ascii')+b'\0' for row in data)
            assert compiled not in raw, 'original resource table linked into '+name+':'+symbol
        if name.endswith('.dll'):
            assert minhook_license in raw, 'MinHook/HDE license missing from '+name
            assert provenance in raw, 'original resource provenance missing from '+name
            machine, public, imports, digest, ordinals, live = core.inspect(path)
            pe = struct.unpack_from('<I', raw, 60)[0]
            assert struct.unpack_from('<H', raw, pe+22)[0] & 0x2000, 'DLL flag missing'
            assert machine == (0x14c if arch == 'x86' else 0x8664)
            bits = 32 if arch == 'x86' else 64
            definitions = ('ntwshk-dll/ntwshk.def', f'ntw16s-dll/ntw16s{bits}.def', 'ntw2ks-dll/ntw2ks.def')
            exports = {line.strip().split()[0].split("=", 1)[0]
                       for definition in definitions
                       for line in (root/'src'/definition).read_text().splitlines()
                       if line.startswith('    ') and 'NONAME' not in line}
            assert standalone <= exports
            exports.difference_update(standalone)
            if arch == 'x64': exports.add('process_hook_unwind_handler')
            assert set(public) == exports and live-set(ordinals.values()) == {1}
            assert {n.lower() for n in imports if not core.system_crt(n)} == {
                'activeds.dll', 'advapi32.dll', 'comctl32.dll', 'comdlg32.dll', 'dwmapi.dll',
                'gdi32.dll', 'kernel32.dll', 'msimg32.dll', 'ole32.dll', 'oleaut32.dll',
                'shcore.dll', 'shell32.dll', 'user32.dll', 'uxtheme.dll', 'winspool.drv'}
        else:
            subsystem, imports = application(path)
            assert subsystem == 3
            assert {n for n in imports if not core.system_crt(n)} == {
                'kernel32.dll', 'user32.dll', 'shell32.dll', 'ole32.dll', 'oleaut32.dll',
                'advapi32.dll', 'dwmapi.dll', 'ntwshk64.dll'}
            digest = hashlib.sha256(raw).hexdigest()
        if args.built_root:
            assert hashlib.sha256((args.built_root/arch/name).read_bytes()).hexdigest() == digest, 'mixed build'
            verify_runtime_symbols(path, args.built_root/arch, arch, standalone)
        result[name] = dict(architecture=arch, version='M0 T'+str(args.task), sha256=digest, imports=imports)
    theme_artifacts = {}
    for name in sorted(themes):
        path = package/name
        verify_theme(path)
        digest = hashlib.sha256(path.read_bytes()).hexdigest()
        if args.built_root:
            assert hashlib.sha256((args.built_root/'x64'/name).read_bytes()).hexdigest() == digest, 'mixed theme build'
        theme_artifacts[name] = digest
    images, masks = verify_original_theme_art(package/'Win16/Win16.msstyles', glyph_tables)
    print(json.dumps(dict(status='PACKAGE PASS', artifacts=result, theme_artifacts=theme_artifacts,
                          original_theme_images=images, original_theme_masks=masks,
                          original_resource_tables_per_ntwshk_dll=0, original_resource_tables_in_launcher=0), indent=2))


if __name__ == '__main__':
    main()
