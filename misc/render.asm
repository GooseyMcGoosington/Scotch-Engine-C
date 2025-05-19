BITS 64 ; this is important
DEFAULT REL

section .data align=16
  cy dd 0
  tTop dd 0
  idxY dd 0.0
  shade dd 0
  UCoord dd 0
  idxYFixed dd 0

  align 8
  texturePtr dq 0

  r12Old dq 0
  cR dw 0
  cG dw 0
  cB dw 0

section .text
  global draw_wall_Y

clampMin:
  ; If the value is less than the minimum (r10), return r10
  mov rax, r10   ; rax = r10 (min value)
  ret

clampMax:
  ; If the value is greater than the maximum (r11), return r11
  mov rax, r11   ; rax = r11 (max value)
  ret

clamp:
  ; Compare the value (rax) with the minimum (r10)
  cmp rax, r10   ; Compare rax (value) with r10 (min)
  jl clampMin    ; Jump to clampMin if rax is less than r10
  ; Compare the value (rax) with the maximum (r11)
  cmp rax, r11   ; Compare rax (value) with r11 (max)
  jg clampMax    ; Jump to clampMax if rax is greater than r11
  ; If rax is within the range [r10, r11], just return
  ret

end_loop:
  ret

draw_loop:
  ; rcx is pixel pointer
  ; rdx is SW
  ; r8 is clampSY0
  ; r9 is clampSY1
  inc r8
  cmp r8, r9 ; compare our current y with the top of SY1
  jge end_loop
  
  ; lets calculate texY
  mov rax, r8
  sub rax, [tTop]              ; (y - tTop)
  imul rax, [idxYFixed]     ; (y - tTop) * idxYFixed
  sar rax, 12               ; >> 12 (same as / 4096)
  and rax, 0x3F             ; & 0x3F
  shl rax, 6 ; << 6

  lea r11, [texturePtr]
  add r11, rax ; U + V
  mov r11, [r11] ; we now have the pixel colour
  
  ; let's calculate our pixel position
  mov r12, r11
  shr r12, 11 ; calculate R
  and r12, 0x1F
  mov [cR], r12

  mov r12, r11
  shr r12, 5 ; calculate G
  and r12, 0x3F
  mov [cG], r12

  mov r12, r11
  and r12, 0x1F ; calculate B
  mov [cB], r12

  add rcx, rdx
  mov word [rcx], 5000
  ;mov dword [r10], 5000
  jmp draw_loop ; return back to the top

draw_wall_Y:
  ; r0 = rcx
  ; r1 = rdx
  ; r2 = r8
  ; r3 = r9
  ; ret = rax

  ; xmm0, xmm1, xmm2, xmm3 = x0, x1, y0, y1
  ; can access r10 to r15
  ; for any variables that do not fit within rcx, rdx, r8 and r9, use the stack; PUSH POP

  ; shadow space for stack = 28h
  ; move the pixel pointer into memory

  ; rcx is pixel pointer
  ; rdx is SW
  ; r8 is clampSY0
  ; r9 is clampSY1

  ; mov dword [rcx], 5000
  ; let's calculate the top of our loop
  ;  int tTop,
  ;  int idxY,
  ;  int shade,
  ;  int U,
  ;  long long wtPtr

  ;   int_fast16_t texY = ((int_fast16_t)((y - tTop) * idxY) & 0x3F);
  ;   int_fast16_t V = (texY << 6);

  mov rax, [rsp+28h] ; tTop
  mov [tTop], rax

  movss xmm0, [rsp+30h] ; idxY
  movss [idxY], xmm0

  cvttss2si rax, xmm0
  imul rax, rax, 4096
  mov [idxYFixed], rax

  mov rax, [rsp+32h] ; shade
  mov [shade], rax

  mov rax, [rsp+34h] ; U
  mov [UCoord], rax

  mov rax, [rsp+3ch] ; wtPtr
  mov [texturePtr], rax

  shl rdx, 1

  mov [r12Old], r12
  call draw_loop
  mov r12, [r12Old]
  ret

