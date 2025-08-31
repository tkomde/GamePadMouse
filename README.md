# GamePadMouse

This is a bridge program that receives Bluetooth Gamepad data on Raspberry Pi Pico W / Pico 2 W and acts as a USB mouse. Currently, it has been tested with Pico/Pico2 W and JOYCON(L). For a list of controllers that are likely to work, see [here](https://bluepad32.readthedocs.io/en/latest/supported_gamepads/).

https://github.com/user-attachments/assets/5c00ffa1-d619-4b3a-968f-d7519610d4f4
https://github.com/user-attachments/assets/af58bd26-5fe7-4fce-afbb-caf3b4e20d17


This project is possible thanks to [Bluepad32](https://github.com/ricardoquesada/bluepad32) and [TinyUSB](https://github.com/hathach/tinyusb). The HID bridge was implemented with reference to [PicoSwitch-WirelessGamepadAdapter](https://github.com/juan518munoz/PicoSwitch-WirelessGamepadAdapter).

## Installing

1. Download the latest `.uf2` file from [releases](https://github.com/tkomde/GamePadMouse/releases/tag/v0.1.0).
2. Connect the Pico to your PC while holding down the bootsel button.
3. A folder will appear. Drag and drop the `.uf2` file into it.

## Connecting

1. Connect the Pico W (with UF2 installed) to the device you want to use as a USB mouse.
2. Put your gamepad into pairing mode. When the connection is complete, the LED on the Pico W will light up and mouse signals will be sent.

## Usage

- Cursor move: 
- Left click: ZL trigger
- Right click: L trigger
- Scroll up/down: up/down button

## Building

This repo is compliant to github codespace.
Or if you want to build from source locally, follow these steps:

1. Install Make, CMake (at least version 3.13), and the GCC cross compiler:
```bash
sudo apt-get install make cmake gdb-arm-none-eabi gcc-arm-none-eabi build-essential
```
2. Install the [Pico SDK](https://github.com/raspberrypi/pico-sdk) and set the `PICO_SDK_PATH` environment variable to the SDK path.
3. Update submodules (Bluepad32)
```bash
git submodule update --init
```
4. Build
```bash
mkdir build ; cd build
cmake .. -DPICO_BOARD=pico_w # for pico w
# cmake .. -DPICO_BOARD=pico2_w # for pico2 w
make -j2
```

The file `GamePadMouse.uf2` will be created under the build directory.

## FAQ

- Can I use this on other boards like esp32?
  - No, support for other boards is not planned as this project depends on the pico-sdk.
- Does it work on Pico / Pico2 (without wwireless)?
  - Wireless functionality is required, so please use Pico W or Pico 2 W.
