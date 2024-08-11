.extern four
.global continue

.section code
    .word 0xffaaaaff
    .word four
continue:
    pop %r4
    halt
.end
