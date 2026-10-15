# ESP32-S3 + ILI9341 2.4" TFT + Joystick Drawing

A simple drawing program for a **Waveshare ESP32-S3-DEV-KIT-NXRX** with a **2.4" 240x320 SPI TFT (ILI9341)** and an analog **joystick module**. Move the joystick to move a crosshair around the screen and draw lines. When the cross is green it will draw, and red is movement only.

Built with **PlatformIO** in **VS Code**, using the **TFT_eSPI** library.

## Hardware

- Waveshare ESP32-S3-DEV-KIT-NXRX
- 2.4" 240x320 SPI TFT module, 8-pin header (GND, VCC, CLK, MOSI, RES, DC, BLK, MISO). The back of the module is marked ZJY240S0800TG01-ILI9341.
- Analog joystick module with 5 pins (GND, +5V, VRx, VRy, SW)

> This screen module has **no chip select (CS) pin and no touch pins**, so touch and stylus input are not available with it.

## Wiring

### Screen

| Screen pin | ESP32-S3 pin | Notes |
|---|---|---|
| GND | GND | |
| VCC | 3V3 | |
| CLK | IO12 | SPI clock |
| MOSI | IO11 | SPI data out |
| RES | IO14 | Reset |
| DC | IO13 | Data/command |
| BLK | 3V3 | Backlight always on. Use a free GPIO for PWM dimming if wanted |
| MISO | not connected | Not needed for display-only use |

### Joystick

| Joystick pin | ESP32-S3 pin | Notes |
|---|---|---|
| GND | GND | |
| +5V | 3V3 | Power it from **3V3, not 5V**. The X/Y outputs follow the supply voltage and 5V is too high for the ESP32 analog pins |
| VRx | IO1 | X axis (ADC1) |
| VRy | IO2 | Y axis (ADC1) |
| SW | IO4 | Button. Uses the internal pull-up (`INPUT_PULLUP`) |

The board has two 3V3 pins, so the screen and the joystick can each use their own.

## PlatformIO configuration

`platformio.ini`:

```ini
[env:esp32-s3-devkitc-1]
platform = espressif32@6.9.0
board = esp32-s3-devkitc-1
framework = arduino
monitor_speed = 115200
lib_deps = bodmer/TFT_eSPI
build_flags =
  -DARDUINO_USB_CDC_ON_BOOT=1
  -DUSER_SETUP_LOADED=1
  -DILI9341_DRIVER
  -DUSE_HSPI_PORT
  -DTFT_MOSI=11
  -DTFT_SCLK=12
  -DTFT_MISO=-1
  -DTFT_CS=-1
  -DTFT_DC=13
  -DTFT_RST=14
  -DSPI_FREQUENCY=4000000
```

Notes on the settings:

- `USER_SETUP_LOADED=1` makes TFT_eSPI use these build flags instead of its own `User_Setup.h`, so there is no need to edit the library.
- `TFT_CS=-1` because the module has no chip select pin.
- `TFT_MISO=-1` because the screen is write-only here.
- `SPI_FREQUENCY=4000000` is a slow, reliable speed for jumper wires. It can be raised (try 10000000) once everything works.
- `platform = espressif32@6.9.0` is pinned because TFT_eSPI can show a blank white screen on the ESP32-S3 with newer versions of the ESP32 Arduino core.
- A build warning about `TOUCH_CS pin not defined` is harmless, since this module has no touch.

## Controls

| Action | Result |
|---|---|
| Move the joystick | Moves the crosshair. The further you push, the faster it moves |
| Short press of the button | Toggles pen down / pen up. The crosshair is **green** with the pen down (drawing) and **red** with the pen up (moving only) |
| Hold the button for about 0.8 seconds | Clears the screen |

The program records the joystick's resting position at start-up, so keep your hands off the stick for the first second after powering on.

## Tuning

These constants are at the top of `src/main.cpp`:

- `DEADZONE` (default 300): how far the stick must move from centre before the cursor responds. Raise it if the cursor drifts.
- `MAX_SPEED` (default 6): pixels per update at full deflection.
- `LONG_PRESS_MS` (default 800): how long to hold the button to clear the screen.

If a direction feels reversed, flip the sign on `dx` or `dy` in `loop()`. If the axes are swapped, swap `JOY_X` and `JOY_Y`.

## Troubleshooting

- **Screen is white while uploading.** This is normal. The board is in bootloader mode and the screen has not been initialised yet. It should clear once the program starts.
- **Screen stays white, or shows stripes or garbage.** Lower `SPI_FREQUENCY`, make sure the PlatformIO environment you uploaded is the one you intended, and check the CLK, MOSI, DC and RES wires and solder joints. A blank white screen can also come from the ESP32 platform version, so keep it pinned.
- **Wrong or inverted colours.** Add `-DTFT_INVERSION_ON` or `-DTFT_RGB_ORDER=TFT_BGR` to the build flags.
- **Cursor drifts when the stick is untouched.** Restart the board with your hands off the stick so the centre is measured correctly, or raise `DEADZONE`.
- **Cursor wanders with the joystick unplugged.** Expected. The analog inputs are floating and pick up noise.
- **Colours still cycling or an old program running.** The board holds one program at a time. Make sure you uploaded from the right PlatformIO project.

## Testing the screen on its own

If the screen misbehaves, test it separately with a small sketch that fills the screen red, green, blue, white and black in turn, using the same pins and build flags. If that works, the wiring and settings are good and any remaining problem is in the application code.
<img width="204" height="338" alt="PXL_20261015_114603738~2" src="https://github.com/user-attachments/assets/774342b8-9066-4eb7-9505-7fa564a2ea40" />
<img width="235" height="141" alt="PXL_20261015_114608335~2" src="https://github.com/user-attachments/assets/241aa036-0e0f-4637-8023-482f070690b7" />
<img width="207" height="125" alt="PXL_20261015_114615728~2" src="https://github.com/user-attachments/assets/5961176c-e5d2-4922-9930-8ba6a7ef3851" />
<img width="214" height="209" alt="PXL_20261015_114631767 MACRO_FOCUS~2" src="https://github.com/user-attachments/assets/353b9af4-e547-492b-aa47-7c27636ed357" />



