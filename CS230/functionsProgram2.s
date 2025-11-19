# int64_t get_rand(int64_t mn, int64_t mx) {
#    return mn + rand() % (mx - mn + 1);
# }

    .text
    .globl get_rand

get_rand:
    # Step 1: Setup stack and save registers
    addi    sp, sp, -32       # Allocate stack space (aligned to 16 bytes)
    sd      ra, 0(sp)         # Save return address
    sd      s0, 8(sp)         # Save s0 (mn)
    sd      s1, 16(sp)        # Save s1 (mx)

    # Step 2: Move arguments to saved registers
    mv      s0, a0            # s0 = mn
    mv      s1, a1            # s1 = mx

    # Step 3: Call rand()
    call    rand              # rand() result in a0

    # Step 4: Perform calculations
    sub     t0, s1, s0        # t0 = mx - mn
    addi    t0, t0, 1         # t0 = mx - mn + 1
    rem     t1, a0, t0        # t1 = rand() % (mx - mn + 1)
    add     a0, s0, t1        # a0 = mn + (rand() % (mx - mn + 1))

    # Step 5: Restore saved registers
    ld      ra, 0(sp)         # Restore return address
    ld      s0, 8(sp)         # Restore s0
    ld      s1, 16(sp)        # Restore s1

    # Step 6: Cleanup and return
    addi    sp, sp, 32        # Deallocate stack space
    ret                      # Return to caller
