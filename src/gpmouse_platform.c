#include "usb.h"
#include <stdio.h>
#include <string.h>
#include <pico/cyw43_arch.h>
#include <pico/multicore.h>
#include <pico/async_context.h>
#include <uni.h>
#include "sdkconfig.h"
#include "uni_hid_device.h"
#include "uni_log.h"

// Sanity check
#ifndef CONFIG_BLUEPAD32_PLATFORM_CUSTOM
#error "Pico W must use BLUEPAD32_PLATFORM_CUSTOM"
#endif

#define AXIS_DEADZONE 40
#define SPEED_COEFF 50
#define SCROLL_INTERVAL 6

// Residual value for mouse movement
int16_t residual_mouse_x = 0;
int16_t residual_mouse_y = 0;

// Previous button state
bool former_mouse_left_button = false;
bool former_mouse_right_button = false;

// Counter
uint32_t counter = 0;

MouseOutReport mouse_report;
uint8_t connected_controllers;

int16_t
sign(int16_t val)
{
	return (val > 0) - (val < 0);
}

static void
set_led_status()
{
	if (connected_controllers == 0)
		cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
	else
		cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
}

//
// Platform Overrides
//

static void
gpmouse_platform_init(int argc, const char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	logi("my_platform: init()\n");
	connected_controllers = 0;
	empty_mouse_report();
	// USB mouse initialization is OK with tinyusb initialization
}


static void
gpmouse_platform_on_init_complete(void)
{
	logi("my_platform: on_init_complete()\n");
	uni_bt_enable_new_connections_unsafe(true);
	if (1)
		uni_bt_del_keys_unsafe();
	else
		uni_bt_list_keys_unsafe();
	cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
	logi("BLUEPAD: ready to fill mouse reports");
	multicore_fifo_push_blocking(0);  // signal the other core to start reading
}


static void
gpmouse_platform_on_device_connected(uni_hid_device_t *d)
{
	logi("my_platform: device connected: %p\n", d);
}


static void
gpmouse_platform_on_device_disconnected(uni_hid_device_t *d)
{
	logi("my_platform: device disconnected: %p\n", d);
	empty_mouse_report();
	connected_controllers--;
	set_led_status();
}


static uni_error_t
gpmouse_platform_on_device_ready(uni_hid_device_t *d)
{
	logi("my_platform: device ready: %p\n", d);
	connected_controllers++;
	set_led_status();
	return UNI_ERROR_SUCCESS;
}


static void
gpmouse_platform_on_controller_data(uni_hid_device_t *d, uni_controller_t *ctl)
{
	// required for scrolling
	counter++;
	// Uncomment to check if data is being received
	// Print device Id before dumping gamepad.
	// logi("(%p) id=%d ", d, uni_hid_device_get_idx_for_instance(d));
	// uni_controller_dump(ctl);

	if (ctl->klass != UNI_CONTROLLER_CLASS_GAMEPAD) {
		return;
	}
	uni_gamepad_t *gp = &ctl->gamepad;

	// Detect cursor movement
	int16_t move_x_total =
	        // If less than deadzone
	        gp->axis_x < AXIS_DEADZONE && gp->axis_x > -AXIS_DEADZONE
	                ? 0
	                // If greater than deadzone, add value above deadzone and residual
	                : residual_mouse_x + gp->axis_x -
	                          AXIS_DEADZONE * sign(gp->axis_x);
	int16_t move_y_total =
	        // If less than deadzone
	        gp->axis_y < AXIS_DEADZONE && gp->axis_y > -AXIS_DEADZONE
	                ? 0
	                // If greater than deadzone, add value above deadzone and residual
	                : residual_mouse_y + gp->axis_y -
	                          AXIS_DEADZONE * sign(gp->axis_y);

	// Adjust mouse movement amount
	residual_mouse_x = move_x_total % SPEED_COEFF;
	int8_t move_x =
	        (int8_t) ((move_x_total - residual_mouse_x) / SPEED_COEFF);

	residual_mouse_y = move_y_total % SPEED_COEFF;
	int8_t move_y =
	        (int8_t) ((move_y_total - residual_mouse_y) / SPEED_COEFF);

	// add acceleration based on speed
	int16_t speed = move_x * move_x + move_y * move_y;
	int8_t cursor_accel = speed > 50 ? 7 : speed > 30 ? 3 : 1;

	move_x *= cursor_accel;
	move_y *= cursor_accel;

	// Update button state
	// BUTTON_SHOULDER_L: upper side
	// BUTTON_SHOULDER_R: lower side
	// BUTTON_A: right
	// BUTTON_B: down
	// BUTTON_Y: right
	// BUTTON_X: up
	// if (gp->buttons & BUTTON_A) {  // front
	//      logi("BUTTON_A\n");
	//}
	bool left_button_pressed = (gp->buttons & BUTTON_TRIGGER_R) > 0;
	bool right_button_pressed = (gp->buttons & BUTTON_TRIGGER_L) > 0;

	// should be larger values for windows/android (eg: 2, -2)
	int8_t scroll_amount = (gp->buttons & BUTTON_X) > 0   ? 1
	                       : (gp->buttons & BUTTON_B) > 0 ? -1
	                                                      : 0;

	if (move_x_total == 0 && move_y_total == 0 &&
	    left_button_pressed == former_mouse_left_button &&
	    right_button_pressed == former_mouse_right_button &&
	    scroll_amount == 0) {
	} else {
		mouse_report.y = move_x;
		mouse_report.x = -move_y;
		mouse_report.buttons =
		        left_button_pressed + right_button_pressed * 2;

		mouse_report.wheel =
		        counter % SCROLL_INTERVAL == 0
		                ? scroll_amount
		                : 0;  // scroll amount every SCROLL_INTERVAL
		/*logi("mouse_report: x=%d, y=%d, buttons=0x%02x\n",
		               mouse_report.x,
		               mouse_report.y,
		               mouse_report.buttons);*/
		send_mouse_report(&mouse_report);
	}

	former_mouse_left_button = left_button_pressed;
	former_mouse_right_button = right_button_pressed;
}


static const uni_property_t *
gpmouse_platform_get_property(uni_property_idx_t idx)
{
	ARG_UNUSED(idx);
	return NULL;
}


static void
gpmouse_platform_on_oob_event(uni_platform_oob_event_t event, void *data)
{
	ARG_UNUSED(event);
	ARG_UNUSED(data);
	return;
}


// Entry Point
struct uni_platform *
get_my_platform(void)
{
	static struct uni_platform plat = {
		.name = "My Platform",
		.init = gpmouse_platform_init,
		.on_init_complete = gpmouse_platform_on_init_complete,
		.on_device_connected = gpmouse_platform_on_device_connected,
		.on_device_disconnected = gpmouse_platform_on_device_disconnected,
		.on_device_ready = gpmouse_platform_on_device_ready,
		.on_oob_event = gpmouse_platform_on_oob_event,
		.on_controller_data = gpmouse_platform_on_controller_data,
		.get_property = gpmouse_platform_get_property,
	};
	return &plat;
}
