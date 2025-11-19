.section .text
.global map
map:
    addi sp, sp, -80        # allocate stack space
    sd ra, 0(sp)            # save return address
    sd s0, 8(sp)            # save s0 (values)
    sd s1, 16(sp)           # save s1 (loop counter)
    sd s2, 24(sp)           # save s2 (num_values)
    sd s3, 32(sp)           # save s3 (mapping_func)
    sd s4, 40(sp)           # save s4 (offset ptr)
    sd s5, 48(sp)           # save s5 (used for map_value copies)
    fsd fs0, 56(sp)         # save fs0 (map_value)
    fsd fs1, 64(sp)         # save fs1 (map_value temp copy)
    fsd fs2, 72(sp)         # save fs2 (result register if needed)

    mv s0, a0               # s0 = values[]
    mv s2, a1               # s2 = num_values
    mv s3, a2               # s3 = mapping_func
    fmv.d fs0, fa0          # fs0 = map_value (store in callee-saved reg)

    li s1, 0                # i = 0 (loop counter)
1:
    bge s1, s2, 1f          # if i >= num_values, exit loop

    slli s4, s1, 3          # s4 = i * 8 (offset)
    add s4, s0, s4          # s4 = &values[i]
    fld fa0, 0(s4)          # fa0 = values[i]
    fmv.d fa1, fs0          # fa1 = map_value

    jalr s3                 # call mapping_func(fa0, fa1)

    fsd fa0, 0(s4)          # values[i] = result (stored in fa0)
    addi s1, s1, 1          # i++
    j 1b                    # repeat loop
1:
    ld ra, 0(sp)            # restore return address
    ld s0, 8(sp)
    ld s1, 16(sp)
    ld s2, 24(sp)
    ld s3, 32(sp)
    ld s4, 40(sp)
    ld s5, 48(sp)
    fld fs0, 56(sp)         # restore fs0
    fld fs1, 64(sp)
    fld fs2, 72(sp)
    addi sp, sp, 80         # deallocate stack space
    ret

.global map_add
map_add:
    fadd.d fa0, fa0, fa1    # fa0 = fa0 + fa1 
    ret

.global map_sub
map_sub:
    fsub.d fa0, fa0, fa1    # fa0 = fa0 - fa1
    ret

.global map_min
map_min:
    fmin.d fa0, fa0, fa1    # fa0 = min(fa0, fa1)
    ret

.global map_max
map_max:
    fmax.d fa0, fa0, fa1    # fa0 = max(fa0, fa1)
    ret
