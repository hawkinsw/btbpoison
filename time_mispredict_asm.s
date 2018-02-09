bits 64

section .data
align 64
predicted_format: db "predicted val: %f", 10, 0
missed_format: db    "missed    val: %lu", 10, 0

fpu0:
dq 0x0
fpu1:
dq 0x0

align 64
times 22 db 0x0
call_targets:
dq predicted
dq predicted
dq predicted
dq predicted
dq predicted
dq predicted
dq predicted
dq predicted
dq predicted
dq predicted
dq predicted
dq predicted
dq predicted
dq predicted
; this function does not read from a register. So, 
; the 15th address in read_targets could be all 0x0s.
; Setting it to warm_addr will push the speculative
; execution (it thinks it is going to execute read_hot)
; to read from warm_addr.
dq predicted 

section .text
global main 
extern printf

main:

	sub rsp, 0x8
; counters for tsc delta accumulation
; r11 is for predicted read time 
; r12 is for missed read time
	mov r11, 0x0
	mov r12, 0x0

; setup driver_loop variables
	mov r14, 0x0
	mov r15, 0xc000

driver_loop_head:

	push r11
	push r12


; setup read_loop variables.
	mov r13, 0x0 
	mov rax, 0x0
	mov rbx, 0xf

call_loop_head:
	mov r10, [call_targets + rax*8]

; read_tsc tramples rax. Save our iterator.
	push rax

; push the return value
	push post_call

	call read_tsc
	mov r8, rbp

	jmp r10

post_call:

	call read_tsc
	sub rbp, r8

; remember to restore our iterator.
	pop rax

	inc rax
	cmp rax, rbx

; If we are done with the loop, jump to the end.
; Otherwise, add rbp to r13
; and go back to the start of the loop.
	jge loop_end
	add r13, rbp
	jmp call_loop_head 
loop_end:
	; restore the tsc delta accumulation variables.
	pop r12
	pop r11

	add r11, r13
	add r12, rbp

; ++, check and loop(?)
	inc r14
	cmp r14, r15
	jb driver_loop_head 



	finit 
	mov [fpu1], r11
	mov qword [fpu0], 0xf
	fild qword [fpu1]
	fild qword [fpu0]
	fdivp
	fst qword [fpu0]

; print r11 (the time to read hot_addr)
	movq xmm0, qword [fpu0]
	lea edi, [predicted_format]
	mov eax, 0x1
	call printf

; print r12 (the time to read warm_addr)
	mov rsi, r12
	lea edi, [missed_format]
	mov eax, 0
	call printf

; exit
	mov eax, 1
	int 0x80

; support functions

; read the tsc into rbp
read_tsc:
	rdtscp
	shl rdx, 32
	or rdx, rax
	mov rbp, rdx
	ret

predicted:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	ret

; don't do anything
missed:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	ret
