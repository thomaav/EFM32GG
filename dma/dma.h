#ifndef DMA_H
#define DMA_H

struct channel_cfg {
	unsigned int cycle_ctrl : 3;
	unsigned int next_useburst : 1;
	unsigned int n_minus_1 : 10;
	unsigned int r_power : 4;
	unsigned int src_prot_ctrl : 3;
	unsigned int dst_prot_ctrl : 3;
	unsigned int src_size : 2;
	unsigned int src_inc : 2;
	unsigned int dst_size : 2;
	unsigned int dst_inc : 2;
};

struct DMA {
	uint32_t src_end;
	uint32_t dst_end;
	struct channel_cfg ctrl;
	uint32_t user;
};

struct DMA dma[32] __attribute__ ((aligned (512)));

#define BUFSIZE 1
uint32_t ping[BUFSIZE];
uint32_t pong[BUFSIZE];
bool curbuf;

void setup_DMA();

#endif
