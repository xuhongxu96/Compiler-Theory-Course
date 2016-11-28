%include "io.inc"

section .data
__fl_neg:	dd	2147483648

section .bss
_r:	resd	1
_n:	resd	1

section .text
GLOBAL CMAIN
CMAIN:
jmp f_main

f_fib: 
push	ebp
mov	ebp, esp
mov	eax, [_n]
push	eax
mov	eax, 2
pop	ebx
cmp	ebx, eax
jle	__j_0
jmp	__nj_0
__j_0:
mov	eax, 1
jmp __endj_0
__nj_0:
mov	eax, 0
__endj_0:
cmp	eax, 0
je	__ifend_0
mov	eax, 1
mov	ebx, [ebp+8]
mov	[_n], ebx
mov	esp, ebp
pop	ebp
ret
__ifend_0:
mov	eax, 1
push	eax
mov	eax, [_n]
pop	ebx
sub	eax, ebx
push	dword [_n]
mov	[_n], eax
call	f_fib
add	esp, 4
push	eax
mov	eax, 2
push	eax
mov	eax, [_n]
pop	ebx
sub	eax, ebx
push	dword [_n]
mov	[_n], eax
call	f_fib
add	esp, 4
pop	ebx
add	eax, ebx
mov	ebx, [ebp+8]
mov	[_n], ebx
mov	esp, ebp
pop	ebp
ret

f_main: 
push	ebp
mov	ebp, esp
mov	eax, 10
push	dword [_n]
mov	[_n], eax
call	f_fib
add	esp, 4
mov	[_r], eax
mov	eax, 0
PRINT_HEX	4, _r
PRINT_CHAR	13
PRINT_CHAR	10
PRINT_HEX	4, _n
PRINT_CHAR	13
PRINT_CHAR	10
mov	esp, ebp
pop	ebp
ret
