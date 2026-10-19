#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Karsei
"""Build a freestanding Windows x64 EXE with GCC and GNU binutils; Python is BUILD-ONLY."""
from pathlib import Path
import subprocess,os
from elf_to_coff import convert
root=Path(__file__).resolve().parents[1];out=root/'build/desktop';out.mkdir(exist_ok=True)
exports={
'KERNEL32.dll':'GetModuleFileNameW ExpandEnvironmentStringsW GetFileAttributesW GetModuleHandleW GetProcessHeap HeapAlloc HeapFree CreateFileW ReadFile GetFileSize CloseHandle CreateThread ExitProcess Sleep',
'USER32.dll':'MessageBoxW RegisterClassW CreateWindowExW DefWindowProcW DestroyWindow GetMessageW TranslateMessage DispatchMessageW PostQuitMessage LoadIconW CreatePopupMenu DestroyMenu AppendMenuW TrackPopupMenu GetCursorPos SetForegroundWindow',
'SHELL32.dll':'ShellExecuteW Shell_NotifyIconW',
'WS2_32.dll':'WSAStartup WSACleanup socket bind listen accept getsockname closesocket setsockopt send recv shutdown'}
def run(args):subprocess.run(list(map(str,args)),check=True,cwd=root)
cc=['gcc','-Os','-mabi=ms','-fshort-wchar','-ffreestanding','-fPIC','-fno-builtin','-fno-stack-protector','-fno-asynchronous-unwind-tables','-fno-unwind-tables','-fcf-protection=none']
libs=[]
for dll,names in exports.items():
 base=dll.split('.')[0].lower();c=out/(base+'.c');d=out/(base+'.def');lib=out/(base+'.lib')
 c.write_text('\n'.join('void '+n+'(void){}' for n in names.split()))
 d.write_text('LIBRARY '+dll+'\nEXPORTS\n'+'\n'.join(names.split())+'\n')
 run(cc+['-c',c,'-o',out/(base+'.o')]);convert(out/(base+'.o'),out/(base+'.obj'))
 run(['ld','-m','i386pep','--dll','--entry','0','--out-implib',lib,'-o',out/(base+'.stub.dll'),out/(base+'.obj'),d]);libs.append(lib)
run(cc+['-Wall','-Wextra','-c',root/'desktop/red_mass_app.c','-o',out/'app.o'])
convert(out/'app.o',out/'app.obj')
exe=out/'RED-MASS-VAMPIRE-OS.exe'
run(['ld','-m','i386pep','--subsystem','windows','--entry','entry','--nxcompat','--dynamicbase','--high-entropy-va','--major-os-version','6','--minor-os-version','1','--major-subsystem-version','6','--minor-subsystem-version','1','--strip-all','-o',exe,out/'app.obj']+libs)
headers=subprocess.check_output(['objdump','-p',exe],text=True)
assert 'Windows GUI' in headers and 'PE32+' in headers and 'CLR Runtime Header' in headers
for dll in exports:assert ('DLL Name: '+dll) in headers
assert 'python' not in headers.lower() and 'mscoree.dll' not in headers.lower()
(out/'pe-imports.txt').write_text(headers)
print(f'Windows x64 app: {exe} ({exe.stat().st_size} bytes); native system DLLs only; Python/Node/.NET not required')
