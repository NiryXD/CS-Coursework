# Program Name: Make triangle
# Student Name: Ar-Raniry Ar-Rasyid
# Net ID: jzr266
# Student ID: 000-663-921
# Program Description: Takes the input of two legs and output the info for the rest of the triangle

.section .text
.global  make_triangle

make_triangle:
    
    # Make space on the stack 
    addi    sp, sp, -48    
    # Save the addresses 
    sw      ra, 44(sp)   
    sw      s1, 40(sp)      
    
    # Store pointer to s1 for later use
    # I've had seg faults and apparently this helps
    mv      s1, a0
    # Store leg inputs
    fsw     fa0, 0(s1)      
    fsw     fa1, 4(s1)      
    
    # Save leg inputs onto stack
    # More preventative measures for seg fault
    fsw     fa0, 32(sp)    
    fsw     fa1, 36(sp)    
    
    # Find hypotnuse (Pythagorean Theorem)
    fmul.s  ft0, fa0, fa0   
    fmul.s  ft1, fa1, fa1   
    fadd.s  fa0, ft0, ft1   
    call    sqrtf       
    # TL;DR A^2 + B^2 = C^2 
    
    # Save it into struct and stack
    fsw     fa0, 8(s1)
    fsw     fa0, 28(sp)    
    
    # Find theta0
    flw     fa1, 32(sp)   
    fdiv.s  fa0, fa1, fa0  
    call    asinf           
    
    # Store what we just found
    fsw     fa0, 12(s1)
    # I think I messed up somewhere, so I can't use s0
    # Alternatively, s1 works so I'll roll with that     
    
    # Now do theta1
    flw     fa1, 36(sp)     
    flw     fa0, 28(sp)     
    fdiv.s  fa0, fa1, fa0  
    call    asinf           

    # Store what we just found
    fsw     fa0, 16(s1)     
    
    # Move pointer back
    mv      a0, s1
    
    # Clean up the stack
    lw      s1, 40(sp)
    lw      ra, 44(sp)
    addi    sp, sp, 48
    
    ret