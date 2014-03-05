;
; Copyright Jeremy 2012-11-28
;
; NOTE:
; - It is for the nasm.
;

; **************************************
; CONSTANT
; **************************************

%define C_SEG_INIT    0x9000
%define C_SEG_SETUP   0x9020
%define C_SEG_SYSTEM  0x1000

; **************************************
; CODE
; **************************************

	jmp L_START

	; Variable
	V_STR_ERROR db 'It is dead in Setup q(<_<!)p',$0

L_START:
	; set the ds,es point to 0x9000
	mov ax, C_SEG_INIT
	mov ds, ax
	mov es, ax
	
	; read cursor position
	mov ah, 0x03
	xor bh, bh
	int 0x10
	mov [0], dx
	
	; get memory size (extended mem, kB)
	; interrupt 0x15 & operation ah = 0x88
	; return -> ax
	; - success : the external memory size in KB which after 1MB
	; - failed  : the error code and CF is set.
	mov ah, 0x88
	int 0x15
	mov [2], ax
	
	; get video card data
	mov ah, 0x0F
	int 0x10
	mov [4], bx
	mov [6], ax
	
	; check for EGA/VGA and some config parameter
	mov ah, 0x12
	mov bl, 0x10
	int 0x10
	mov [8], ax
	mov [10], bx
	mov [12], cx
	
	; enter VGA 320x200 8bit color mode
	mov al, 0x13
	mov ah, 0x00
	int 0x10
	mov al, 8            ; color mode
	mov [14], al
	mov ax, 320          ; width
	mov [16], ax         
	mov ax, 200          ; high
	mov [18], ax         
	mov eax, 0x000a0000  ; video memory address
	mov [20], eax
	
	; Get hd0 data
	mov ax, 0x0000
	mov ds, ax
	lds si, [4 * 0x41]
	mov ax, C_SEG_INIT
	mov es, ax
	mov di, 0x0080
	mov cx, 0x10
	rep movsb

	; Get hd1 data
	mov ax, 0x0000
	mov ds, ax
	lds si, [4 * 0x46]
	mov ax, C_SEG_INIT
	mov es, ax
	mov di, 0x0090
	mov cx, 0x10
	rep movsb
	
	; check that there IS a hd1
	mov ax, 0x01500
	mov dl, 0x81
	int 0x13
	jc L_NO_DISK_1
	cmp ah, 3
	je L_IS_DISK_1
	
L_NO_DISK_1:
	mov ax, C_SEG_INIT
	mov es, ax
	mov di, 0x0090
	mov cx, 0x10
	mov ax, 0x00
	rep stosb
	
	; go to pretected mode...
L_IS_DISK_1:
	; no interrupt
	cli
	mov ax, 0x0000
	cld
L_DO_MOVE:
	mov es, ax
	add ax, 0x1000
	cmp ax, 0x9000
	jz L_END_MOVE
	mov ds, ax
	sub di, di
	sub si, si
	mov cx, 0x8000
	rep movsw
	jmp L_DO_MOVE
	
L_END_MOVE:
	mov ax, C_SEG_SETUP
	mov ds, ax
	lidt [L_IDT_48]
	lgdt [L_GDT_48]
	
	call FN_EMPTY_8042
	mov al, 0xD1
	out 0x64, al
	call FN_EMPTY_8042
	mov al, 0xDF
	out 0x60, al
	call FN_EMPTY_8042
	
	mov al, 0x11         ; initialization sequence
	out 0x20, al	     ; send it to 8259A-1
	dw 0x00EB, 0x00EB
	out 0xA0, al         ; and to 8259A-2
	dw 0x00EB, 0x00EB
	
	mov al, 0x20         ; start of hardware int (0x20)
	out 0x21, al
	dw 0x00EB, 0x00EB
	
	mov al, 0x28         ; start of hardware int (0x28)
	out 0xA1, al
	dw 0x00EB, 0x00EB
	
	mov al, 0x04         ; 8269-1 is master
	out 0x21, al
	dw 0x00EB, 0x00EB
	
	mov al, 0x02         ; 8259-2 is slave
	out 0xA1, al
	dw 0x00EB, 0x00EB
	
	mov al, 0x01         ; 8086 mode for both
	out 0x21, al
	dw 0x00EB, 0x00EB
	out 0xA1, al
	dw 0x00EB, 0x00EB
	
	mov al, 0xFF         ; mask off all interrupts
	out 0x21, al
	dw 0x00EB, 0x00EB
	out 0xA1, al

    ; check for PS/2 pointing device
    ;int 0x11
    ;test al, 0x04
    ;jz L_ERROR

	; enter 32-bit proctected mode
	mov ax, 0x0001
	; 0x90322
	lmsw ax
	jmp 8:0
	
L_ERROR:
	; print the welcome message
	mov ax, cs
	mov ds, ax
	mov si, V_STR_ERROR
L_PRINT:
	lodsb
	cmp al, 0
	je L_END
	mov ah, 0Eh
	mov bx, 7
	int 10h
	jmp L_PRINT
L_END:
	jmp L_END
	
FN_EMPTY_8042:
	dw 0x00eb, 0x00eb
	in al, 0x64
	test al, 2
	jnz FN_EMPTY_8042
	ret
	
L_GDT:
	dw 0, 0, 0, 0
	dw 0x07FF
	dw 0x0000
	dw 0x9A00
	dw 0x00C0
	dw 0x07FF
	dw 0x0000
	dw 0x9200
	dw 0x00C0
	
L_IDT_48:
	dw 0
	dw 0, 0

L_GDT_48:
	dw 0x0800
	dw 512+L_GDT, 0x9

