#include <stdint.h>
#include <stdbool.h>

#include "dma.h"
#include "efm32gg.h"

uint32_t ping[BUFSIZE] = {0x0050005};
uint32_t pong[BUFSIZE] = {0x0};
bool curbuf = false;

void setup_DMA()
{
	*CMU_HFPERCLKEN0 |= CMU2_HFPERCLKEN0_PRS;
	*CMU_HFCORECLKEN0 |= CMU_HFCORECLKEN0_DMA;

	dma[0].src_end = (uint32_t) ping;
	dma[0].dst_end = (uint32_t) DAC0_COMBDATA;

	dma[0].ctrl.cycle_ctrl = 0b011;         // ping-pong
	dma[0].ctrl.next_useburst = 0;          // scatter-gather
	dma[0].ctrl.n_minus_1 = BUFSIZE - 1;    // cycle size
	dma[0].ctrl.r_power = 0;                // always arbitrate
	dma[0].ctrl.src_prot_ctrl = 0;          // non-privileged
	dma[0].ctrl.dst_prot_ctrl = 0;          // non-privileged
	dma[0].ctrl.src_size = 2;               // word size
	dma[0].ctrl.src_inc = 3;                // no increment
	dma[0].ctrl.dst_size = 2;               // word size
	dma[0].ctrl.dst_inc = 3;                // no increment

	// The alternate structure for channel 0 should be the same,
	// except data source
	dma[16] = dma[0];
	dma[16].src_end = (uint32_t) pong;

	*DMA_CTRLBASE = (uint32_t) (&dma[0]);   // CTRL (complete block)
	*DMA_CONFIG = 1;                        // DMA enable
	*DMA_CH0_CTRL = (0xA << 16) | 0;        // DAC0 as input source
	*DMA_CHUSEBURSTS = 1;                   // CH0 useburst setting
	*DMA_CHALTC = 1;
	*DMA_CHENS = 1;                         // CH0 enable
	*DMA_REQMASKC = 1;                      // CH0 enable peripheral requests
	*DMA_IEN = 1;                           // CH0 interrupt enable

	*PRS_CH0_CTRL = 0x1D0001;               // SOURCESEL=0x1D (TIMER1), SIGSEL=1 (OF)
}
