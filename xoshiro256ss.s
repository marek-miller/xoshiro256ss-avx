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
	vmovdqa ymm4, 32[rdi]
	vmovdqa ymm1, 64[rdi]
	vmovdqa ymm5, 96[rdi]
	vmovdqa ymm2, 128[rdi]
	vmovdqa ymm6, 160[rdi]
	vmovdqa ymm3, 192[rdi]
	vmovdqa ymm7, 224[rdi]

.l1:	; compute the result
	vpsllq	ymm8, ymm1, 2
	vpaddq	ymm8, ymm1
	vpsrlq	ymm9, ymm8, 57
	vpsllq	ymm8, 7
	vpor	ymm8, ymm9
	vpsllq	ymm9, ymm8, 3
	vpaddq	ymm8, ymm9
	vmovdqa [rsi], ymm8

	vpsllq	ymm10, ymm5, 2
	vpaddq	ymm10, ymm5
	vpsrlq	ymm11, ymm10, 57
	vpsllq	ymm10, 7
	vpor	ymm10, ymm11
	vpsllq	ymm11, ymm10, 3
	vpaddq	ymm10, ymm11
	vmovdqa [rsi + 0x20], ymm10

	; update the state
	vpsllq	ymm8, ymm1, 17
	vpxor	ymm2, ymm0
	vpxor	ymm3, ymm1
	vpxor	ymm1, ymm2
	vpxor	ymm0, ymm3
	vpxor	ymm2, ymm8
	vpsrlq	ymm8, ymm3, 19
	vpsllq	ymm3, 45
	vpor	ymm3, ymm8

	vpsllq	ymm10, ymm5, 17
	vpxor	ymm6, ymm4
	vpxor	ymm7, ymm5
	vpxor	ymm5, ymm6
	vpxor	ymm4, ymm7
	vpxor	ymm6, ymm10
	vpsrlq	ymm10, ymm7, 19
	vpsllq	ymm7, 45
	vpor	ymm7, ymm10

	; rinse and repeat
	add	rsi, 0x40	
	dec	rdx
	jnz	.l1

	vmovdqa [rdi], ymm0
	vmovdqa 32[rdi], ymm4
	vmovdqa 64[rdi], ymm1
	vmovdqa 96[rdi], ymm5
	vmovdqa 128[rdi], ymm2
	vmovdqa 160[rdi], ymm6
	vmovdqa 192[rdi], ymm3
	vmovdqa 224[rdi], ymm7
	vzeroupper
.rt:	ret

