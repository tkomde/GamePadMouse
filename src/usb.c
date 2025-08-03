#include "usb.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include <stdint.h>
#include <stdbool.h>
#include <pico/stdlib.h>
#include <pico/cyw43_arch.h>
#include <pico/multicore.h>
#include <pico/async_context.h>
#include <bsp/board_api.h>

static MouseOutReport mouse_report;

// send mouse report to host
void
send_mouse_report(const MouseOutReport *report)
{
	tud_hid_mouse_report(REPORT_ID_MOUSE,
	                     report->buttons,
	                     report->x,
	                     report->y,
	                     report->wheel,
	                     0);
}

// send empty mouse report to host
void
empty_mouse_report(void)
{
	tud_hid_mouse_report(REPORT_ID_MOUSE, 0, 0, 0, 0, 0);
}


void
usb_core_task()
{
	board_init();

	tusb_init();

	// TinyUSB board init callback after init
	if (board_init_after_tusb) {
		board_init_after_tusb();
	}

	stdio_init_all();

	while (1) {
		tud_task();
		if (tud_suspended()) {
			tud_remote_wakeup();
			continue;
		}
	}
}

// (unused now) callback when data is received on a CDC interface
/*
void
tud_cdc_rx_cb(uint8_t itf)
{
        // allocate buffer for the data in the stack
        uint8_t buf[CFG_TUD_CDC_RX_BUFSIZE];

        logi("RX CDC %d\n", itf);

        // read the available data
        // | IMPORTANT: also do this for CDC0 because otherwise
        // | you won't be able to print anymore to CDC0
        // | next time this function is called
        uint32_t count = tud_cdc_n_read(itf, buf, sizeof(buf));

        // check if the data was received on the second cdc interface
        if (itf == 1) {
                // process the received data
                buf[count] = 0;  // null-terminate the string
                // now echo data back to the console on CDC 0
                logi("Received on CDC 1: %s\n", buf);

                // and echo back OK on CDC 1
                tud_cdc_n_write(itf, (uint8_t const *) "OK\r\n", 4);
                tud_cdc_n_write_flush(itf);
        }
}
*/