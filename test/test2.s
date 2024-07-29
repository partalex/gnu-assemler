.extern message
.global four, continue
.equ four, 4

.section test2
    .word 10
continue:
    pop %r0
    ld message, %r1
    add %r2, %r1
    halt
.end

