#include "tusb.h"
#include <bsp/board_api.h>
#include "usb_descriptors.h"

#define _PID_MAP(itf, n) ((CFG_TUD_##itf) << (n))
#define CDC_EXAMPLE_VID 0xCafe
// use _PID_MAP to generate unique PID for each interface
#define CDC_EXAMPLE_PID (0x4000 | _PID_MAP(CDC, 0))
// set USB 2.0
#define CDC_EXAMPLE_BCD 0x0200

// defines a descriptor that will be communicated to the host
tusb_desc_device_t const desc_device = {
	.bLength = sizeof(tusb_desc_device_t),
	.bDescriptorType = TUSB_DESC_DEVICE,
	.bcdUSB = CDC_EXAMPLE_BCD,

	.bDeviceClass =
	        0x00,  // TUSB_CLASS_MISC,          // CDC is a subclass of misc
	.bDeviceSubClass =
	        0x00,  // MISC_SUBCLASS_COMMON,  // CDC uses common subclass
	.bDeviceProtocol = 0x00,  // MISC_PROTOCOL_IAD,     // CDC uses IAD

	.bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,  // 64 bytes

	.idVendor = CDC_EXAMPLE_VID,
	.idProduct = CDC_EXAMPLE_PID,
	.bcdDevice = 0x0100,  // Device release number

	.iManufacturer = 0x01,  // Index of manufacturer string
	.iProduct = 0x02,       // Index of product string
	.iSerialNumber = 0x03,  // Index of serial number string

	.bNumConfigurations = 0x01  // 1 configuration
};

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const *
tud_descriptor_device_cb(void)
{
	return (uint8_t const *) &desc_device;
}

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+


uint8_t const desc_hid_report[] = { TUD_HID_REPORT_DESC_MOUSE(
	HID_REPORT_ID(REPORT_ID_MOUSE)) };

// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *
tud_hid_descriptor_report_cb(uint8_t instance)
{
	(void) instance;
	return desc_hid_report;
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

enum { ITF_NUM_HID, ITF_NUM_CDC, ITF_NUM_CDC_DATA, ITF_NUM_TOTAL };

#define CONFIG_TOTAL_LEN                                                       \
	(TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN + CFG_TUD_CDC * TUD_CDC_DESC_LEN)

#define EPNUM_HID 0x81
#define EPNUM_CDC_NOTIF 0x82
#define EPNUM_CDC_OUT 0x02
#define EPNUM_CDC_IN 0x83


//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+

uint8_t const desc_configuration[] = {
	// Config number, interface count, string index, total length, attribute, power in mA
	TUD_CONFIG_DESCRIPTOR(1,
	                      ITF_NUM_TOTAL,
	                      0,
	                      CONFIG_TOTAL_LEN,
	                      TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP,
	                      500),

	// Interface number, string index, protocol, report descriptor len, EP In address, size & polling interval
	TUD_HID_DESCRIPTOR(ITF_NUM_HID,
	                   0,
	                   HID_ITF_PROTOCOL_NONE,
	                   sizeof(desc_hid_report),
	                   EPNUM_HID,
	                   CFG_TUD_HID_EP_BUFSIZE,
	                   5),
	TUD_CDC_DESCRIPTOR(
	        ITF_NUM_CDC, 4, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, 64),
};

// called when host requests to get configuration descriptor
// uint8_t const *tud_descriptor_configuration_cb(uint8_t index);

// more device descriptor this time the qualifier
tusb_desc_device_qualifier_t const desc_device_qualifier = {
	.bLength = sizeof(tusb_desc_device_t),
	.bDescriptorType = TUSB_DESC_DEVICE_QUALIFIER,
	.bcdUSB = CDC_EXAMPLE_BCD,

	.bDeviceClass = 0x00,     // TUSB_CLASS_CDC,
	.bDeviceSubClass = 0x00,  // MISC_SUBCLASS_COMMON,
	.bDeviceProtocol = 0x00,  // MISC_PROTOCOL_IAD,

	.bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
	.bNumConfigurations = 0x01,
	.bReserved = 0x00
};

// called when host requests to get device qualifier descriptor
// uint8_t const *tud_descriptor_device_qualifier_cb(void);
uint8_t const *
tud_descriptor_device_qualifier_cb(void)
{
	return (uint8_t const *) &desc_device_qualifier;
}

// String descriptors referenced with .i... in the descriptor tables
enum {
	STRID_LANGID = 0,    // 0: supported language ID
	STRID_MANUFACTURER,  // 1: Manufacturer
	STRID_PRODUCT,       // 2: Product
	STRID_SERIAL,        // 3: Serials
	STRID_CDC_0,         // 4: CDC Interface 0
	STRID_CDC_1,         // 5: CDC Interface 1
};

// array of pointer to string descriptors
char const *string_desc_arr[] = {
	// switched because board is little endian
	(const char[]) { 0x09, 0x04 },  // 0: supported language is English (0x0409)
	"Raspberry Pi",                 // 1: Manufacturer
	"GPMouse CDC",                  // 2: Product
	NULL,             // 3: Serials (null so it uses unique ID if available)
	"Pico SDK stdio"  // 4: CDC Interface 0,
	"RPiReset"        // 5: Reset Interface
};

// buffer to hold the string descriptor during the request | plus 1 for the null terminator
static uint16_t _desc_str[32 + 1];

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *
tud_descriptor_configuration_cb(uint8_t index)
{
	// avoid unused parameter warning and keep function signature consistent
	(void) index;
	// return switch_configuration_descriptor;
	return desc_configuration;
}

uint16_t const *
tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
	// TODO: check lang id
	(void) langid;
	size_t char_count;

	// Determine which string descriptor to return
	switch (index) {
	case STRID_LANGID:
		memcpy(&_desc_str[1], string_desc_arr[STRID_LANGID], 2);
		char_count = 1;
		break;

	case STRID_SERIAL:
		// try to read the serial from the board
		char_count = board_usb_get_serial(_desc_str + 1, 32);
		break;

	default:
		// COPYRIGHT NOTE: Based on TinyUSB example
		// Windows wants utf16le

		// Determine which string descriptor to return
		if (!(index <
		      sizeof(string_desc_arr) / sizeof(string_desc_arr[0]))) {
			return NULL;
		}

		// Copy string descriptor into _desc_str
		const char *str = string_desc_arr[index];

		char_count = strlen(str);
		size_t const max_count = sizeof(_desc_str) / sizeof(_desc_str[0]) -
		                         1;  // -1 for string type
		// Cap at max char
		if (char_count > max_count) {
			char_count = max_count;
		}

		// Convert ASCII string into UTF-16
		for (size_t i = 0; i < char_count; i++) {
			_desc_str[1 + i] = str[i];
		}
		break;
	}

	// First byte is the length (including header), second byte is string type
	_desc_str[0] =
	        (uint16_t) ((TUSB_DESC_STRING << 8) | (char_count * 2 + 2));

	return _desc_str;
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t
tud_hid_get_report_cb(uint8_t instance,
                      uint8_t report_id,
                      hid_report_type_t report_type,
                      uint8_t *buffer,
                      uint16_t reqlen)
{
	return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void
tud_hid_set_report_cb(uint8_t itf,
                      uint8_t report_id,
                      hid_report_type_t report_type,
                      uint8_t const *buffer,
                      uint16_t bufsize)
{
	return;
}