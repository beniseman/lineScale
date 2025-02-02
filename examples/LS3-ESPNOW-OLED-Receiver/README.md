# LS3-ESPNOW-OLED

This is the ESPNOW receiver sketch for the device you want to use for viewing remote readings.

Button functionality (receiver):
  - click: wake the device
  - double click: reset max and min
  - hold (3 seconds) sleep the device

### Test Setup
![](../LS3-BLE-OLED-ESPNOW/esp32-breadboard.jpg)

### Wiring diagram
- Xiao ESP32S3
- OLED 128x64 I2C
- 10kΩ resistor
- button
  
![Fritzing Part](../LS3-BLE-OLED-ESPNOW/xiao-oled-button.png)
