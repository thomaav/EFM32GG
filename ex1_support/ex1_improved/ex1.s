.syntax unified
.include "efm32gg.s"

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
	// BUTTON_STATE = r7
	// BINARY_COUNTER = r8
	// if r11 is set to 1, this means this is the initial
	// interrupt that we want to ignore
	ldr r5, =GPIO_PA_BASE			// base for GPIO
	ldr r6, =GPIO_PC_BASE
	mov r7, #0
	mov r8, #0
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

	// setup for energy mode
	ldr r1, =SCR
	mov r2, #0
	mov r3, #0x6
	str r3, [r1, r2]

loop:
	wfi
	b loop

.thumb_func
read_buttons:
	// if r11 is set to 1, this means this is the initial
	// interrupt that we want to ignore
	cmp r11, #1
	itt eq
	moveq r11, #0
	bxeq lr					// jump back to main loop

	mov r3, #GPIO_DIN			// get current values of buttons
	ldr r2, [r6, r3]

	ldr r1, =SW1				// button (SW1), left for decrement
	ldr r3, =SW3				// button (SW3), right for increment

	cmp r2, r1				// decrement
	it eq
	subeq r8, r8, #1

	cmp r2, r3				// increment
	it eq
	addeq r8, r8, #1

	ldr r1, =SW2				// check if reset button (SW2) is pushed
	cmp r2, r1
	it eq					// if pushed, reset the counter
	moveq r8, #0

	lsl r2, r8, #8				// display binary counter on leds
	ldr r1, =0xFF00
	eor r2, r2, r1
	mov r3, #GPIO_DOUT
	str r2, [r5, r3]

	// clear interrupt flag
	ldr r1, =GPIO_BASE
	mov r2, #GPIO_IF
	mov r3, #GPIO_IFC
	ldr r4, [r1, r2]
	str r4, [r1, r3]

	bx lr

.thumb_func
dummy_handler:
	b .
