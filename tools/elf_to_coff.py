#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Karsei
"""Convert freestanding x86-64 ELF PIC objects to AMD64 COFF.

Strict supported relocation set; no instruction rewriting, runtime or imports.
GNU objcopy does not translate ELF section-symbol relocations correctly.
"""
import struct
from pathlib import Path


def convert(source, destination):
    raw = Path(source).read_bytes()
    assert raw[:6] == b'\x7fELF\x02\x01' and struct.unpack_from('<H',raw,18)[0] == 62
    shoff = struct.unpack_from('<Q',raw,40)[0]
    entsize, count, strings_index = struct.unpack_from('<HHH',raw,58)
    assert entsize == 64
    sections = [struct.unpack_from('<IIQQQQIIQQ',raw,shoff+i*64) for i in range(count)]
    def data(section):
        return raw[section[4]:section[4]+section[5]]
    names = data(sections[strings_index])
    def string(table, offset):
        return table[offset:table.index(0,offset)].decode('utf-8')
    selected = [i for i,s in enumerate(sections) if s[2]&2 and s[1] in (1,8)]
    indices = {original:i+1 for i,original in enumerate(selected)}
    sym_index = next(i for i,s in enumerate(sections) if s[1]==2)
    symtab = sections[sym_index]
    syms = [struct.unpack_from('<IBBHQQ',raw,symtab[4]+i*24) for i in range(symtab[5]//24)]
    strtab = data(sections[symtab[6]])
    strings = bytearray(b'\0\0\0\0')
    def coff_name(name, section=False):
        value = name.encode('utf-8')
        if len(value)<=8:
            return value.ljust(8,b'\0')
        offset = len(strings);strings.extend(value+b'\0')
        return ('/'+str(offset)).encode().ljust(8,b'\0') if section else struct.pack('<II',0,offset)
    symbols = bytearray();sym_map={}
    for i,s in enumerate(syms):
        name,info,other,section,value,size=s
        if section not in indices and section not in (0,0xfff1):
            continue
        if info&15==4:  # STT_FILE
            continue
        text = string(strtab,name) if name else string(names,sections[section][0]) if section in indices else ''
        sym_map[i]=len(symbols)//18
        symbols.extend(coff_name(text)+struct.pack('<IhHBB',value,indices.get(section,-1 if section==0xfff1 else 0),
                      0x20 if info&15==2 else 0,2 if info>>4 else 3,0))
    payload=bytearray();headers=[];offset=20+40*len(selected)
    for original in selected:
        sec=sections[original];name=string(names,sec[0]);bytes_=bytearray(data(sec)) if sec[1]!=8 else bytearray()
        relocations=bytearray()
        for rela in sections:
            if rela[1]!=4 or rela[7]!=original:
                continue
            assert rela[6]==sym_index
            for off in range(rela[4],rela[4]+rela[5],24):
                address,info,addend=struct.unpack_from('<QQq',raw,off)
                kind=info&0xffffffff;symbol=info>>32
                assert symbol in sym_map
                if kind in (2,4):  # PC32 / PLT32; COFF REL32 is relative to field end.
                    coff_kind,width,value=4,4,addend+4
                elif kind==1:
                    coff_kind,width,value=1,8,addend
                elif kind in (10,11):
                    raise ValueError('Absolute 32-bit relocation is unsafe with Windows x64 ASLR. Compile with -fPIC.')
                else:
                    raise ValueError(f'Unsupported ELF relocation {kind} in {name}.')
                assert address+width<=len(bytes_)
                bytes_[address:address+width]=(value&((1<<(width*8))-1)).to_bytes(width,'little')
                relocations.extend(struct.pack('<IIH',address,sym_map[symbol],coff_kind))
        assert len(relocations)//10<65536
        raw_at=offset+len(payload) if bytes_ else 0
        payload.extend(bytes_)
        reloc_at=offset+len(payload) if relocations else 0
        payload.extend(relocations)
        flags=0x60000020 if sec[2]&4 else 0xc0000080 if sec[1]==8 else 0xc0000040 if sec[2]&1 else 0x40000040
        headers.append(coff_name(name,True)+struct.pack('<IIIIIIHHI',0,0,sec[5],raw_at,reloc_at,0,len(relocations)//10,0,flags))
    struct.pack_into('<I',strings,0,len(strings))
    header=struct.pack('<HHIIIHH',0x8664,len(selected),0,offset+len(payload),len(symbols)//18,0,0)
    Path(destination).write_bytes(header+b''.join(headers)+payload+symbols+strings)
