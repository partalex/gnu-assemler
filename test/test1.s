.global message
.extern continue, four

.equ test, init_pc - less
.equ init_pc, 0xFFEEDDCC
.equ less, 128

.section data
.word four,1,2,init_pc
message:
    .ascii "Hello Aleksandar"
.section test1
chick:
    jmp message
    jmp chick
    jmp init_pc
    jmp 0x40000000
    ld $four, %r2
    ld $test, %r2
    ld $less, %r2
    ld $init_pc, %r2
    ld $2047, %r2
    ld $2048, %r2
    add %r0, %r1
    ld $0x291, %r4
    ld $test, %r4
testJmp:
    jmp message
    ld continue, %r1
    jmp continue
    jmp end
    jmp testJmp
    jmp testJmp
    jmp 0x50000000
end:
    jmp end
.end



# .extern message
# .section code
#    push %r1
#    jmp message         # 0x00000000
#.end

# literals pool         # 0x00000008
# 00 00 00 00           # 0x00000008 message
# 00 00 00 00           # 0x0000000c PC+message

# push %r14             # 0x00000000            0
# ld PC+message, %r14   # 0x00000004            4
# jmp mem[%r14]         # 0x00000008            8
# pop %r14              # 0x0000000c            12
# jmp overLiterals      # 0x00000010            16

# literals pool         # 0x00000014            20
# 00 00 00 00           # 0x00000014 message    24
