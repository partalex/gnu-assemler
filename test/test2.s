.extern message

.section txt
    pop %r0
    ld message, %r1
    add %r2, %r1
.end

.section txt1
    pop %r0
    ld message, %r1
    add %r2, %r1
.end

.section txt2
data:
    .ascii "Ascii"
    pop %r0
    ld message, %r1
    add %r2, %r1
.end
