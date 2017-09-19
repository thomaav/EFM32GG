.syntax unified
.include "../core/efm32gg.s"

/////////////////////////////////////////////////////////////////////////////
//
// Exception vector table
// This table contains addresses for all exception handlers
//
/////////////////////////////////////////////////////////////////////////////

.section .vectors

	.long   stack_top               /* Top of Stack                 */
	.long   _reset                  /* Reset Handler                */
	.long   dummy_handler           /* NMI Handler                  */
	.long   dummy_handler           /* Hard Fault Handler           */
	.long   dummy_handler           /* MPU Fault Handler            */
	.long   dummy_handler           /* Bus Fault Handler            */
	.long   dummy_handler           /* Usage Fault Handler          */
	.long   dummy_handler           /* Reserved                     */
	.long   dummy_handler           /* Reserved                     */
	.long   dummy_handler           /* Reserved                     */
	.long   dummy_handler           /* Reserved                     */
	.long   dummy_handler           /* SVCall Handler               */
	.long   dummy_handler           /* Debug Monitor Handler        */
	.long   dummy_handler           /* Reserved                     */
	.long   dummy_handler           /* PendSV Handler               */
	.long   dummy_handler           /* SysTick Handler              */

	/* External Interrupts */
	.long   dummy_handler
	.long   read_buttons            /* GPIO even handler */
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   read_buttons            /* GPIO odd handler */
	.long   timer_interrupt /* timer handler */
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler

/////////////////////////////////////////////////////////////////////////////
//
// Reset handler
// The CPU will start executing here after a reset
//
/////////////////////////////////////////////////////////////////////////////

.section .text

.globl  _reset
.type   _reset, %function
.thumb_func
_reset:
	// GPIO_PA_BASE = r5
	// GPIO_PC_BASE = r6
	// r11 is used to ignore first interrupt, as it fires every startup
	ldr r5, =GPIO_PA_BASE			// base for GPIO
	ldr r6, =GPIO_PC_BASE
	mov r10, #0
	mov r11, #1

	//
	// setup for GPIO
	//
	ldr r1, =CMU_BASE			// enable GPIO clock here
	mov r3, #CMU_HFPERCLKEN0
	ldr r2, [r1, r3]			// current value of clock enabling register
	mov r3, #1				// as we don't want to unset enabled clocks
	lsl r3, r3, #CMU_HFPERCLKEN0_GPIO	// left-shift to GPIO-bit
	orr r2, r2, r3				// store new value of HFPERCLKEN0
	str r2, [r1, #CMU_HFPERCLKEN0]		// store the new value with GPIO enabled

	// set high drive strength for LEDs
	mov r2, #0x2
	str r2, [r5, #GPIO_CTRL]		// store 0x2 in GPIO_PA_CTRL

	// set pins 8-15 to output
	ldr r2, =0x55555555
	str r2, [r5, #GPIO_MODEH]		// store 0x55555555 in GPIO_PA_MODEH

	// turn on LEDS (use for initial LED values)
	ldr r2, =0xFFFFF7FF
	str r2, [r5, #GPIO_DOUT]

	// set pins 0-7 to input
	ldr r2, =0x33333333
	str r2, [r6, #GPIO_MODEL] 		// store 0x33333333 in GPIO_PC_MODEL

	// enable internal pull-up
	mov r2, #0xFF
	str r2, [r6, #GPIO_DOUT]		// store 0xFF to GPIO_PC_DOUT

	//
	// setup for interrupts
	//
	ldr r1, =GPIO_BASE			// write 0x22222222 to GPIO_EXTIPSELL
	mov r2, #GPIO_EXTIPSELL
	ldr r3, =0x22222222
	str r3, [r1, r2]

	mov r2, #GPIO_EXTIFALL			// enable 1->0 transition interrupt
	mov r3, #0xFF
	str r3, [r1, r2]

	mov r2, #GPIO_IEN			// enable interrupt generation in IEN
	mov r3, #0xFF
	str r3, [r1, r2]

	ldr r2, =ISER0				// enable interrupt handling
	ldr r3, =0x802
	str r3, [r2, #0]

	//
	// setup for energy mode
	//
	ldr r1, =SCR
	mov r2, #0
	mov r3, #0x6
	str r3, [r1, r2]

	//
	// setup for systick
	//
	ldr r1, =CMU_BASE			// enable clock timer by setting bit 6 in HFPERCLKEN0
	mov r3, #CMU_HFPERCLKEN0
	ldr r2, [r1, r3]
	mov r3, #1				// as we don't want to unset enabled clocks
	lsl r3, r3, #6
	orr r2, r2, r3
	mov r3, #CMU_HFPERCLKEN0
	str r2, [r1, r3]

	ldr r1, =TIMER1_BASE			// store 14MHz in timer1_top for length
	mov r2, #TIMER1_TOP			// between interrupts
	ldr r3, =0xFFFF
	str r3, [r1, r2]

	mov r2, #TIMER1_IEN			// store 1 in timer1_ien to enable timer interrupt
	mov r3, #1
	str r3, [r1, r2]

	ldr r2, =ISER0				// enable interrupt handling of timer by also setting
	ldr r3, =0x1802				// bit 12
	str r3, [r2, #0]

	ldr r1, =TIMER1_BASE
	mov r2, #TIMER1_CMD
	mov r3, #1
	str r3, [r1, r2]
	/*
	*/
loop:
	// a way set all leds directly from buttons, perhaps useful later
	// lsl r2, r2, #0x8			// left shift value of buttons to fit LEDs
	// ldr r3, =0xFFFF00FF			// set all bits outside 8-15 to high
	// orr r2, r2, r3			//

	// wfi
	orr r1, r1, r1
	b loop

// handler for system clock interrupts

.thumb_func
timer_interrupt:
/*
	mov r3, #GPIO_DIN			// get current values of buttons
	ldr r2, [r6, r3]

	lsl r2, r2, #0x8			// left shift value of buttons to fit LEDs
	ldr r3, =0xFFFF00FF			// set all bits outside 8-15 to high
	orr r2, r2, r3				//

	// store r1 back to GPIO_DOUT for LEDs
	mov r2, #GPIO_DOUT
	str r1, [r5, r2]
	*/

	ldr r1, =TIMER1_BASE			// clear interrupt flag
	mov r2, #TIMER1_IFC
	mov r3, #1
	str r3, [r1, r2]

	add r10, #1
	cmp r10, #1000
	it ne
	bne return

	mov r10, #0
	ldr r1, =0xFFFFFFFF
	mov r2, #GPIO_DOUT
	str r1, [r5, r2]

return:
	bx lr

// INFO:
// it seems the buttons are originally initialized as 0xF7,
// having bit 4 unset - keep this in mind as it is probably
// what is firing the first interrupt
.thumb_func
read_buttons:
	// if r11 is set to 1, this means this is the initial
	// interrupt that we want to ignore
	cmp r11, #1
	itt eq
	moveq r11, #0
	bxeq lr					// jump back to main loop

	// first read current active LED to r1 and OR with 0xFFFF in
	// higher order bits, as the microcontroller seemingly refuses
	// to set it
	mov r3, #GPIO_DOUT
	ldr r1, [r5, r3]
	ldr r3, =0xFFFF00FF
	orr r1, r1, r3

	mov r3, #GPIO_DIN			// get current values of buttons
	ldr r2, [r6, r3]

	ldr r3, =0x000000FE			// if only button 1 is pressed
	ldr r4, =0x000000FB			// if only button 3 is pressed

	// check if left button is pushed
	cmp r2, r3
	it ne
	bne left_not_pushed			// skip code for pushing left button

	// left was pushed
	lsr r1, r1, #1				// right shift LED light
	ldr r3, =0x0000FF00			// handle overflow
	and r3, r3, r1
	ldr r7, =0xFF00				// if all LED unset, then we have 0xFF00 here
	cmp r3, r7
	it eq
	ldreq r1, =0xFFFF7FFF

left_not_pushed:
	// check if right button is pushed
	cmp r2, r4
	it ne
	bne right_not_pushed			// skip code for pushing right button
	lsl r1, r1, #1
	ldr r3, =0x0000FF00			// handle overflow
	and r3, r3, r1
	ldr r7, =0xFF00				// if all LED unset, then we have 0xFF00 here
	cmp r3, r7
	it eq
	ldreq r1, =0xFFFFFEFF

right_not_pushed:
	// store r1 back to GPIO_DOUT for LEDs
	mov r2, #GPIO_DOUT
	str r1, [r5, r2]

	// clear interrupt flag
	ldr r1, =GPIO_BASE
	mov r2, #GPIO_IF
	mov r3, #GPIO_IFC
	ldr r4, [r1, r2]
	str r4, [r1, r3]

	bx lr

.thumb_func
dummy_handler:
	bx lr
