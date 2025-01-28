# LineScale Library v0.1.0 (Very Beta)

## Overview

The **LineScale** library is an early-stage, experimental library for interfacing with the [LineScale dynamometer](https://www.linegrip.com/linescale-3/) from **LineGrip Corp.** It is built for **ESP32** using the **Arduino IDE**. This is my first attempt at coding a library and I am using AI for code completion. Expect bugs, *strange* choices, incomplete documentation, sporadic updates at best, and major changes that break code.

### Features:
- **BLE Communication**: Identifies and connects to LineScales using their Bluetooth serviceUUID or if specified the last 3 bytes of the MAC address (which is engraved above the screen).
- **Command Execution**: Implements some of the documented commands, allows sending a sequence of commands, and testing others.
- **Data Parsing**: Processes 20 byte data into variables.
- **Min and Max tracking**: track min and max values across received packets.

### Limitations:
- **Incomplete Command Set**: Not all LS3 functions are implemented yet.
- **BLE**: No support for BLE pass. It must be **'0000'**. No support for multiple lineScales. It finds and connects.
  

## Table of Contents

- [Understanding How the LineScale Works](#understanding-how-the-linescale-works)
- [Documented Commands](#documented-commands)
- [Functions](#functions)
- [Example Sketches](#example-sketches)
- [Future Plans](#future-plans)
- [Acknowledgements](#acknowledgements)



## Understanding How the LineScale Works

#### **Data Stream**
After connecting to the LineScale via Bluetooth, the **"Request PC or Bluetooth online command"** must be sent to begin receiving data. Without this command, the device will remain idle and will not transmit measurements.

#### **Command Behavior Based on Screen State**
The LineScale's responsiveness to commands depends on whether the screen is **locked**, **actively navigating menus**, or **on the measurement screen**:

- **When the screen is locked**:  
  - Only **Request PC or Bluetooth online** and **Disconnect PC or Bluetooth online** commands will function.

- **When navigating menus**:  
  - Only the **Power off** and **Zero** commands will function. These mimic button presses. As there are no commands that mimic up or down button presses menu automation is not currently possible. This is probably for the best, as it would make it easier to accidentally edit the service menu and risk bricking your device.

- **Measurement screen**:
  - The lineScale will accept commands
  - Functionality may be context dependent, as with the Zero command. In Absolute Zero mode this resets the min and max. In Relative Zero mode this sets a reference zero.  


#### **Lock Mode Behavior**:  
  - The screen **will not enter lock mode** while navigating menus. This prevents a **"Request PC or Bluetooth online command"** from working. I have added the function **"homeScreen()"** to deal with this situation.
  - If left idle outside of menus, the device will lock based on user settings.

#### **Min and Max**:
  - The LineScale does not transmit min or max measurements. The library tracks only the values it receives. This is done in kN, which will lead do some discrepancy from what is displayed on the screen if the unit is set to kgf or lbf. Resetting the min and max on the lineScale does not transmit anything over bluetooth, so the library has no way of knowing this has happened. Resetting min and max in the library can reset the min and max on the lineScale *if* the screen is unlocked. 

## Documented Commands
[LS3_command_table_&_port_protocol.pdf](LS3_command_table_&_port_protocol.pdf).


| Command | HEX | Observed Functionality | Implemented in Library |
|---------|-----|------------------------|-------------------------|
| Power off | `4F 0D 0A 66` | Power button press | `powerButton()` |
| Zero command | `5A 0D 0A 71` | Zero button press | `zeroButton()` |
| Unit switch to kN | `4E 0D 0A 65` | Set unit to kN | `switchToKN()` |
| Unit switch to kgf | `47 0D 0A 5E` | Set unit to kgf | `switchToKGF()` |
| Unit switch to lbf | `42 0D 0A 59` | Set unit to lbf | `switchToLBF()` |
| Speed switch to SLOW (10Hz) | `53 0D 0A 6A` | Set sampling rate to 10Hz | `speedSlow()` |
| Speed switch to FAST (40Hz) | `46 0D 0A 5D` | Set sampling rate to 40Hz | `speedFast()` |
| Speed switch to 640Hz | `4D 0D 0A 64` |  | ❌ |
| Speed switch to 1280Hz | `51 0D 0A 68` |  | ❌ |
| Relative/Absolute zero mode switch | `4C 0D 0A 63` | Switch between relative and absolute modes | `toggleZeroMode()` |
| Switch to relative zero mode | `58 0D 0A 6F` | Set relative zero mode | `setRelativeZeroMode()` |
| Switch to absolute zero mode | `59 0D 0A 70` | Set absolute zero mode | `setAbsoluteZeroMode()` |
| Set current value as absolute zero | `54 0D 0A 6B` | Set current value as absolute zero or reference zero | `setAbsoluteZero()` |
| Peak clearing operation | `43 0D 0A 5A` | ❌ none? | `clearPeak()` |
| Request PC or Bluetooth online command | `41 0D 0A 58` | Start data stream (Bluetooth icon will be highlighted) | `startDataStream()` |
| Disconnect PC or Bluetooth online command | `45 0D 0A 5C` | Stop data stream | `stopDataStream()` |
| Read first log entry | `52 30 30 0D 0A C9` |  | ❌ |
| Read x-th log entry | `52 3x 3y 0D 0A ??` |  | ❌ |
| Read 100th log entry | `52 39 39 0D 0A DB` |  | ❌ |

## Functions
Functions in previous section are not repeated here.

| Command | Description |
|---------|-------------|
| `void sendCommand(const String& command);` | Converts string to 4 byte HEX command(s) to be sent to the lineScale (e.g., `A` + `\r+\n+crc` converts to `41 0D 0A 58`). Can handle single commands `Z` or multiple commands `AZ`. |
| `void parseData(const uint8_t* pData, size_t length);` | Parses incoming BLE data packets from the LineScale. |
| `void homeScreen();` | Simulates pressing the power button repeatedly to exit menus and return to the home screen. Also sends `startDataStream()` |
| `void resetMinMax();` | Resets the stored minimum and maximum force values. Sends `setAbsoluteZeroMode()` and `zeroButton()` |
| `float convertToKN(float value, const char* unit);` | Converts a given force value to kN from the specified unit. |
| `char getWorkingMode() const;` | Retrieves the current working mode (real-time, overload, or max capacity). |
| `float getMeasuredValue() const;` | Retrieves the current measured force value. |
| `float getRelativeForce() const;` | Retrieves the current relative force value. |
| `char getZeroMode() const;` | Retrieves the current zero mode (relative or absolute). |
| `float getReferenceZero() const;` | Retrieves the current reference zero value. |
| `int getBatteryLevel() const;` | Retrieves the current battery level percentage. |
| `const char* getUnit() const;` | Retrieves the current unit of measurement (kN, kgf, or lbf). |
| `int getSpeed() const;` | Retrieves the current sample rate in Hz. |
| `float getMaxMeasuredValue() const;` | Retrieves the maximum recorded force value. |
| `float getMinMeasuredValue() const;` | Retrieves the minimum recorded force value. |
| `void checkNotifyTimeout(unsigned int timeoutSeconds);` | Checks if there has been a timeout in receiving BLE notifications within the given timeout duration. |

## Example Sketches

- [LS3-BLE-Serial](examples/LS3-BLE-Serial/README.md) - Serial output testing playground.
- [LS3-BLE-ESPNOW](examples/LS3-BLE-ESPNOW/README.md) - Outputs data to an OLED and broadcasts over ESPNOW.  
- [ESPNOW-Receiver](examples/ESPNOW_Sender/README.md) - Receives data over ESPNOW and outputs to an OLED. 

## Future Plans

### Structural improvements / known bugs
- get rid of extra stuff in the loop, to a coherent handleConnection or similar (I have tried and its causing bluetooth problems)

### BLE
- multi device support with selection when connecting, naming devices

### USB UART
- add support for wired connection
 
### ESPNOW
- ability to choose between or simultaneously display info from multiple data streams

### LORA
- add support for LORA modules to further extend range

## Acknowledgements
Much thanks to Andy Reidrich for his contributions to the slackline world and for enabling developments like this by releasing the lineScale command protocol. 

This library is based on [Central mode (client) BLE UART for ESP32](https://github.com/ThingEngineer/ESP32_BLE_client_uart/tree/master) by [Josh Campbell](https://github.com/ThingEngineer). I was relieved to find it, as none of the other BLE client sketches included in various libraries were able to interact with charcteristics without crashing.

I also referenced [PyLS3](https://gitlab.com/bjri/pyls3) by [Björn Riske](https://gitlab.com/bjoernr) and [ESP-NOW Two-Way Communication Between ESP32 Boards](https://randomnerdtutorials.com/esp-now-two-way-communication-esp32/) by [Rui Santos](https://randomnerdtutorials.com).


