"""Check the actual demo PE and the source's native-only boundary."""
from pathlib import Path
import hashlib
import re
import struct
import sys

image = Path(sys.argv[1])
raw = image.read_bytes()
u16 = lambda offset: struct.unpack_from('<H', raw, offset)[0]
u32 = lambda offset: struct.unpack_from('<I', raw, offset)[0]
assert raw[:2] == b'MZ'
pe = u32(60)
assert raw[pe:pe+4] == b'PE\0\0' and u16(pe+4) == 0x8664
optional = pe+24
assert u16(optional) == 0x20b and u16(optional+68) == 2, 'x64 GUI required'
assert not u16(pe+22) & 0x2000, 'demo must be an application'
sections = optional+u16(pe+20)


def offset(rva):
    for index in range(u16(pe+6)):
        size, base, length, start = struct.unpack_from('<IIII', raw, sections+40*index+8)
        if base <= rva < base+max(size, length):
            result = start+rva-base
            assert result < len(raw)
            return result
    raise AssertionError('Unmapped RVA')


def name(rva):
    start = offset(rva)
    return raw[start:raw.index(b'\0', start)].decode('ascii')


assert not u32(optional+112), 'demo must not export component ABI'
assert not u32(optional+112+13*8), 'unexpected delay-loaded dependency'
entry = offset(u32(optional+112+8))
imports = []
while any(raw[entry:entry+20]):
    imports.append(name(u32(entry+12)).lower())
    entry += 20
def system_crt(name):
    return name in {'msvcrt.dll','ucrtbase.dll'} or re.fullmatch(
        r'api-ms-win-crt-(conio|convert|environment|filesystem|heap|locale|math|multibyte|private|process|runtime|stdio|string|time|utility)-l1-1-0\.dll',name) is not None


assert {name for name in imports if not system_crt(name)} == {'kernel32.dll', 'user32.dll'}, imports
root = Path(__file__).resolve().parents[2]
sources = root/'src/demo-exe'
assert {p.name for p in sources.glob('*.c')} == {'main.c', 'gallery.c'}
for file in sources.glob('*.c'):
    text = file.read_text(encoding='utf-8')
    assert not re.search(r'Ntw16s|win16_style_|LoadLibrary|GetProcAddress|GetEnvironmentVariable|GetCommandLine', text), file
    assert not re.search(r'#\s*include\s*"', text), 'demo cannot include private component headers'
print('DEMO BOUNDARY PASS x64 GUI, Windows USER32/KERNEL32/system CRT only, no exports/delay imports or source style consumption; sha256='+hashlib.sha256(raw).hexdigest())
