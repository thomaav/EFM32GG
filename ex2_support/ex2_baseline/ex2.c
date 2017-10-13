#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

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

// how loud?
uint16_t max_amplitude = 0xF;

// variables to keep track of square wave
_Bool square_high_treble = 0;
uint16_t tick_counter = 0;

// toggle for button press, as we cannot get interrupted within
// another interrupt
_Bool button_press = 0;
uint32_t buttons_pressed;

// current playing song
static const struct melody empty_melody;
struct melody current_melody;

void tick()
{
	// pretend we are an interrupt handler with busy-waiting,
	// using the value of TIMER1_CNT would be absurd, as most
	// instructions don't take exactly one clock cycle, thus
	// making it very imprecise. instead we check whether the
	// overflow bit is set, and then clear said flag
	if (*TIMER1_IF & 0x1) {
		++tick_counter;
		*TIMER1_IFC = 0x1;
	} else {
		return;
	}

	// test that all is good by blinking LEDs every second
	if (tick_counter == SAMPLE_RATE) {
		tick_counter = 0;
		*GPIO_PA_DOUT ^= 0xFFFF;
	}

	// set current_note every tick for simplicity
	if (!current_melody.notes) {
		return;
	}

	// count msec every 10th msec (1/100th of the sample rate)
	if (!(tick_counter % (SAMPLE_RATE / 100))) {
		if (current_melody.msec_left > 0) {
			// don't let msec left of note become negative, to allow for
			// values that that 10 does not divide
			if (current_melody.msec_left - 10 <= 0) {
				// we need a new note, or nothing
				if (current_melody.current_note_idx < current_melody.length) {
					++current_melody.current_note_idx;
					uint16_t next_note_idx = ++current_melody.current_note_length_idx;
					uint16_t next_note_length = current_melody.note_lengths[next_note_idx];
					current_melody.msec_left = next_note_length;
				} else {
					// we are done with the song/melody
					current_melody = empty_melody;
				}
			} else {
				current_melody.msec_left -= 10;
			}
		}
	}

	uint16_t samples_per_period_treble = SAMPLE_RATE / current_melody.notes[current_melody.current_note_idx];

	// flip between low and high square every half period
	if (!(tick_counter % (samples_per_period_treble / 2))) {
		square_high_treble = !square_high_treble;
	}

	// create a square sound function
	if (square_high_treble) {
		*DAC0_CH0DATA = max_amplitude;
	} else {
		*DAC0_CH0DATA = 0x000;
	}
}

int main(void)
{
	struct melody lisa_melody = create_melody(lisa_notes, lisa_note_lengths, lisa_length);

	// setup all peripherals
	setupGPIO();
	setupDAC();
	setupTimer(SAMPLE_PERIOD);

	// enable interrupt handling
	setupNVIC();

	// initialize a song to test
	current_melody = lisa_melody;

	while (true) {
		tick();
	}

	return 0;
}

void setupNVIC()
{
	// to enable GPIO-interrupts, write bits 1 and 11
	*ISER0 |= 0x802;

	// to enable timer-interrupts, write bit 12
	/* *ISER0 |= (1 << 12); */
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
