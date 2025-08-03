
#ifndef _USB_H_
#define _USB_H_

#include <stdint.h>

// USB mouse report structure
typedef struct {
	uint8_t buttons;  // bit0: left, bit1: right, bit2: middle
	int8_t x;         // X movement
	int8_t y;         // Y movement
	int8_t wheel;     // Wheel movement
} MouseOutReport;

void send_mouse_report(const MouseOutReport *report);
void empty_mouse_report(void);
void usb_core_task(void);
// void tud_cdc_rx_cb(uint8_t itf);

#endif