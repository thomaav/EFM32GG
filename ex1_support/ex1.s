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
	.long   gpio_handler            /* GPIO even handler */
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   dummy_handler
	.long   gpio_handler            /* GPIO odd handler */
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

cmu_base_addr:
	.long CMU_BASE

gpio_pa_base_addr:
	.long GPIO_PA_BASE

gpio_pc_base_addr:
	.long GPIO_PC_BASE

.globl  _reset
.type   _reset, %function
.thumb_func
_reset:
	// enable clock for GPIO controller
	ldr r1, cmu_base_addr			// get base of CMU
	ldr r2, [r1, #CMU_HFPERCLKEN0]		// current value of clock enabling register
						// as we don't want to unset enabled clocks
	mov r3, #1
	lsl r3, r3, #CMU_HFPERCLKEN0_GPIO	// left-shift to GPIO-bit
	orr r2, r2, r3				// store new value of HFPERCLKEN0
	str r2, [r1, #CMU_HFPERCLKEN0]		// store the new value with GPIO enabled

	// set high drive strength
	ldr r1, gpio_pa_base_addr		// base for GPIO
	mov r2, #0x2
	str r2, [r1, #GPIO_CTRL]		// store 0x2 in GPIO_PA_CTRL

	// set pins 8-15 to output
	ldr r2, =0x55555555
	str r2, [r1, #GPIO_MODEH]		// store 0x55555555 in GPIO_PA_MODEH

	// turn on LEDS
	ldr r2, =0xFFFF8EFF
	str r2, [r1, #GPIO_DOUT]		// store 0xFFFE0000 in GPIO_PA_DOUT

	// use GPIO_PC as base instead of PA
	ldr r1, gpio_pc_base_addr

	// set pins 0-7 to input
	ldr r2, =0x33333333
	str r2, [r1, #GPIO_MODEL] 		// store 0x33333333 in GPIO_PC_MODEL

	// enable internal pull-up
	mov r2, #0xFF
	str r2, [r1, #GPIO_DOUT]		// store 0xFF to GPIO_PC_DOUT

	// get current value of GPIO_PC_DIN
	ldr r2, [r1, #GPIO_DIN]			// load r2 with GPIO_PC_DIN offset from PC_BASE

	b .  // do nothing

/////////////////////////////////////////////////////////////////////////////
//
// GPIO handler
// The CPU will jump here when there is a GPIO interrupt
//
/////////////////////////////////////////////////////////////////////////////

.thumb_func
gpio_handler:
	b .  // do nothing

.thumb_func
dummy_handler:
	b .  // do nothing
