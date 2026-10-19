#!/usr/bin/env python3
"""Recover the three hash-checked SDK blobs from our released FM1 package."""
from pathlib import Path
import sys, struct, hashlib
import fm1pkg_make as pkg
from build import SDK_SHA256

src, dst = Path(sys.argv[1]), Path(sys.argv[2]) / 'cpu/wl82/tools'
wire = src.read_bytes()
logical = bytearray(b''.join(wire[i*48:i*48+47] for i in range(20)) + wire[960:])
entry = bytearray(logical[0x40:0x90]); pkg.enc(entry, 0, len(entry))
off, size = struct.unpack_from('<II', entry, 8)
flash = bytearray(logical[off:off+size])
head = bytearray(flash[:160])
for i in range(0, len(head), 32): pkg.enc(head, i, 32)
spl_off, spl_size = struct.unpack_from('<II', head, 36)
region = bytearray(flash[0x4000:0x93000]); pkg.sfc(region, 0, len(region), 0, 0x980F)
block = struct.unpack_from('<I', region, 8)[0]
cfg_off, cfg_size = struct.unpack_from('<II', region, 68)
eq_off, eq_size = struct.unpack_from('<II', region, block+36)
blobs = {'uboot.boot':flash[spl_off:spl_off+spl_size],
         'cfg_tool.bin':region[cfg_off:cfg_off+cfg_size],
         'cfg/eq_cfg_hw.bin':region[block+eq_off:block+eq_off+eq_size]}
for name, data in blobs.items():
    digest = hashlib.sha256(data).hexdigest()
    if digest != SDK_SHA256[name]: raise SystemExit('SDK hash mismatch: '+name)
    p = dst/name; p.parent.mkdir(parents=True, exist_ok=True); p.write_bytes(data)
    print(name, len(data), digest)
