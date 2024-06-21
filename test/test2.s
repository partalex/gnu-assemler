.extern message
.global init, continue

.sectionIndex one
init:
    .word 10
    pop %r0

    ld message, %r1
    add %r2, %r1
.end

A;