%include "io.inc"

section .data
__fl_neg:	dd	2147483648
__fl_0:	dd	3.300000

section .bss
_t:	resd	1
_x:	resd	1
_a:	resd	1
_b:	resd	1

section .text
GLOBAL CMAIN
CMAIN:
jmp f_main

f_add: 
push	ebp
mov	ebp, esp
movss	xmm0, [_a]
lea	esp, [esp-4]
movss	[esp], xmm0
movss	xmm0, [_b]
movss	xmm1, [esp]
lea	esp, [esp+4]
addss	xmm0, xmm1
movss	xmm1, [ebp+12]
movss	[_b], xmm1
movss	xmm1, [ebp+8]
movss	[_a], xmm1
mov	esp, ebp
pop	ebp
ret

f_main: 
push	ebp
mov	ebp, esp
movss	xmm0, [__fl_0]
movss	[_x], xmm0
movss	xmm0, [_x]
lea	esp, [esp-4]
movss	xmm1, [_a]
movss	[esp], xmm1
movss	[_a], xmm0
movss	xmm0, [_x]
lea	esp, [esp-4]
movss	xmm1, [_b]
movss	[esp], xmm1
movss	[_b], xmm0
call	f_add
add	esp, 8
movss	[_t], xmm0
mov	eax, 0
PRINT_HEX	4, _t
PRINT_CHAR	13
PRINT_CHAR	10
PRINT_HEX	4, _x
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
