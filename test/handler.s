.extern isr_timer, isr_terminal

.global handler
.section my_handler

handler:
    push %r1
    push %r2
    csrrd %cause, %r1
    ld $2, %r2
    beq %r1, %r2, handle_timer
    ld $3, %r2
    beq %r1, %r2, handle_terminal
finish:
    pop %r2
    pop %r1
    iret
# obrada prekida od tajmera
handle_timer:       # 0x000000A0
    call isr_timer  # isr_timer = 0x00500000; D = 0x00500000 - 0x000000A0 = 0x004FF960
    call 0x00500000 # can not fitIn12Bits; D = 0x000000A0 - 0x00000014 = 0x0000008C in dec 140
    jmp finish
# obrada prekida od terminala
handle_terminal:
    call isr_terminal
    jmp finish
.end

.literals       # 0x00000014
    0x004FF960  # 0x00000014

