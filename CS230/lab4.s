#function for reference
#RightTriangle make_triangle(float side0, float side1)
#{
#    RightTriangle rt;
#
#    rt.s0 = side0;
#    rt.s1 = side1;
#    rt.hypotenuse = sqrtf((side0 * side0) + (side1 * side1));
#
#    rt.theta0 = asinf(side0 / rt.hypotenuse);
#    rt.theta1 = asinf(side1 / rt.hypotenuse);
#
#    return rt;
#}

.section .text
.global make_triangle

make_triangle:
    #a0 = pointer to triangle
        #s0 offset 0  in a0
        #s1 offset 4 in a0
        #hypotenuse offset 8 in a0
        #theta0 offset 12 in a0
        #theta1 offset 16 in a0
    #grab input values
    fsw     fa0, 0(a0) #s0 in struct
    fsw     fa1, 4(a0) #s1 in struct

    #allocate stack pointer
    addi    sp, sp, -16 #use 3 4 bytes (3 * 4 = 12) but I need multiple of 16
    fsw     fs0, 0(sp)
    fsw     fs1, 4(sp) #fs0 and fs1 for extra stuff and reusing
    sw      ra, 8(sp) #save return address from function call

    #compute hypotenuse s0**2 + s1**2 = fa0**2  
    fmul.s  fs0, fa0, fa0 #s0**2
    fmul.s  fs1, fa1, fa1 #s1**2
    fadd.s  fa0, fs0, fs1 #store hypotenuse in fa0
    call sqrtf #consider fa0 destroyed
    fsw     fa0, 8(a0) #store hypotenuse offset by 8

    #reusing hypotenuse
    fmv.s   fs0, fa0 #replacing og value in fs0 with hypotenuse

    #compute theta0 = asin(s0 / hypotenuse)
    flw     fs1, 0(a0) #store s0 and replace value in fs1 with s0
    fdiv.s  fa0, fs1, fs0 #store quotient in fa0 of s0 / hypotenuse
    call asinf
    fsw     fa0, 12(a0) #store theta0 offset by 12

    #compute theta1 = asin(s1 / hypotenuse)
    flw     fs1, 4(a0) #store s1 and replace value in fs1 with s1
    fdiv.s  fa0, fs1, fs0 #store quotient in fa0 of s1 / hypotenuse
    call asinf
    fsw     fa0, 16(a0) #store theta1 offset by 16

    #restore registers and deallocate stack 
    flw     fs0, 0(sp)
    flw     fs1, 4(sp)
    lw      ra, 8(sp)
    addi    sp, sp, 16 #deallocate memory to struct

    ret
