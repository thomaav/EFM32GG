#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"
#include "audio.h"

/*
 * TODO calculate the appropriate sample period for the sound wave(s) you 
 * want to generate. The core clock (which the timer clock is derived
 * from) runs at 14 MHz by default. Also remember that the timer counter
 * registers are 16 bits. 
 */
/*
 * The period between sound samples, in clock cycles 
 */
#define   SAMPLE_PERIOD   0

/*
 * Declaration of peripheral setup functions 
 */
void setupGPIO();
void setupTimer(uint32_t period);
void setupDAC();
void setupNVIC();

uint16_t max_amplitude = 0xF;

// non-global audio values
uint16_t current_treble_note = 0;
uint16_t current_bass_note = 0;
int16_t msec_left = 0;
_Bool square_high_treble = 0;
_Bool square_high_bass = 0;
uint16_t tick_counter = 0;

// toggle for button press, as we cannot get interrupted within
// another interrupt
_Bool button_press = 0;
uint32_t buttons_pressed;

int main(void)
{
	// setup all peripherals
	setupGPIO();
	setupDAC();
	setupTimer(SAMPLE_PERIOD);

	// enable interrupt handling
	setupNVIC();

	/*
	 * TODO for higher energy efficiency, sleep while waiting for
	 * interrupts instead of infinite loop for busy-waiting 
	 */
	while (true) {
		if (button_press) {
			button_press = 0;
			if (buttons_pressed == 0xFE) {
				laser_shot();
			} else if (buttons_pressed == 0xFD) {
				explosion();
			} else if (buttons_pressed == 0xFB) {
				mario_game_over();
			} else if (buttons_pressed == 0xF7) {
				windows_xp_startup();
			}
		}
	}

	return 0;
}

void setupNVIC()
{
	// to enable GPIO-interrupts, write bits 1 and 11
	*ISER0 |= 0x802;

	// to enable timer-interrupts, write bit 12
	*ISER0 |= (1 << 12);
}

/*
 * if other interrupt handlers are needed, use the following names:
 * NMI_Handler HardFault_Handler MemManage_Handler BusFault_Handler
 * UsageFault_Handler Reserved7_Handler Reserved8_Handler
 * Reserved9_Handler Reserved10_Handler SVC_Handler DebugMon_Handler
 * Reserved13_Handler PendSV_Handler SysTick_Handler DMA_IRQHandler
 * GPIO_EVEN_IRQHandler TIMER0_IRQHandler USART0_RX_IRQHandler
 * USART0_TX_IRQHandler USB_IRQHandler ACMP0_IRQHandler ADC0_IRQHandler
 * DAC0_IRQHandler I2C0_IRQHandler I2C1_IRQHandler GPIO_ODD_IRQHandler
 * TIMER1_IRQHandler TIMER2_IRQHandler TIMER3_IRQHandler
 * USART1_RX_IRQHandler USART1_TX_IRQHandler LESENSE_IRQHandler
 * USART2_RX_IRQHandler USART2_TX_IRQHandler UART0_RX_IRQHandler
 * UART0_TX_IRQHandler UART1_RX_IRQHandler UART1_TX_IRQHandler
 * LEUART0_IRQHandler LEUART1_IRQHandler LETIMER0_IRQHandler
 * PCNT0_IRQHandler PCNT1_IRQHandler PCNT2_IRQHandler RTC_IRQHandler
 * BURTC_IRQHandler CMU_IRQHandler VCMP_IRQHandler LCD_IRQHandler
 * MSC_IRQHandler AES_IRQHandler EBI_IRQHandler EMU_IRQHandler 
 */
