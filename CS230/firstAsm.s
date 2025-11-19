# Program Name: My First Assembly Program
# Student Name: Ar-Raniry Ar-Rasyid
# Net ID: jzr266
# Student ID: 000-663-921
# Program Description: Function that calculates the sum and product

.section .text
.global sum_prod

sum_prod:
    # Below I'm initalizing the sum, product, and the counter
    
    li    t0, 0   # Use t0 for the sum
    li    t1, 1   # Use t1 for the product
    li    t2, 0   # Use t2 for i
   
    # int32_t (a0)
    #     sum_prod
    #        const int32_t values[] (a0)
    #        uint64_t num_values    (a1)
    #        int32_t &product       (a2)
    # Rewritten:
    # a0      sum_prod (a0, a1, a2)

condition:
    # Below the loop happens

    bge    t2, a1, returning

    slli    t3, t2, 2    # t3 = t2 * 4
    add     t3, t3, a0   # t3 = &values[i]

    # Below loads values
    lw    t4, 0(t3)      # t4 = values[i]

    # Below does the math needed
    add    t0, t0, t4    # t0 (sum)     += t4 (values[i])
    mul    t1, t1, t4    # t1 (product) *= t4 (values[i])   

    # Below increments the counter
    addi    t2, t2, 1    # i += 1
    j      condition   

returning:
    # Below returns the sum and product
    mv      a0, t0       # return register a0 = t0 (accumulating sum)
    sw      t1, 0(a2)    # *product = t1 (accumulating product)

ret