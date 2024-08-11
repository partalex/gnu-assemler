.extern value1
# .equ value1, 0xffff

.section code
#    st %r1, $0x1       # OK: REG_DIR
#    st %r1, $0xffff    # OK: IN_DIR_OFFSET
#    st %r1, 0x1        # OK: IN_DIR_OFFSET
#    st %r1, $value1    # OK: IN_DIR_OFFSET
#    st %r1, 0x1        # OK: IN_DIR_OFFSET
#    st %r1, 0xffff     # OK: IN_DIR_IN_DIR
#    st %r1, value1     # OK: IN_DIR_IN_DIR

#    ld $0x1, %r1       # OK: REG_DIR
#    ld $0xffff, %r1    # OK: IN_DIR_OFFSET
#    ld 0x1, %r1        # OK: IN_DIR_OFFSET
#    ld $value1, %r1    # OK: IN_DIR_OFFSET
#    ld 0x1, %r1        # OK: IN_DIR_OFFSET
#    ld 0xffff, %r1     # OK: IN_DIR_IN_DIR
#    ld value1, %r1     # OK: IN_DIR_IN_DIR


.end

# .section my_data
# value1:
# .word 0
# .end

# 0x00000000:   st %r1, value1      # st %r1, [PC+offset]
# 0x00000004:   st %r1, $value1
# 0x00000008:   jmp overLiterals
# 0x0000000C:   00 00 00 00: address(value1)
# 0x00000010:   00 00 00 00: value(value1)
# .. .. .. ..
# 0x00000100:   .word 0; value1

############## After Linker ##############

# 0x00000000:   st %r1,
# 0x00000004:   st %r1, $value1
# 0x00000008:   jmp overLiterals
# 0x0000000C:   00 00 01 00: address of value1
# 0x00000010:   00 00 00 00: value(value1)
# .. .. .. ..
# 0x00000100:   .word 0; value1

