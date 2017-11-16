#pragma once

#define BLACK   (0x0000)
#define WHITE   (0xFFFF)
#define RED     (0xF000)
#define GREEN   (0x0FF0)
#define BLUE    (0x000F)

uint16_t rgb888_to_rgb565(uint8_t r, uint8_t g, uint8_t b);
void __nanosleep(const struct timespec *req, struct timespec *rem);
void __ssleep(int sec);
