[bits 64]
default rel

section .data

section .text
	global xoshiro256ss_filln_avx2

xoshiro256ss_filln_avx2:
	mov	rax, rdx
	test	rdx, rdx
	jz	.rt

	vmovdqa ymm0, [rdi]
	vmovdqa ymm1, 32[rdi]
	vmovdqa ymm2, 64[rdi]
	vmovdqa ymm3, 96[rdi]

.l1:	; compute the result
	vpsllq	ymm4, ymm1, 2
	vpaddq	ymm4, ymm1
	vpsrlq	ymm5, ymm4, 57
	vpsllq	ymm4, 7
	vpor	ymm4, ymm5
	vpsllq	ymm5, ymm4, 3
	vpaddq	ymm4, ymm5
	vmovdqa [rsi], ymm4
	; update the state
	vpsllq	ymm4, ymm1, 17
	vpxor	ymm2, ymm0
	vpxor	ymm3, ymm1
	vpxor	ymm1, ymm2
	vpxor	ymm0, ymm3
	vpxor	ymm2, ymm4
	vpsrlq	ymm4, ymm3, 19
	vpsllq	ymm3, 45
	vpor	ymm3, ymm4
	; rinse and repeat
	add	rsi, 0x20	
	dec	rdx
	jnz	.l1

	vmovdqa [rdi], ymm0
	vmovdqa 32[rdi], ymm1
	vmovdqa 64[rdi], ymm2
	vmovdqa 96[rdi], ymm3
	vzeroupper
.rt:	ret

