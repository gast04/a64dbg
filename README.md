# a64dbg

Native debugger on arm64.

```
coral:/data/local/tmp # ./a64dbg  /data/local/tmp/target_bin
[*] Executable path: /data/local/tmp/target_bin
[*] Debugger Started, tracee pid: 6498
[+] (6498) pc: 0x7fbf60c070 sp: 0x7ffffff320 instr: 0x910003e0
a64> b 0x555555c4f0
[+] Set Breakpoint 0x555555c4f0: 0xaa0103e2f81f0ffe
a64> c
Tracee Stop Signal: 5
[!] Breakpoint Hit at 0x555555c4f0
[+] Restore at 0x000000555555c4f0 - 0xaa0103e2f81f0ffe
[+] (6498) pc: 0x555555c4f0 sp: 0x7ffffff2c0 instr: 0xf81f0ffe
a64> readm 0x555555c4f0 25
000000555555c4f0: f81f0ffe aa0103e2 b00001e8 f947b108 39400108
000000555555c504: 2a0003e8 93407d01 90000000 910f6000 9400010d
000000555555c518: f84107fe d65f03c0 d100c3ff f90013fe aa0003e8
000000555555c52c: f9000be8 f9000fe1 97fffe90 f90003e0 f90007e1
000000555555c540: f94007e1 f94003e0 f94013fe 9100c3ff d65f03c0
```