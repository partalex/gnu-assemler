.global message
.extern continue, four

.equ init_pc, 0xFFEEDDCC

.section data
.word four,1,2,init_pc
message:
    .ascii "Hello Aleksandar"
.section test1
    ld $2047, %r2
    ld $2048, %r2
    ld $init_pc, %r2
    jmp continue
.end
# .literalstest1
# 00000000: 00 00 07 FF CC DD EE FF   00 00 00 00 .. .. .. ..   |................|


# with addresses
# 0x40000000:   ld $0x$291, %r4
# 0x40000004:   push %r0
# 0x40000008:   jmp continue
# 0x4000000c:   .word 10
# 0x40000010:   pop %r0                     # continue:
# 0x40000014:   ld message, %r1
# 0x40000018:   add %r0, %r1
# 0x4000001c:   halt
# 0x40000020:   .equ four=4
# 0x40000024:   .word four
# 0x40000028:   .word 1
# 0x4000002c:   .word 2
# 0x40000030:   .word 3
# 0x40000034:   .ascii "Hello Aleksandar"   # message:



