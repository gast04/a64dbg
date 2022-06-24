/*
    Syscall numbers sources:
    /apex/com.android.runtime/lib64/bionic/libc.so
    /apex/com.android.runtime/bin/linker64
    https://github.com/torvalds/linux/blob/v4.17/include/uapi/asm-generic/unistd.h
*/

#define MAX_SYSCALL_NUM 168
#define SYSCALL_MAXARGS 6

enum argtype {
    ARG_INT,
    ARG_PTR,
    ARG_STR
};

struct syscall_entry {
    const char *name;
    int nargs;
    enum argtype args[SYSCALL_MAXARGS];
};

struct syscall_entry aarch64_syscalls[] = {
    [51] = {
        .name  = "chroot",
        .nargs = 1,
        .args  = {}},
    [56] = {
        .name  = "open",
        .nargs = 1,
        .args  = {}},
    [73] = {
        .name  = "ppoll",
        .nargs = 1,
        .args  = {}},
    [117] = {
        .name  = "ptrace",
        .nargs = 1,
        .args  = {}},
    [134] = {
        .name  = "sigaction",
        .nargs = 1,
        .args  = {}},
    [167] = {
        .name  = "prctl",
        .nargs = 1,
        .args  = {}},
};