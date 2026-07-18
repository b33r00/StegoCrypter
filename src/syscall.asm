; syscall.asm – x64 syscall stubok (NASM)
; Fordításhoz: nasm -f win64 syscall.asm

section .text
global syscall_NtAllocateVirtualMemory
global syscall_NtProtectVirtualMemory
global syscall_NtCreateThreadEx
global syscall_NtWaitForSingleObject

syscall_NtAllocateVirtualMemory:
    mov r10, rcx
    mov eax, [rel NtAllocateVirtualMemory_SSN]
    syscall
    ret

syscall_NtProtectVirtualMemory:
    mov r10, rcx
    mov eax, [rel NtProtectVirtualMemory_SSN]
    syscall
    ret

syscall_NtCreateThreadEx:
    mov r10, rcx
    mov eax, [rel NtCreateThreadEx_SSN]
    syscall
    ret

syscall_NtWaitForSingleObject:
    mov r10, rcx
    mov eax, [rel NtWaitForSingleObject_SSN]
    syscall
    ret

section .data
NtAllocateVirtualMemory_SSN dd 0
NtProtectVirtualMemory_SSN dd 0
NtCreateThreadEx_SSN dd 0
NtWaitForSingleObject_SSN dd 0
