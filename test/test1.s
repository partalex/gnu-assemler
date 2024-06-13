.section data
.word 1,2,3,4
message:
    .ascii "Hello, World!"
.section txt
    ld $0x1, %r0
    push %r0
.end