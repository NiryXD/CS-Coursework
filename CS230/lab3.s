# Program name: Floating Point
# Student name: Tristan Lopez
# Net ID: tlopez7
# User ID: 000664711
# Description The program uses a mapping function that is able
# to be changed by different instructions called by the user
.section .text
.global map
map:
    addi sp, sp, -48            # space is allocated
    sd   ra, 40(sp)             # return address is set
    sd   s0, 32(sp)             # address of the array is set
    sd   s3, 24(sp)             # mapping_func is set
    sd   s4, 16(sp)             # num_values is set
    sd   s5, 8(sp)              # loop is set

    mv   s0, a0                 # address of values is moved to s0
    mv   s3, a2                 # move mapping_func into s3
    mv   s4, a1                 # move num_values into s4
    fmv.d   fs0, fa0            # move map_value into fs0
    li   s5, 0                  # i

1:  
    bge    s5, s4, 1f           # loop ends if i >= number values

    slli t0, s5, 3              # t0 is scaled by i^3
    add  t1, s0, t0             # t1 is set to the address of the values
    fld  fa0, 0(t1)             # the values are loaded into fa0

    fmv.d  fa1, fs0             # set double right = map_value 
    jalr   s3                   # map_function(fa0, fa1)

    fsd    fa0, 0(t1)           #  the values are stored into fa0
    addi   s5, s5, 1            # i++

    j 1b                        # the loop goes again

1:  
    ld   ra, 40(sp)             # the return address is restored 
    ld   s0, 32(sp)             # s0 is restored
    ld   s3, 24(sp)             # s3 is restored
    ld   s4, 16(sp)             # s4 is restored
    ld   s5, 8(sp)              # s5 is restored
    addi sp, sp, 48             # space in the stack is deallocated

    ret                         # return 

.global map_add
map_add:
    fadd.d fa0, fa0, fa1        # map_add function is done (add+)
    ret                         # return 

.global map_sub
map_sub:
    fsub.d fa0, fa0, fa1        # map_sub function is done (subtract-)
    ret                         # return 

.global map_min
map_min:
    fmin.d fa0, fa0, fa1        # map_min function is done (minimum)
    ret                         # return 

.global map_max
 map_max:
    fmax.d fa0, fa0, fa1        # map_max function is done (maximum)
    ret                         # return

