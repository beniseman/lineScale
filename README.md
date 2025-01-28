## 1. Overview

**LineScale Library v0.1.0 (Very Beta)**

The **LineScale** library is an early-stage, experimental library designed to interface with the LineScale dynamometer from LineGrip corp. It provides a structured way to send commands and parse responses.

This library is in **version 0.1.0**, meaning it is highly experimental and subject to significant changes. Some functionality may not be fully tested or implemented.

### Features:
- **BLE Communication**: Connects to the LS3 device via Bluetooth Low Energy (BLE).
- **Command Execution**: Implements various LS3 commands for unit switching, zeroing, data streaming, and more.
- **Data Parsing**: Processes force readings, battery status, and measurement units.

### Limitations:
- **Incomplete Command Set**: Not all LS3 functions are implemented yet.
- **BLE Connectivity**: Stability improvements may be needed.
- **Testing Required**: Further validation is necessary to ensure full compatibility with LS3.

### Intended Use:
This library is meant for developers experimenting with LS3 integration and those looking to extend its capabilities. Expect bugs, missing features, and breaking changes in future updates.

## 2. Documented Commands

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
