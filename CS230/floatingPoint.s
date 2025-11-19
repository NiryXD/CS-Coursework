# Program Name: Floating Point Program
# Student Name: Ar-Raniry Ar-Rasyid
# Net ID: jzr266
# Student ID: 000-663-921
# Program Description: Mapping function in Assembly

.section .text
.global map
map:
    addi sp, sp, -40
    # Make space on the stack
    sd ra, 32(sp)
    sd s0, 24(sp)
    sd s1, 16(sp)
    sd s2, 8(sp)
    sd s5, 0(sp)
    # Saving return address, array address, mapping address, number of elements, and loop counter
    # In that order

    mv s0, a0
    mv s3 , a2
    mv s4, a1 
    fmv.d fs0, fa0
    # Above we're getting ready to loop so move info into saved registers

    li s5, 0
    # Start the loop counter

1:
    # Startin' the loop
    bge s5, s4, 1f

    slli t0, s5, 3
    add t1, s0, t0
    fld fa0, 0(t1)
    # Give offset and load values in fa0

    fmv.d fa1, fs0
    jalr s3
    fsd fa0, 0(t1)
    addi s5, s5, 1
    j 1b
    # Calls function, stores result, update counter, repeat

1:
    ld ra, 32(sp)
    ld s0, 24(sp)
    ld s3, 16(sp)
    ld s4, 8(sp)
    ld s5, 0(sp)
    addi sp, sp, 40
    ret
    # Cleans up the stack and returns



.global map_add
map_add:
    fadd.d fa0, fa0, fa1
    ret
    # This adds, then returns

.global map_sub
map_sub:
    fsub.d fa0, fa0, fa1
    ret
    # This substracts, then returns

.global map_min
map_min:
    fmin.d fa0, fa0, fa1
    ret
    # This finds the minimum, then returns

.global map_max
map_max:
    fmax.d fa0, fa0, fa1
    ret
    #T This finds the maximum, then returns