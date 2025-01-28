# LineScale Library Documentation

## Overview
**LineScale Library v0.1.0 (Very Beta)**

The **LineScale** library is an early-stage, experimental library for interfacing with the [LineScale dynamometer](https://www.linegrip.com/linescale-3/) from **LineGrip Corp.** It is built for **ESP32** using the Arduino framework, leveraging **Bluetooth Low Energy (BLE)** for communication. This is my first attempt at coding a library and I am using AI for code completion. Expect bugs, sporadic updates at best, and major changes that break code.

### Features:
- **BLE Communication**: Identifies and connects to LineScales using their Bluetooth serviceUUID or last 3 bytes of the MAC address which is engraved above the screen.
- **Command Execution**: Implements some of the documented commands, allows sending a sequence of commands, and testing others.
- **Data Parsing**: Processes 20 byte data into variables.
- **Min and Max tracking**: track min and max values across received packets.

### Limitations:
- **Incomplete Command Set**: Not all LS3 functions are implemented yet.
- **BLE**: No support for BLE pass. It must be **'0000'**. 
- **Testing Required**: Further validation is necessary to ensure full compatibility with LS3.

### Intended Use:
This library is meant for developers experimenting with LS3 integration and those looking to extend its capabilities. Expect bugs, missing features, and breaking changes in future updates.



## Table of Contents

- [Understanding How the LineScale Works](#understanding-how-the-linescale-works)
- [Documented Commands](#documented-commands)
- [Functions in C++](#functions-in-c)
- [Example - Cereal Box](#example---cereal-box)
- [Example - LS3-BLE-ESPNOW](#example---ls3-ble-espnow)
- [Example - ESPNOW-Receiver](#example---espnow-receiver)
- [Future Plans](#future-plans)
- [Acknowledgements](#acknowledgements)



## Understanding How the LineScale Works

#### **Data Stream**
After connecting to the LineScale via Bluetooth, the **"Request PC or Bluetooth online command"** must be sent to begin receiving data. Without this command, the device will remain idle and will not transmit measurements.

#### **Command Behavior Based on Screen State**
The LineScale's responsiveness to commands depends on whether the screen is **locked**, **unlocked**, or **actively navigating menus**:

- **When the screen is locked**:  
  - Only **Request PC or Bluetooth online** and **Disconnect PC or Bluetooth online** commands will function.

- **When navigating menus**:  
  - Only the **Power off** and **Zero** commands will function. These mimic button presses. As there are no commands that mimic up or down button presses menu automation is not currently possible. This is probably for the best, as it would make it easier to accidentally edit the service menu.

#### **Lock Mode Behavior**:  
  - The screen **will not enter lock mode** while navigating menus. This prevents a **"Request PC or Bluetooth online command"** from working. I have added the function **"homeScreen()"** to deal with this situation.
  - If left idle outside of menus, the device will eventually lock based on user settings.

#### **Min and Max**:
  - The LineScale does not transmit min or max measurements. The library tracks these values in kN, which will lead do some discrepancy from what is displayed on the screen if the unit is set to kgf or lbf. Resetting the min and max on the lineScale does not transmit anything over bluetooth, so the library has no way of knowing this has happened. Resetting min and max in the library can reset the min and max on the lineScale *if* the screen is unlocked. This requires setting **Absolute Zero** mode.  

## Documented Commands
This section details the LS3 commands as documented by the manufacturer, including their hex values, observed functionality, and whether they are implemented in the library.

[Manufacturer Reference](LS3_command_table_&_port_protocol.pdf)

| Command | HEX | Observed Functionality | Implemented in Library |
|---------|-----|------------------------|-------------------------|
| Power off | `4F 0D 0A 66` | Power button press | `powerOff()` |
| Zero command | `5A 0D 0A 71` | Zero button press | `zero()` |
| Unit switch to kN | `4E 0D 0A 65` | Set unit to kN | `setUnit("kN")` |
| Unit switch to kgf | `47 0D 0A 5E` | Set unit to kgf | `setUnit("kgf")` |
| Unit switch to lbf | `42 0D 0A 59` | Set unit to lbf | `setUnit("lbf")` |
| Speed switch to SLOW (10Hz) | `53 0D 0A 6A` | Set sampling rate to 10Hz | `speedSlow` |
| Speed switch to FAST (40Hz) | `46 0D 0A 5D` | Set sampling rate to 40Hz | `speedFast` |
| Speed switch to 640Hz | `4D 0D 0A 64` |  | ❌ |
| Speed switch to 1280Hz | `51 0D 0A 68` |  | ❌ |
| Relative/Absolute zero mode switch | `4C 0D 0A 63` | Switch between relative and absolute modes | `toggleZeroMode()` |
| Switch to relative zero mode | `58 0D 0A 6F` | Set relative zero mode | `setRelativeZeroMode()` |
| Switch to absolute zero mode | `59 0D 0A 70` | Set absolute zero mode | `setAbsoluteZeroMode()` |
| Set current value as absolute zero | `54 0D 0A 6B` | Set current value as absolute zero or reference zero | `setAbsoluteZero()` |
| Peak clearing operation | `43 0D 0A 5A` | none ? | `clearPeak()` |
| Request PC or Bluetooth online command | `41 0D 0A 58` | Start data stream (Bluetooth icon will be highlighted) | `startDataStream()` |
| Disconnect PC or Bluetooth online command | `45 0D 0A 5C` | Stop data stream | `stopDataStream()` |
| Read first log entry | `52 30 30 0D 0A C9` |  | ❌ |
| Read x-th log entry | `52 3x 3y 0D 0A ??` |  | ❌ |
| Read 100th log entry | `52 39 39 0D 0A DB` |  | ❌ |

## Functions in C++
List of available functions.

## Example - Cereal Box
Example sketch details.

## Example - LS3-BLE-ESPNOW
Example sketch details.

## Example - ESPNOW-Receiver
Example sketch details.

## Future Plans
Upcoming features and improvements.

## Acknowledgements
Much thanks to Andy Reidrich for his contributions to the slackline world and enabling developments like this by releasing the command protocol. 

This library is based on [Central mode (client) BLE UART for ESP32](https://github.com/ThingEngineer/ESP32_BLE_client_uart/tree/master) by [Josh Campbell](https://github.com/ThingEngineer). I was relieved to find it, as none of the other BLE client sketches included in various libraries were able to interact with charcteristics without crashing.

I also referenced [PyLS3](https://gitlab.com/bjri/pyls3) by [Björn Riske](https://gitlab.com/bjoernr) and [ESP-NOW Two-Way Communication Between ESP32 Boards](https://randomnerdtutorials.com/esp-now-two-way-communication-esp32/) by [Rui Santos](https://randomnerdtutorials.com)


