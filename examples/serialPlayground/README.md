# Example - Cereal Box
This sketch is a serial monitor testing setup and a good place to start.

- Reads incoming text and converts to 4 byte HEX command to be sent to the lineScale
  - (e.g., `A` + `\r+\n+crc` converts to `41 0D 0A 58`)
- Allows multiple commands in a single input
  - (e.g., `"YZ"` sets absolute zero mode and resets max/min values).

## Documented Commands

| Command | Function |
|---------|----------|
| `A` | Start Data Stream |
| `E` | Stop Data Stream |
| `Q` | Reset Tracked Max/Min |
| `H` | Navigate back to home screen |
| `O` | Power Button |
| `Z` | Zero Button |
| `N` | Switch to kN |
| `G` | Switch to kgf |
| `B` | Switch to lbf |
| `S` | Speed Slow (10Hz) |
| `F` | Speed Fast (40Hz) |
| `L` | Toggle Zero Mode |
| `X` | Set Relative Zero Mode |
| `Y` | Set Absolute Zero Mode |
| `T` | Set Current Value as Zero |
| `C` | Peak Clearing Operation |

## Sketch Commands
| Command | Function |
|---------|----------|
| `space` | Start / Stop output to serial monitor |
| `Q` | Reset Tracked Max/Min via [resetMinMax()](#functions) |
| `H` | Navigate back to home screen via [homeScreen()](#functions) |
