# Program Name: Fucntion Program
# Student Name: Ar-Raniry Ar-Rasyid
# Net ID: jzr266
# Student ID: 000-663-921
# Program Description: Assemby program that replicates the function below
# Preface: The code is from Stephen Marz's walkthrough. 

# int64_t get_rand(int64_t mn, int64_t mx) {
#    return mn + rand() % (mx - mn + 1);
# }

.section .text
.global sum_prod

# Registers:
# a0 get_rand(a0 mn, a1 mx) {
#    return mn + rand() % (mx - mn + 1);
# }

get rand:
addi    sp, sp, -32      # ra + s0 + s1 = 24 bytes aligned to 32
# Above is stack allocation

sd      ra, 0(sp)        # Save return address
sd      s0, 8(sp)        # Save old s0
sd      s1, 16(sp)       # Save old s1
# Above saves the aforementioned registers and below stores arguments

mv      s0, a0           # minimum
mv      s1, a1           # maximum

call     rand             # rand()
# Above gets a random number

sub      t0, s1, s0      # t0 = mx - mn
addi     t0, t0, 1       # t0 = mx - mn + 1

rem      t1, a0, t0       # t1 = rand() % (mx - mn + 1)
# Helps with offset

add      a0, s0, t1       # a0 = mn + rand() % (mx - mn + 1)

ld       ra, 0(sp)         # restore return address
ld       s0, 8(sp)         # restore original s0
ld       s1, 16(sp)        # restore original s1

addi      sp, sp, 32       # Move stack back (deallocate)
# Deallocation and returning 

ret