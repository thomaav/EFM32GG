#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"
#include "audio.h"

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

// initialize a sound player, and an empty one for no sound
struct player sound_player;
struct melody empty_melody;

int main(void)
{
	// setup all peripherals
	setupGPIO();
	setupDAC();
	setupTimer(SAMPLE_PERIOD);

	// enable interrupt handling
	setupNVIC();

	// initialize all music
	setup_melodies();

	// startup with windows_xp
	set_current_melody(&sound_player, windows_xp_startup_melody);

	/*
	 * TODO for higher energy efficiency, sleep while waiting for
	 * interrupts instead of infinite loop for busy-waiting
	 */
	while (true) {
		__asm__("wfi");
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
