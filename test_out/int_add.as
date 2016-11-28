%include "io.inc"

section .data
__fl_neg:	dd	2147483648

section .bss
_t:	resd	1
_a:	resd	1
_b:	resd	1

section .text
GLOBAL CMAIN
CMAIN:
jmp f_main

f_add: 
push	ebp
mov	ebp, esp
mov	eax, [_a]
push	eax
mov	eax, [_b]
pop	ebx
add	eax, ebx
mov	ebx, [ebp+8]
mov	[_b], ebx
mov	ebx, [ebp+8]
mov	[_a], ebx
mov	esp, ebp
pop	ebp
ret

f_main: 
push	ebp
mov	ebp, esp
mov	eax, 3
push	dword [_a]
mov	[_a], eax
mov	eax, 4
push	dword [_b]
mov	[_b], eax
call	f_add
add	esp, 8
mov	[_t], eax
mov	eax, 0
PRINT_HEX	4, _t
PRINT_CHAR	13
PRINT_CHAR	10
PRINT_HEX	4, _a
PRINT_CHAR	13
PRINT_CHAR	10
PRINT_HEX	4, _b
PRINT_CHAR	13
PRINT_CHAR	10
mov	esp, ebp
pop	ebp
ret
