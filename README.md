[![Arduino Library](https://img.shields.io/badge/Arduino-Library-00979D?logo=arduino&logoColor=white&style=flat)](https://www.ardu-badge.com/lineScale)
[![Arduino Library Manager](https://www.ardu-badge.com/badge/lineScale.svg?)](https://www.ardu-badge.com/lineScale)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/beniseman/library/lineScale.svg)](https://registry.platformio.org/libraries/beniseman/lineScale)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![GitHub release](https://img.shields.io/github/v/release/beniseman/lineScale)](https://github.com/beniseman/lineScale/releases)
[![GitHub issues](https://img.shields.io/github/issues/beniseman/lineScale)](https://github.com/beniseman/lineScale/issues)

# LineScale Library v0.1.2 (alpha)

## Overview

The **LineScale** library is an early-stage, experimental library for interfacing with the [LineScale dynamometer](https://www.linegrip.com/linescale-3/) from **LineGrip Corp.** It is built for **ESP32** using the **Arduino IDE**. This is my first attempt at coding a library and I am using AI for code completion. Expect bugs, *strange* choices, incomplete documentation, sporadic updates at best, and major changes that break code. 

### Features:
- **BLE Communication**: Identifies and connects to LineScales using their Bluetooth serviceUUID or a specified MAC address.
- **Command Execution**: Implements some of the documented commands, allows sending a sequence of commands, and testing others.
- **Data Parsing**: Processes 20 byte data into variables.
- **Callbacks**: callbacks for when new data has been processed, connection or disconnection events
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
After connecting to the LineScale via Bluetooth, the **"Request PC or Bluetooth online command"** must be sent to begin receiving data. Without this command, the device will remain idle and will not transmit measurements. I have opted to send this by default (for now).

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
  - The screen **will not enter lock mode** while navigating menus. This prevents a **"Request PC or Bluetooth online command"** from working.
  - If left idle outside of menus, the device will lock based on user settings.

#### **Min and Max**:
  - The LineScale does not transmit min or max measurements, only a measurement and reference zero. The library tracks min and max using the force values it receives. This is done in kN, which will lead do some discrepancy from what is displayed on the screen if the unit is set to kgf or lbf. Resetting the min and max on the lineScale does not transmit anything over bluetooth, so the library has no way of knowing this has happened. Resetting min and max in the library can reset the min and max on the lineScale *if* the screen is unlocked. 

## Documented Commands
[LS3_command_table_&_port_protocol.pdf](LS3_command_table_&_port_protocol.pdf)


| Command | HEX | Observed Functionality | Implemented in Library |
|---------|-----|------------------------|-------------------------|
| Power off | `4F 0D 0A 66` | Power button press | `powerButton()` |
| Zero command | `5A 0D 0A 71` | Zero button press | `zeroButton()` |
| Unit switch to kN | `4E 0D 0A 65` | Set unit to kN | `setUnitKN()` |
| Unit switch to kgf | `47 0D 0A 5E` | Set unit to kgf | `setUnitKGF()` |
| Unit switch to lbf | `42 0D 0A 59` | Set unit to lbf | `setUnitLBF()` |
| Speed switch to SLOW (10Hz) | `53 0D 0A 6A` | Set sampling rate to 10Hz | `setScanRate(10)` |
| Speed switch to FAST (40Hz) | `46 0D 0A 5D` | Set sampling rate to 40Hz | `setScanRate(40)` |
| Speed switch to 640Hz | `4D 0D 0A 64` |  | ❌ |
| Speed switch to 1280Hz | `51 0D 0A 68` |  | ❌ |
| Relative/Absolute zero mode switch | `4C 0D 0A 63` | Switch between relative and absolute modes | ❌ |
| Switch to relative zero mode | `58 0D 0A 6F` | Set relative zero mode | `setRelativeZeroMode()` |
| Switch to absolute zero mode | `59 0D 0A 70` | Set absolute zero mode | `setAbsoluteZeroMode()` |
| Set current value as absolute zero | `54 0D 0A 6B` | Set current value as absolute zero or reference zero | `setAbsoluteZero()` |
| Peak clearing operation | `43 0D 0A 5A` | ❌ none? | ❌ |
| Request PC or Bluetooth online command | `41 0D 0A 58` | Start data stream (Bluetooth icon will be highlighted) **and sets Scan Rate to 40Hz** | `startDataStream()` |
| Disconnect PC or Bluetooth online command | `45 0D 0A 5C` | Stop data stream | `stopDataStream()` |
| Read first log entry | `52 30 30 0D 0A C9` |  | ❌ |
| Read x-th log entry | `52 3x 3y 0D 0A ??` |  | ❌ |
| Read 100th log entry | `52 39 39 0D 0A DB` |  | ❌ |

## Functions

### Connectivity
| Type | Function | Description |
|------|----------|------------|
| `void` | `setMAC(const std::string& mac);` | Set the MAC address for connecting to a specific LineScale device. When this is set connect() will exclusively choose this device. |
| `bool` | `connect(const std::string& mac = "");` | Connect to the LineScale device. If no MAC is provided, it scans for available devices. |
| `void` | `disconnect();` | Disconnect from the LineScale device. |
| `void` | `handleTimeout(int timeoutSeconds=30);` | Manages timeout behavior when connection is lost or inactive due to being in the menus or if `startDataStream` needs to be re-sent. Must be in the loop. If you don't pass a timeout in seconds it will default to 30. Minimum accepted is 10. |
| `std::string` | `getMAC() const;` | Retrieve the MAC address of the lineScale you are connected to. |
| `bool` | `isConnected();` | Retruns connection status. |

### Basic 
| Type | Function | Description |
|------|----------|------------|
| `void` | `sendCommand(const std::string& command);` | Send a command to the LineScale device. |
| `void` | `setDebug(bool enable);` | Enable or disable debugging output to the serial monitor. |


### Callbacks
| Type | Function | Description |
|------|----------|------------|
| `void` | `setDataCallback(LineScaleDataCallback callback);` | Set a callback function to handle incoming data from the LineScale. |
| `void` | `setConnectionCallback(ConnectionCallback callback);` | Set a callback function to handle successful connection events. |
| `void` | `setDisconnectionCallback(DisconnectionCallback callback);` | Set a callback function to handle disconnecction events. |

### lineScale documented commands
| Type | Function | Description |
|------|----------|------------|
| `void` | `setScanRate(int speedValue);` | Set the scan rate (speed) for data acquisition. Over BLE this works with 10 or 40. |
| `void` | `setUnitKN();` | Set the force unit to kilonewtons (kN). |
| `void` | `setUnitKGF();` | Set the force unit to kilogram-force (kgf). |
| `void` | `setUnitLBF();` | Set the force unit to pound-force (lbf). |
| `void` | `powerButton();` | Simulate pressing the power button. |
| `void` | `zeroButton();` | Simulate pressing the zero button. |
| `void` | `setRelativeZeroMode();` | Set zero mode to relative zero. |
| `void` | `setAbsoluteZeroMode();` | Set zero mode to absolute zero. |
| `void` | `setAbsoluteZero();` | Set the absolute zero reference. |
| `void` | `startDataStream();` | Start the data stream from the LineScale. |
| `void` | `stopDataStream();` | Stop the data stream from the LineScale. |

### Data retrieval
| Type | Function | Description |
|------|----------|------------|
| `void` | `resetMaxMin();` | Reset the max and min force values stored by the library. *Attempt* to reset these values on the lineScale, but only works if the screen is unlocked. |
| `int` | `NotificationRate();` | Get the last notification rate (notifications per second). |
| `char` | `WorkingMode();` | Get the current working mode. |
| `char` | `ZeroMode();` | Get the current zero mode. |
| `float` | `Force();` | Get the current force value. |
| `float` | `RelativeForce();` | Get the relative force value. |
| `float` | `ReferenceZero();` | Get the reference zero value. |
| `std::string` | `Unit();` | Get the current force unit. |
| `int` | `ScanRate();` | Get the current scan rate (speed). |
| `float` | `MaxForce();` | Get the maximum recorded force in the current unit. |
| `float` | `MinForce();` | Get the minimum recorded force in the current unit. |
| `int` | `BatteryLevel();` | Get the current battery level of the device. |

## Example Sketches

- [HelloWorld](examples/HelloWorld/)!   
- [LS3-BLE-Serial](examples/CerealBox/) - Serial output testing playground.
- [LS3-BLE-ESPNOW](examples/LS3-BLE-OLED-ESPNOW/) - Outputs data to an OLED and broadcasts over ESPNOW.  
- [ESPNOW-Receiver](examples/LS3-ESPNOW-OLED-Receiver/) - Receives data over ESPNOW and outputs to an OLED. 

## Future Plans

### Structural improvements / known bugs
- I'll fill this in once I get some feedback.

### BLE
- multi device support with selection when connecting, naming devices

### USB UART
- add support for wired connection
 
### ESPNOW
- ability to choose between or simultaneously display info from multiple data streams

### LORA
- add support for LORA modules to further extend range

### Custom PCBs
#### Base Station
  - powered by 18650
  - footprint for optional LORA module

#### Remote viewer
  - small and light so it is comfortable on the harness
  - 150-300mAh battery?

## Acknowledgements
Much thanks to Andy Reidrich for his contributions to the slackline world and for enabling developments like this by releasing the lineScale command protocol. 

This library is based on [Central mode (client) BLE UART for ESP32](https://github.com/ThingEngineer/ESP32_BLE_client_uart/tree/master) by [Josh Campbell](https://github.com/ThingEngineer). I was relieved to find it, as none of the other BLE client sketches included in various libraries were able to interact with charcteristics without crashing. As it turns out, the issue was that I was trying to read the characteristic when all I had to do was use the notify callback. 

I also referenced [PyLS3](https://gitlab.com/bjri/pyls3) by [Björn Riske](https://gitlab.com/bjoernr) and [ESP-NOW Two-Way Communication Between ESP32 Boards](https://randomnerdtutorials.com/esp-now-two-way-communication-esp32/) by [Rui Santos](https://randomnerdtutorials.com).


