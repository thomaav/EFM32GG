#define LED1 (1 << 8)
#define LED2 (1 << 9)
#define LED3 (1 << 10)
#define LED4 (1 << 11)
#define LED5 (1 << 12)
#define LED6 (1 << 13)
#define LED7 (1 << 14)
#define LED8 (1 << 15)
#define ALL (0xFFFF)

void toggle_led(uint32_t led);
void enable_led(uint32_t led);
void disable_led(uint32_t led);
