.global message
.extern continue, four

.section data
.word 1,2,3,four
message:
    .ascii "Hello, World!"
.section test1
    ld $0x1, %r0
    push %r0
    jmp continue
.end