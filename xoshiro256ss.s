[bits 64]
default rel

section .data

section .text
	global xoshiro256ss_filln

xoshiro256ss_filln:
	mov	rax, rdx
	test	rdx, rdx
	jz	.rt

	vmovdqa ymm0, [rdi]
	vmovdqa ymm1, 32[rdi]
	vmovdqa ymm2, 64[rdi]
	vmovdqa ymm3, 96[rdi]

.l1:	; compute the result
	vmovdqa ymm4, ymm1
	vpsllq	ymm4, ymm4, 2
	vpaddq	ymm4, ymm4, ymm1
	vmovdqa ymm5, ymm4
	vpsllq	ymm4, ymm4, 7
	vpsrlq	ymm5, ymm5, 57
	vpor	ymm4, ymm4, ymm5
	vmovdqa	ymm5, ymm4
	vpsllq	ymm4, ymm4, 3
	vpaddq	ymm4, ymm4, ymm5
	vmovdqa [rsi], ymm4
	; update state
	vmovdqa ymm4, ymm1
	vpsllq	ymm4, 17
	vpxor	ymm2, ymm2, ymm0
	vpxor	ymm3, ymm3, ymm1
	vpxor	ymm1, ymm1, ymm2
	vpxor	ymm0, ymm0, ymm3
	vpxor	ymm2, ymm2, ymm4
	vmovdqa	ymm4, ymm3
	vpsllq	ymm3, ymm3, 45
	vpsrlq	ymm4, ymm4, 19
	vpor	ymm3, ymm3, ymm4
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

