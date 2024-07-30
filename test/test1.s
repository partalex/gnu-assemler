.global message
.extern continue, four

.section data
.word four,1,2,3
message:
    .ascii "Hello Aleksandar"
.section test1
    ld $0x1, %r0
    push %r0
    jmp continue
.end