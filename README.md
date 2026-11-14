# AirMouse — ESP32-C3 SuperMini Bluetooth Motion Mouse

Turn an ESP32-C3 SuperMini, an MPU6050 motion sensor, and two pushbuttons into a Bluetooth Low Energy (BLE) air mouse. Tilt or rotate the device to move the pointer; the two buttons act as left- and right-click.

This README documents the supplied sketch. It uses I²C on GPIO 8/9, buttons on GPIO 5/6, the MPU6050's default `0x68` address, and the legacy `ESP32 BLE Mouse` library API.

## What it does

- Advertises as a BLE mouse named **AirMouse**.
- Reads the MPU6050 gyroscope and maps its X and Z axes to relative pointer movement.
- Calibrates stationary gyro bias at startup.
- Applies a dead zone, smoothing, and fractional-motion accumulation.
- Sends left/right button states as HID mouse-button presses.
- Prints gyro and mouse-delta diagnostics at 115200 baud while connected.

## Hardware

| Item | Notes |
| --- | --- |
| ESP32-C3 SuperMini | BLE-capable ESP32-C3 board |
| MPU6050 breakout | I²C accelerometer/gyroscope module; this sketch uses its gyro only |
| 2 × momentary pushbuttons | Left and right mouse buttons |
| Jumper wires and USB data cable | For wiring, power, and programming |

## Wiring

Use the GPIO labels printed on the board, not physical pin numbers.

| ESP32-C3 SuperMini | Connect to | Purpose |
| --- | --- | --- |
| 3V3 | MPU6050 VCC | Sensor power |
| GND | MPU6050 GND | Common ground |
| GPIO 8 | MPU6050 SDA | I²C data |
| GPIO 9 | MPU6050 SCL | I²C clock |
| GPIO 5 | One terminal of left button | Left-click input |
| GPIO 6 | One terminal of right button | Right-click input |
| GND | Other terminal of both buttons | `INPUT_PULLUP` return path |

Keep the MPU6050 `AD0` pin low (normally connected to GND) so it uses I²C address `0x68`. The MPU6050 `INT` pin is not used.

> Keep ESP32-C3 GPIO and I²C signals at 3.3 V. Do not connect 5 V directly to an ESP32-C3 GPIO.

## Software and libraries

| Component | Version/status | Installation |
| --- | --- | --- |
| Arduino IDE | No specific IDE version is required | [Download Arduino IDE](https://www.arduino.cc/en/software) |
| **esp32 by Espressif Systems** | See compatibility note below | Arduino IDE → **Tools → Board → Boards Manager** → search `esp32` |
| [ESP32 BLE Mouse](https://github.com/T-vK/ESP32-BLE-Mouse) by T-vK | **0.3.1** | Download its release ZIP, then use **Sketch → Include Library → Add .ZIP Library…** |
| [I2Cdev](https://github.com/jrowberg/i2cdevlib/tree/master/Arduino/I2Cdev) by Jeff Rowberg | Install from I2Cdevlib | Provides `I2Cdev.h` |
| [MPU6050](https://github.com/jrowberg/i2cdevlib/tree/master/Arduino/MPU6050) by Jeff Rowberg | Install from I2Cdevlib | Provides `MPU6050.h` |
| `Wire` | Included with ESP32 board support | No separate installation needed |

### Installing I2Cdevlib

1. Download or clone [jrowberg/i2cdevlib](https://github.com/jrowberg/i2cdevlib).
2. Copy both `Arduino/I2Cdev` and `Arduino/MPU6050` into your Arduino sketchbook’s `libraries` folder.
3. Restart Arduino IDE.

## ESP32-C3 SuperMini compatibility

The ESP32-C3 supports BLE, but this project uses T-vK’s older `ESP32 BLE Mouse` library, which can be version-sensitive on C3 boards.

| Type | Guidance |
| --- | --- |
| Verified requirement | Use an ESP32-C3 board profile and pair it as a Bluetooth **LE** mouse. The C3 does not support Bluetooth Classic. |
| Recommended C3 workaround | Set **esp32 by Espressif Systems** to **v2.0.17** in Boards Manager. Community reports show this can resolve hangs or crashes around `bleMouse.begin()` on C3 SuperMini boards. |
| Not verified | A required Arduino IDE downgrade. The reported fix is changing the **ESP32 board package** version, not downgrading Arduino IDE itself. |
| Avoid assuming | That every ESP32 board-package 3.x release works reliably with this older BLE Mouse library on ESP32-C3. |

To select the recommended version:

1. Open **Tools → Board → Boards Manager**.
2. Search for **esp32**.
3. Select **esp32 by Espressif Systems**.
4. Choose version **2.0.17**.
5. Select **Tools → Board → ESP32 Arduino → ESP32C3 Dev Module**.

Enable `USB CDC On Boot` if you want Serial Monitor output through the board’s USB connector.

## Setup and upload

1. Complete the wiring.
2. Install the required libraries.
3. Select **ESP32C3 Dev Module**.
4. Install/select **esp32 by Espressif Systems 2.0.17**.
5. Save the supplied sketch as `AirMouse.ino`, open it in Arduino IDE, and upload it.
6. Open Serial Monitor at **115200 baud**.
7. Keep the AirMouse completely still during startup calibration.
8. Pair **AirMouse** from your computer or phone’s Bluetooth settings.

## Calibration and use

At startup, the sketch collects 500 gyro samples over roughly 1.5 seconds. Keep the device motionless during this time; moving it while calibrating can cause pointer drift.

The physical direction of movement depends on how the MPU6050 is mounted. This sketch maps gyro X to horizontal pointer movement and gyro Z to vertical movement. A different mounting orientation may make the axes feel swapped or reversed.

The buttons use `INPUT_PULLUP`:

- Unpressed = HIGH
- Pressed = connected to GND / LOW

## Bluetooth pairing

1. Power the device and wait for calibration to finish.
2. Open Bluetooth settings on the host device.
3. Find **AirMouse**.
4. Pair it as a mouse/HID device.

If pairing fails, remove or forget the existing AirMouse entry from the host, restart the board, then pair again.

## Troubleshooting

| Symptom | What to check |
| --- | --- |
| `MPU6050 NOT DETECTED!` | Check 3V3/GND, GPIO 8 → SDA, GPIO 9 → SCL, shared ground, and that `AD0` is low. |
| `I2Cdev.h` or `MPU6050.h` not found | Install both matching folders from Jeff Rowberg’s I2Cdevlib. |
| `BleMouse.h` not found | Install T-vK’s ESP32 BLE Mouse library. |
| Crash or hang at `bleMouse.begin()` | Change the ESP32 board package to **2.0.17**. |
| AirMouse does not appear in Bluetooth scan | Confirm Serial Monitor reaches `Bluetooth ready. Pair AirMouse.` Remove old pairings and try again. |
| Connected but no cursor movement | The sketch only sends input after Bluetooth connects and the MPU6050 passes its test. Check the serial gyro values and recalibrate. |
| Cursor drifts or jitters | Restart and keep the device still during calibration. Check sensor wiring. |
| Buttons do not work | Each button must connect GPIO 5 or GPIO 6 to GND when pressed. |
| No serial output | Use 115200 baud and enable `USB CDC On Boot` if available. |

## Known limitations

- BLE mouse only; not USB HID or Bluetooth Classic.
- Relative pointer control only; no absolute pointing or orientation tracking.
- Motion direction depends on MPU6050 mounting orientation.
- No software debounce for the buttons.
- No scroll wheel, middle button, battery measurement, sleep mode, or charging circuit.
- The reported battery level is fixed at 100%.
- ESP32-C3 reliability may vary with newer 3.x Espressif board packages.
- macOS and iOS can be less reliable with the upstream BLE Mouse library.

## Sources

- [ESP32 BLE Mouse](https://github.com/T-vK/ESP32-BLE-Mouse)
- [ESP32-C3 SuperMini compatibility issue](https://github.com/T-vK/ESP32-BLE-Mouse/issues/69)
- [Espressif Arduino-ESP32 documentation](https://docs.espressif.com/projects/arduino-esp32/en/latest/getting_started.html)
- [Jeff Rowberg’s I2Cdevlib](https://github.com/jrowberg/i2cdevlib)
