# how it should work

https://github.com/nelhage/ministrace
https://github.com/nafeabd/Ptrace-for-Android

# register fetching works different on aarch64
https://stackoverflow.com/questions/39597719/tracing-a-process-using-ptrace-on-android-arm64

NOTE:
there is no PTRACE_SETREGS and PTRACE_GETREGS it uses
PTRACE_GETREGSET and PTRACE_SETREGSET

ARM however uses the other ones...

TODO:
* memory read using /proc/child/mem
* memory maps read  /proc/child/maps
* check on https://source.android.com/devices/tech/debug/ftrace
* https://eklitzke.org/ptrace
* attach vs startup (probably attach only needed)
* how to set a memory readion with no permissions in the child space?

# what about using clone instead of fork?
```
If this is Linux (which the tags suggest it is), you can share the
entirety of the child's address space with the parent by using clone()
with the CLONE_VM flag. As the two processes share the same VM space, all
modifications will be immediately visible between the two, with essentially
zero overhead.
https://stackoverflow.com/questions/958369/low-overhead-way-to-access-the-memory-space-of-a-traced-process
```

# Resources
* https://web.archive.org/web/20170628183438/http://www.advogato.org/person/StephanPeijnik/diary.html?start=26


