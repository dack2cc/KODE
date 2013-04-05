;
; Copyright Jeremy 2012-11-28
;
; NOTE:
; - It is for the nasm.
;

; **************************************
; CONSTANT
; **************************************

%define C_SEG_BOOT     0x07C0
%define C_SEG_INIT     0x9000

%define C_SEG_SETUP    0x9020
%define C_SETUP_LEN    4

%define C_SEG_SYSTEM   0x1000
%define C_SYSTEM_SIZE  0x3000
%define C_SEG_END      C_SEG_SYSTEM + C_SYSTEM_SIZE

;
; Device : 0x0?00 - 1 ram, 2 floopy, 3 hard disk, 4 ttyx, 5 tty, 6 serial port, 7 non-named piple
;          0x0000 - the same type of floppy as boot
;          0x0300 - /dev/hd0 1st disk
;          0x0301 - /dev/hd1 1st pattern of 1st disk
;          ...
;          0x0304 - /dev/hd4 4th pattern of 1st disk
;          0x0305 - /dev/hd5 2nd disk
;          0x0306 - /dev/hd6 1st pattern of 2nd disk
;          ...
;          0x0309 - /dev/hd9 4th pattern of 2nd disk
;
%define C_ROOT_DEV_ADDR   508
%define C_ROOT_DEV_VALUE  0x0000

; **************************************
; CODE
; **************************************

	; start
ORG 0
	jmp C_SEG_BOOT:L_START

	; variable
    V_STR_ERROR  db 'It is dead in Boot q(<_<!)p', $0
    V_DISK_SECTORS dw 0x0000
    V_SYSTEM_SECTORS dw 1 + C_SETUP_LEN
    V_HEAD dw 0
    V_TRACK dw 0

L_START:
	; ds:si = 0x07C0:0x0000
	; absolutely address = 0x07C00 (0x07C00 + 0x00000)
	; the source of movsw is ds:si
	mov ax, C_SEG_BOOT
	mov ds, ax
	sub si, si

	; es:di = 0x9000:0x0000
	; absolutely address = 0x90000 (0x90000 + 0x00000)
	; the destiny of movsw is es:di
	mov ax, C_SEG_INIT
	mov es, ax
	sub di, di
	
	; copy 256 bytes: 0x07C00 -> 0x90000
	mov cx, 256 ; the loop count = 256
	rep movsw   ; copy one byte
	
	; continue from the 0x9000:L_GO
	jmp C_SEG_INIT:L_GO

L_GO:
	; set the ds,es,ss point to 0x9000
	mov ax, cs
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, 0xFF00

	; 'es' is already set to 0x9000	
	; load the setup sectors after the boot block.
	
L_LOAD_SETUP:
	mov dx, 0x0000
	mov cx, 0x0002
	mov bx, 0x0200
	mov ax, 0x0200 + C_SETUP_LEN
	int 0x13
	jnc L_LOAD_SETUP_OK
	mov dx, 0x0000
	mov ax, 0x0000
	int 0x13
	jmp L_LOAD_SETUP
	
L_LOAD_SETUP_OK:
	; get disk drive parameters, specifially sectors/track
	mov dl, 0x00
	mov ax, 0x0800
	int 0x13
	mov ch, 0x00
	mov [V_DISK_SECTORS], cx
	mov ax, C_SEG_INIT
	mov es, ax

	; print some message
	mov ah, 0x03
	xor bh, bh
	int 0x10
	
	mov cx, 24
	mov bx, 0x0007
	mov bp, L_MSG_LOADING
	mov ax, 0x1301
	int 0x10
	
	mov ax, C_SEG_SYSTEM
	mov es, ax
	call FN_READ_IT	
	call FN_KILL_MOTOR

L_CHECK_ROOT_DEVICE:
	mov word ax, [C_ROOT_DEV_ADDR]
	cmp ax, 0
	jne L_ROOT_DEFINED
	mov bx, [V_DISK_SECTORS]
	mov ax, 0x0208
	cmp bx, 15
	je L_ROOT_DEFINED
	mov ax, 0x021C
	cmp bx, 18
	je L_ROOT_DEFINED
	
L_UNKNOWN_ROOT:
	jmp L_UNKNOWN_ROOT

L_ROOT_DEFINED:
	; save the root device
	mov word [C_ROOT_DEV_ADDR], ax
	
	; jump to the setup routine
	jmp C_SEG_SETUP:0x0000

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
	
FN_READ_IT:
	mov ax, es
	test ax, 0x0FFF
L_DIE:
	jne L_DIE
	xor bx, bx
L_REPEAT_READ:
	mov ax, es
	cmp ax, C_SEG_END
	jb L_OK1_READ
	ret
L_OK1_READ:
	mov ax, [V_DISK_SECTORS]
	sub ax, [V_SYSTEM_SECTORS]
	mov cx, ax
	shl cx, 9
	add cx, bx
	
	jnc L_OK2_READ
	je L_OK2_READ
	xor ax, ax
	sub ax, bx
	shr ax, 9
L_OK2_READ:
	call FN_READ_TRACK
	mov cx, ax
	add ax, [V_SYSTEM_SECTORS]
	cmp ax, [V_DISK_SECTORS]
	jne L_OK3_READ
	mov ax, 1
	sub ax, [V_HEAD]
	jne L_OK4_READ
	;inc [V_TRACK]
	push ax
	mov ax, [V_TRACK]
	inc ax
	mov [V_TRACK], ax
	pop ax
L_OK4_READ:
	mov [V_HEAD], ax
	xor ax, ax
L_OK3_READ:
	mov [V_SYSTEM_SECTORS], ax
	shl cx, 9
	add bx, cx
	jnc L_REPEAT_READ
	
	mov ax, es
	add ax, 0x1000
	mov es, ax
	xor bx, bx
	jmp L_REPEAT_READ

FN_READ_TRACK:
	push ax
	push bx
	push cx
	push dx
	mov dx, [V_TRACK]
	mov cx, [V_SYSTEM_SECTORS]
	inc cx
	mov ch, dl
	mov dx, [V_HEAD]
	mov dh, dl
	mov dl, 0
	and dx, 0x0100
	mov ah, 2
	int 0x13
	; 0x90158
	jc L_BAD_RET
	pop dx
	pop cx
	pop bx
	pop ax
	ret
L_BAD_RET:
	mov ax, 0
	mov dx, 0
	int 0x13
	pop dx
	pop cx
	pop bx
	pop ax
	jmp FN_READ_TRACK
	
FN_KILL_MOTOR:
	push dx
	mov dx, 0x03F2
	mov al, 0
	out dx, al
	pop dx
	ret

L_MSG_LOADING:
	db 13,10
	db "Loading System <<",$0
	db 13,10,13,10
	
	; fill blank after the code
	times 508-($-$$) db 0
	dw C_ROOT_DEV_VALUE
	dw 0xAA55 ; End he file with AA55

