# file: main.s

.global my_start, my_counter
.extern handler

.equ initial_sp, 0xFFFFFEFE
.equ timer_config, 0xFFFFFF10

.section code
my_start:
    ld $initial_sp, %sp
    ld $handler, %r1
    csrwr %r1, %handler

    ld $0x1, %r1
    st %r1, timer_config
wait:
    ld my_counter, %r1
    ld $20, %r2
    bne %r1, %r2, wait
    halt

.section my_data
my_counter:
.word 0
.end


# 0x40000000:   ld $initial_sp, %sp
# 0x40000004:   ld $handler, %r1
# 0x40000008:   csrwr %r1, %handler
# 0x4000000c:   ld $0x1, %r1
# 0x40000010:   st %r1, timer_config
# 0x40000014:   ld my_counter, %r1              # wait:
# 0x40000018:   ld $20, %r2
# 0x4000001c:   bne %r1, %r2, wait
# 0x40000020:   halt

# 0x50000000:   push %r1                        # handler:
# 0x50000004:   push %r2
# 0x50000008:   csrrd %cause, %r1
# 0x5000000c:   ld $2, %r2
# 0x50000010:   beq %r1, %r2, handle_timer
# 0x50000014:   ld $3, %r2
# 0x50000018:   beq %r1, %r2, handle_terminal
# 0x5000001c:   pop %r2                         # finish:
# 0x50000020:   pop %r1
# 0x50000024:   iret
# 0x50000028:   call isr_timer                  # handle_timer:
# 0x5000002c:   call 0x00500000
# 0x50000030:   jmp finish
# 0x50000034:   call isr_terminal               # handle_terminal:
# 0x50000038:   jmp finish

# 0x5000003c:   push %r1                        # isr_terminal:
# 0x50000040:   push %r2
# 0x50000044:   ld terminal_in, %r1
# 0x50000048:   ld $character_offset, %r2
# 0x5000004c:   add %r2, %r1
# 0x50000050:   st %r1, terminal_out
# 0x50000054:   ld my_counter, %r1
# 0x50000058:   ld $1, %r2
# 0x5000005c:   add %r2, %r1
# 0x50000060:   st %r1, my_counter
# 0x50000064:   pop %r2
# 0x50000068:   pop %r1
# 0x5000006c:   ret

# 0x50000070:   push %r1                        # isr_timer:
# 0x50000074:   push %r2
# 0x50000078:   push %r3
# 0x5000007c:   ld $0, %r2
# 0x50000080:   ld message_start, %r3           # loop:
# 0x50000084:   add %r2, %r3
# 0x50000088:   ld [%r3], %r3
# 0x5000008c:   st %r3, terminal_out
# 0x50000090:   ld $1, %r1
# 0x50000094:   add %r1, %r2
# 0x50000098:   ld $message_len, %r1
# 0x5000009c:   bne %r1, %r2, 0x50000080
# 0x500000a0:   ld $line_feed, %r1
# 0x500000a4:   st %r1, terminal_out
# 0x500000a8:   ld $carriage_return, %r1
# 0x500000ac:   st %r1, terminal_out
# 0x500000b0:   pop %r3
# 0x500000b4:   pop %r2
# 0x500000b8:   pop %r1
# 0x500000bc:   ret
