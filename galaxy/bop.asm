org 0x100
bits 16
mov si, DLL_NAME
mov di, INIT_PROC
; comment out for no init proc...
xor di, di
push di
pop es
mov bx, DISPATCHER
db 0xc4, 0xc4, 0x58, 0x0 ; register
jc error
mov [DLL_HANDLE], ax
mov al, 'L' ; loaded
int 0x29

mov ax, [DLL_HANDLE]
mov cx, message_end - message_start
mov si, message_start
db 0xc4, 0xc4, 0x58, 0x2 ; invoke dispatcher
cmp ax, 0x2005 ; check return value...
jnz no_ret
mov al, 'R'
int 0x29
jmp skip
no_ret:
mov al, 'N'
int 0x29
skip:
;...
;...
mov ax, [DLL_HANDLE]
db 0xc4, 0xc4, 0x58, 0x1 ; unregister
ret
error:
mov al, '!'
int 0x29
ret

DLL_HANDLE: dw 0
DLL_NAME: db "galaxy.dll", 0
INIT_PROC: db "InitProc", 0
DISPATCHER: db "MyRoutine", 0
message_start:
db "http://ragestorm.net",0
message_end: