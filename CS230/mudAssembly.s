# Program Name: MUD Lab
# Student Name: Ar-Raniry Ar-Rasyid
# Net ID: jzr266
# Student ID: 000-663-921
# Program Description: A little game in the console

# String literals that we're going to use
.section .rodata
exit_string: .asciz "%s\n%s\nExits: "
# Above is taken from the write up; Below is the directions alongside newline
n_string:    .asciz "n "
e_string:    .asciz "e "
s_string:    .asciz "s "
w_string:    .asciz "w "
newline:     .asciz "\n"

# The functions we create
.section .text
.global look_at_room
.global look_at_all_rooms
.global move_to

look_at_room:
    # Make room on the stack and saving return address
    addi    sp, sp, -16
    sd      ra, 8(sp)
    sd      s0, 0(sp)
    mv      s0, a0

    # Load information on the room
    # Printf is used to show it to the user
    la      a0, exit_string
    ld      a1, 0(s0)
    ld      a2, 8(s0)
    call    printf
    
    # Load in the exits, if not applicable skip
    lw      t0, 16(s0)
    li      t1, -1
    beq     t0, t1, east
    la      a0, n_string
    call    printf
    
east:
    # This loads in the East exit
    lw      t0, 20(s0)
    li      t1, -1
    beq     t0, t1, south
    la      a0, e_string
    call    printf
    
south:
    # This loads in the South exit
    lw      t0, 24(s0)
    li      t1, -1
    beq     t0, t1, west
    la      a0, s_string
    call    printf
    
west:
    # This loads in the West exit
    lw      t0, 28(s0)
    li      t1, -1
    beq     t0, t1, print_newline
    la      a0, w_string
    call    printf
    
print_newline:
    # This loads in a newline
    la      a0, newline
    call    printf
    
    # Below cleans up the stack
    ld      s0, 0(sp)
    ld      ra, 8(sp)
    addi    sp, sp, 16
    ret

look_at_all_rooms:
    # Make space on the stack and saves registers
    addi    sp, sp, -32
    sd      ra, 24(sp)
    sd      s0, 16(sp)
    sd      s1, 8(sp)
    sd      s2, 0(sp)
    
    # Variables for number of rooms, a counter, and array for rooms
    mv      s0, a0
    mv      s1, a1
    li      s2, 0
    
start_loop:
    # For loop
    bge     s2, s1, done_loop
    
    # Room printing
    slli    t0, s2, 5
    add     a0, s0, t0
    call    look_at_room
    la      a0, newline
    call    printf
    
    # Increment
    addi    s2, s2, 1
    j       start_loop
    
done_loop:
    # Once loop finishes, revert everything
    ld      s2, 0(sp)
    ld      s1, 8(sp)
    ld      s0, 16(sp)
    ld      ra, 24(sp)
    addi    sp, sp, 32
    ret

move_to:
    # Determine the room the player would move to 
    slli    t0, a2, 2
    addi    t1, a1, 16
    add     t1, t1, t0
    lw      t0, 0(t1)

    # If nothing is there return null
    li      t1, -1
    beq     t0, t1, null
    
    # If found output it
    slli    t0, t0, 5
    add     a0, a0, t0
    ret
    
null:
    # Just a null return
    li      a0, 0
    ret
