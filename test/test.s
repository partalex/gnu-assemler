.section txt
message:
.ascii "Hello, World!"
    ld $0x1, %r1
    push %r1
.end