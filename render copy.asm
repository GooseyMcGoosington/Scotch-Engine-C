BITS 64 ; this is important
DEFAULT REL

section .data
  lX dd 0.0 ; dX
  stX dd 0 ; startX
  enX dd 0 ; endX

  x0 dd 0
  x1 dd 0  
  y0 dd 0
  y1 dd 0
  y2 dd 0
  y3 dd 0
  sw dd 0
  sh1 dd 0
  pX dd 0
  fPX dd 0.0

  v dd 0
  pt dd 0

  boundsPtr dq 0
  pixelsPtr dq 0

  fOne dd 1.0

section .text
  global draw_wall_ASM

clampMin:
  mov rax, rcx
  ret
  
clampMax:
  mov rax, rdx
  ret

clamp:
  cmp rax, rcx
  jl clampMin
  cmp rax, rdx
  jg clampMax
  ret

done_x:
  mov rax, 0
  mov [pX], rax ; reset px
  ret

x_loop:
  mov rax, [pX]
  mov rbx, [sw]
  cmp rax, rbx        ; compare pX with sw
  jge done_x     ; if pX >= sw, exit loop

  ; calculate t
  cvtsi2ss xmm0, rax ; precompute fPX
  movss [fPX], xmm0

  mov rbx, [x0]
  cvtsi2ss xmm1, rbx ; convert sx0 to a float
  subss xmm0, xmm1 ; x - sx0
  mov rbx, [lX] ; (x - sx0) / dx
  divss xmm0, xmm1
  
  ; load fPX and px0
  movss xmm0, [fPX]
  lea r10, [boundsPtr]
  movss xmm1, [r10]
  subss xmm0, xmm1 ; (x-portalBounds.x0)
  ; load fPX and px0
  lea r10, [boundsPtr]
  add qword r10, 24 ; access dxCull
  movss xmm1, [r10]
  ;
  divss xmm0, xmm1
  ; draw line. for testing
  mov r11, qword [pixelsPtr]
  mov dword [r11], 25000
  ; decide whether to terminate or loop
  mov rax, [pX]
  inc rax            ; increment pX
  mov [pX], rax
  jmp x_loop        ; continue loop

draw_wall_ASM:
  ; r0 = rcx
  ; r1 = rdx
  ; r2 = r8
  ; r3 = r9
  ; ret = rax

  ; xmm0, xmm1, xmm2, xmm3 = x0, x1, y0, y1
  ; can access r10 to r15
  ; for any variables that do not fit within rcx, rdx, r8 and r9, use the stack; PUSH POP

  subss xmm0, xmm1
  movss [lX], xmm0

  mov rax, [rsp+28h]
  mov [y2], rax ; y2
  mov rax, [rsp+30h]
  mov [y3], rax ; y3

  mov eax, [rsp+48h] ; SW
  mov [sw], eax

  mov eax, [rsp+4ch] ; SH
  mov [sh1], eax
  ; let's locate our portal bounds
  mov r10, [rsp+38h] ; portal bounds address
  mov [boundsPtr], r10
  mov r11, [rsp+40h] ; pixel array address
  mov [pixelsPtr], r11

  cvttss2si rax, xmm2 ; put y0 into rax, converting y0 into an integer
  mov [y0], rax
  cvttss2si rax, xmm3 ; put y1 into rax, converting y1 into an integer
  mov [y1], rax

  cvttss2si rax, xmm1 ; put x1 into rax, converting x1 into an integer
  mov [x1], rax
  call clamp
  mov [x1], rax ; store result of clamp
  cvttss2si rax, xmm0 ; put x0 into rax, converting x0 into an integer
  mov [x0], rax
  call clamp
  mov [x0], rax ; store result of clamp

  ret