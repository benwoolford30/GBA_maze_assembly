@asm source code 
/*
assembly commands used:
add
sub
mov
mul
cmp
bne
bl
bx
str
ldr
push
pop
blo
labels:
function calls
both asm to asm
asm to c

*/
.THUMB	@set asm to thumb mode			
.ALIGN  2			
.GLOBL  generatemazeasm	@declare asm generate maze function	
.GLOBL	drawtileasm		@declare draw tile asm	
.extern drawtile		@get c function so can call in assembly
# struct fourway
# {
	# int up;	#32
	# int left;	#36
	# int right;#40
	# int down; #44
# };
# struct cell
# {
	# /*
		# int byte size is 4
		# bool byte size is 1
		# cell* byte size is 4 ()
		# fourway byte size is 4
	# */
	
	# int x;	//offset 0	0-3	
	# int y;	//offset #4	4-7	
	# int visited; //offset# 8 8-11
	# int current; //offset #12-15 
	# struct cell* neighbour[4];
	# // n1 offset #16
	# // n2 offset #20
	# // n3 offset #24
	# // n4 offset #28
	# /*
	# 0= left
	# 1= up
	# 2= right
	# 3=down
	# */
	# struct fourway wall;
	# // up offset #32
	# // left offset #36
	# // right offset #40
	# // down offset #44
	
# };
.THUMB_FUNC			
generatemazeasm: @start of function generatemazeasm		
push { r4-r7, lr }	
@	to access a 4,1 in a 2 dimensional array 
@	need to do formula @ [((byte size*x)* array size )+(y*byte size)]
@ 	my cell struct byte size is 48
mov r3,#0	@x i
mov r4,#0	@y i2
loopa:		@for loop i
loopb:		@for loop i2
mov r6,#9	@array size
mov r7,#48	@struct size is 48
mul r7,r7,r3	@byte size * x
mul r6,r6,r7	@ * by array size
mov r2,r4		@set r2 to r4 (i2)
mov r7,#48		@set r7 back to byte size 48
mul r2,r2,r7	@multiply i2 by byte size
add r6,r2	@add y for offset
ldr r5,[r0,r6]	@load space[i][i2].x
mov r2,r6		@store offset in r2
mov r5,r3		@add i onto x
str r5,[r0,r6]	@initialise x
add r6,#4		@offset to y
ldr r5,[r0,r6]	@load y

mov r5,r4		@add y

str r5,[r0,r6]	@initialise y
push {r0}		@push begining of maze 2d array ptr
push {r1}		
push {r2}
push {r3}
push {r4}	@push previous registers onto stack
sub r6,#4
mov r2,r6

bl drawtileasm	@call space initialiser
pop {r4}
pop {r3}
pop {r2}
pop {r1}
pop {r0}	@pop stack and get old registers back
add r4,#1	@increment r4
cmp r4,#9	@check r4 is at end of loop

blo loopa @ r4/i2	@branch if result was lower

mov r4,#0
add r3,#1
cmp r3,#9	@check r3 is end of loop
blo loopb @i/r3	@branch if result was lower

pop { r4-r7 }		
pop { r3 }			
bx r3			@end of function generatemazeasm


.THUMB_FUNC			
drawtileasm:			@ function starts here
push { r4-r7, lr }	
ldr r4, [r0,r2]		@load space.x
mov r5,#2			@store a value of two in register 5
mul r4,r4,r5		@multiply space.x by 2
add r4,#5			@offset it by 5
str r4,[r0,r2]			@store back in ptr
add r2,#4
ldr r4,[r0,r2]		@repeat process for space.y
mul r4,r4,r5		
add r4,#1
str r4,[r0,r2]		@store yvalue
push {r0}
push {r1}
push {r2}
sub r2,#4
ldr r1,[r0,r2]		@set values in registers r1 and r2 to x and y
mov r2,r4		@Not in r0 because function returns a value into register r0
bl drawtile			@call drawtile function from asm_example.c
pop {r2}
pop {r1}
pop {r0}
mov r4,#0			@set r4 to 0 for false
mov r6,#4
init:			@Set all other values to false or NULL
add r6,#4
add r2,#4
str r4,[r0,r2]		@initialise  value to false or null
cmp r6,#44		@check to see if we've reached end of struct
bne init		@check we've reached end of struct 
@attempted to draw to background in assembly however tiles were 
@skipping a tile space in offset and couldn't address this 


pop { r4-r7 }		
pop { r3 }			

bx r3 				

