# Program Name: The Final
# Student Name: Ar-Raniry Ar-Rasyid
# Net ID: jzr266
# Student ID: 000-663-921
# Program Description: Searches for struct for matching float and char

.section .text
.global find

find:
    # Allocate space on the stack
    addi    sp, sp, -64
    # Saving things we need (return address, entries, outer, etc.)
    sd      ra, 56(sp)
    sd      s0, 48(sp)
    sd      s1, 40(sp)
    sd      s2, 32(sp)
    sd      s3, 24(sp)
    sd      s5, 16(sp)
    sd      s6, 8(sp)

    mv      s0, a0         
    mv      s1, a1
    # Char we're looking for        
    mv      s2, a2      
    mv      s3, a3    

    # Start at first struct
    li      s5, 0    

outer:
    # If nothing is found go to NA
    bge     s5, s1, NA


    slli    t0, s5, 5  
    add     t1, s0, t0  

    # Load float
    fld     ft0, 0(t1)
    # Check if float matches
    feq.d   t2, ft0, fa0
    # If not equal, move on
    beq     t2, x0, next

    # Start at first char
    li      s6, 0
    addi    t3, t1, 8

inner:
    # Max to check (char) is 21
    li      t4, 21
    bge     s6, t4, next

    add     t5, t3, s6
    lbu     t6, 0(t5)
    # If matching, we found it
    beq     t6, s2, matching

    addi    s6, s6, 1
    # Keep checking
    j       inner

next:
    # Move next struct
    addi    s5, s5, 1
    j       outer

matching:
    # Return outer index
    mv      a0, s5
    # Store inner index
    sd      s6, 0(s3)
    j       done

# This stands for not avaliable
NA:
    mv      a0, s1
    li      t6, 21
    sd      t6, 0(s3)

done:
    # Restore everything
    ld      ra, 56(sp)
    ld      s0, 48(sp)
    ld      s1, 40(sp)
    ld      s2, 32(sp)
    ld      s3, 24(sp)
    ld      s5, 16(sp)
    ld      s6, 8(sp)
    # Clean up stack
    addi    sp, sp, 64
    ret
