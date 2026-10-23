"""Read the built PE directly: architecture, exact C exports and native imports."""
import argparse
import hashlib
import json
import struct
from pathlib import Path


def inspect(path):
    raw=path.read_bytes()
    def u16(offset):return struct.unpack_from('<H',raw,offset)[0]
    def u32(offset):return struct.unpack_from('<I',raw,offset)[0]
    assert raw[:2]==b'MZ','missing MZ'
    pe=u32(0x3c)
    assert raw[pe:pe+4]==b'PE\0\0','missing PE'
    machine=u16(pe+4);count=u16(pe+6);optional=pe+24
    magic=u16(optional)
    assert magic in (0x10b,0x20b),'unsupported PE optional header'
    assert (machine,magic) in ((0x14c,0x10b),(0x8664,0x20b)),'machine/optional header mismatch'
    directories=optional+(96 if magic==0x10b else 112)
    sections=optional+u16(pe+20)
    def offset(rva):
        for index in range(count):
            section=sections+40*index
            size,base,raw_size,raw_offset=struct.unpack_from('<IIII',raw,section+8)
            if base<=rva<base+max(size,raw_size):
                result=raw_offset+rva-base
                assert result<len(raw),'RVA outside file'
                return result
        raise ValueError('unmapped RVA')
    def string(rva):
        start=offset(rva)
        return raw[start:raw.index(b'\0',start)].decode('ascii')
    export_rva=u32(directories);export_size=u32(directories+4)
    export=offset(export_rva)
    name_count=u32(export+24);name_table=offset(u32(export+32))
    names=[string(u32(name_table+4*i)) for i in range(name_count)]
    base=u32(export+16);function_count=u32(export+20)
    ordinal_table=offset(u32(export+36));functions=offset(u32(export+28))
    ordinals={name:base+u16(ordinal_table+2*i) for i,name in enumerate(names)}
    live=set()
    for index in range(function_count):
        rva=u32(functions+4*index)
        if rva:
            assert not export_rva<=rva<export_rva+export_size,'unexpected forwarded export'
            live.add(base+index)
    imports=[];rva=u32(directories+8)
    if rva:
        entry=offset(rva)
        while any(raw[entry:entry+20]):
            imports.append(string(u32(entry+12)))
            entry+=20
    return machine,names,imports,hashlib.sha256(raw).hexdigest(),ordinals,live


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--arch',choices=['x86','x64'],required=True)
    args=parser.parse_args()
    root=Path(__file__).resolve().parents[1]
    path=root/'build/.ninja'/args.arch/('ntw16s32.dll' if args.arch=='x86' else 'ntw16s64.dll')
    plan=json.loads((root/'docs/etc/import/abi-decisions.json').read_text(encoding='utf-8'))
    expected={e['symbol'] for e in plan['exports'] if e['decision']=='retain-public'}
    machine,names,imports,digest,ordinals,live=inspect(path)
    if machine!=(0x14c if args.arch=='x86' else 0x8664):raise SystemExit('PE machine mismatch')
    if len(names)!=31 or set(names)!=expected:raise SystemExit('PE exports differ from standalone core plan: '+repr(names))
    if ordinals!={name:index+2 for index,name in enumerate(sorted(expected))} or live!=set(range(1,33)):
        raise SystemExit('PE ordinal map differs from core plan; helper ordinal 1 must be NONAME')
    forbidden=('nthook','ntvdm','wow32','ntsrv','rpcrt4','vcruntime','msvcp','msvcr','ucrtbase')
    if any(any(token in name.lower() for token in forbidden) for name in imports):raise SystemExit('Forbidden external/CRT import')
    print(json.dumps(dict(status='PE PASS',architecture=args.arch,public_exports=len(names),ordinals='1 NONAME helper; 2..32 core',imports=imports,sha256=digest)))


if __name__=='__main__':main()
