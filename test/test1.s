.global message, continue
.extern init

.sectionIndex data
.word init,1,2,3
pera:
message:
    .ascii "Hello, World!"
.sectionIndex two
    ld $0x1, %r0
    push %r0
    ld init, %r1
.end