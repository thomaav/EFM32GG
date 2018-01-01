#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define true 1
#define false 0

#define SAMPLE_RATE 44100
#define FREQUENCY 440
#define PERIOD (1 / FREQUENCY_A4)

uint32_t sample_rate = SAMPLE_RATE;
uint32_t frequency = FREQUENCY;

uint64_t get_time_usec()
{
	struct timeval current_t;
	gettimeofday(&current_t, NULL);
	return current_t.tv_sec * (int) 1e6 + current_t.tv_usec;
}

void sawtooth()
{
	uint16_t max_amplitude = (0xFFFF & 0x0FFF) / 3.0; // only 12 bits in register
	uint32_t usec_per_period = (1 / (float) frequency) * (int) 1e6;
	uint32_t samples_per_period = (SAMPLE_RATE / FREQUENCY);
	uint32_t amplitude_increase_per_sample = (max_amplitude / (float) samples_per_period);
	uint16_t usec_between_samples = (usec_per_period / (float) samples_per_period);

	uint16_t current_amplitude = 0;
	uint16_t time_slept = 0;
	while (true) {
		current_amplitude += amplitude_increase_per_sample;
		if (time_slept + usec_between_samples >= usec_per_period) {
			usleep(usec_per_period - time_slept);
			time_slept += usec_per_period - time_slept;
			time_slept = 0;
			current_amplitude = 0;
		}
		printf("%d\n", current_amplitude);
		time_slept += usec_between_samples;
		usleep(usec_between_samples);
	}
}

int main(int argc, char *argv[])
{
	sawtooth();

	return 0;
}
