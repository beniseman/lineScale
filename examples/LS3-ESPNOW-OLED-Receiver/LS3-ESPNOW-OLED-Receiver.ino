#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

#include "esp_sleep.h"  // Include ESP32 sleep functions


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C

// OLED Display instance
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

bool screenIsOn = true;
bool isUpdatingOLED = false;

uint8_t broadcastAddress[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
esp_now_peer_info_t peerInfo;
// Define your struct
typedef struct struct_message {
  float measuredValue;
  float maxValue;
  int hertz;
  char unit[4];
  int batteryLevel;
  uint8_t commandByte;  // New single-byte field
} struct_message;

struct_message myData;
struct_message responseMessage;  // Data to Send

unsigned long lastSecond = 0;  // Tracks the last second
int messageCount = 0;          // Counts ESP-NOW messages
int dataRate = 0;              // Stores final messages per second value

//button click variables
const int BUTTON_PIN = 2;       // Button connected to digital pin 2 (active high)
int currentButtonState = LOW;   // Current reading from the button
int previousButtonState = LOW;  // Previous reading from the button

unsigned long lastDebounceTime = 0;       // Last time the button state changed
const unsigned long DEBOUNCE_DELAY = 10;  // Debounce time in ms

unsigned long buttonPressStartTime = 0;  // Timestamp when the button was pressed
bool holdEventTriggered = false;         // Flag indicating if a hold event has been triggered

bool waitingForDoubleClick = false;              // Flag to indicate a potential double click
unsigned long buttonReleaseTime = 0;             // Timestamp when the button was released
const unsigned long DOUBLE_CLICK_TIMEOUT = 250;  // Time window to detect a double click (ms)

const unsigned long HOLD_TIME = 3000;  // Time in ms required to trigger a hold event

//void OnDataSent(const uint8_t* mac_addr, esp_now_send_status_t status) {
//Serial.print("\r\nLast Packet Send Status:\t");
//Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
//}

void OnDataRecv(const uint8_t* mac, const uint8_t* incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));  // Copy received data into struct

  messageCount++;  // Increment message counter

  if (screenIsOn) {  // Only update OLED if the screen is on
    updateOLED();
  }
}

void setupESPNOW() {
  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);  // Force channel 1

  esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  //esp_now_register_send_cb(OnDataSent);

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}


void updateOLED() {
  if (isUpdatingOLED) return;  // If update is already in progress, exit early
  isUpdatingOLED = true;       // Set the flag to indicate OLED is being updated

  display.clearDisplay();  // Clear the screen

  // Display battery percentage at top-left
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(myData.batteryLevel);  // Battery percentage from ESP-NOW message
  display.print("%");

  // Display scan rate (Messages Per Second / Hz)
  display.setTextSize(1);
  String scanRateText = String(dataRate) + "/" + String(myData.hertz) + "Hz";

  int16_t x1, y1;
  uint16_t textWidth, textHeight;
  display.getTextBounds(scanRateText.c_str(), 0, 0, &x1, &y1, &textWidth, &textHeight);
  int xPosScanRate = SCREEN_WIDTH - textWidth - 1;  // Right-align

  display.setCursor(xPosScanRate, 0);
  display.print(scanRateText);

  // Display force measurement in the center
  char measuredBuffer[10];
  snprintf(measuredBuffer, sizeof(measuredBuffer), "%.2f", myData.measuredValue);

  display.setTextSize(3);
  display.getTextBounds(measuredBuffer, 0, 0, &x1, &y1, &textWidth, &textHeight);
  int xPosForce = (SCREEN_WIDTH - textWidth) / 2;
  int yPosForce = (SCREEN_HEIGHT - textHeight) / 2;
  display.setCursor(xPosForce, yPosForce);
  display.print(measuredBuffer);

  // --- Display "MAX" Label and Max Force on the Bottom Line ---
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  String maxForceText;
  if (strcmp(myData.unit, "kN") == 0) {
    maxForceText = String(myData.maxValue, 2);  // Keep two decimal places for kN
  } else {
    maxForceText = String(int(round(myData.maxValue)));  // Round and cast to int for other units
  }
  maxForceText += String(myData.unit);  // Append unit with space

  // Get width of max force text
  display.getTextBounds(maxForceText.c_str(), 0, 0, &x1, &y1, &textWidth, &textHeight);

  int maxRectWidth = 26;   // Width of "MAX" label box
  int maxRectHeight = 14;  // Height of label box
  int spacing = 8;         // Space between "MAX" and max value

  // Calculate total width needed for "MAX" + space + maxForceText
  int totalWidth = maxRectWidth + spacing + textWidth;
  int xStart = (SCREEN_WIDTH - totalWidth) / 2;  // Center align

  // Draw filled rounded rectangle for "MAX" label
  display.fillRoundRect(xStart, SCREEN_HEIGHT - maxRectHeight, maxRectWidth, maxRectHeight, 3, SSD1306_WHITE);

  // Print "MAX" inside the rectangle in BLACK
  display.setTextSize(1);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(xStart + 5, SCREEN_HEIGHT - maxRectHeight + 3);
  display.print("MAX");

  // Print max force value to the right of "MAX" in size 2
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(xStart + maxRectWidth + spacing, SCREEN_HEIGHT - 14);
  display.print(maxForceText);

  display.display();  // Update the display

  isUpdatingOLED = false;  // Clear the flag after updating
}

void setupOLED() {

  //Wire.begin(SDA, SCL); for custom pins

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("SSD1306 allocation failed");
    return;
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.display();
}


void setup() {
  Serial.begin(115200);

  setupESPNOW();
  setupOLED();


  pinMode(BUTTON_PIN, INPUT);  // Ensure an external pulldown resistor is used for active high wiring
}

void loop() {


  processButton();

  if (millis() - lastSecond >= 1000) {  // Every 1 second
    dataRate = messageCount;            // Store messages per second
    messageCount = 0;                   // Reset counter
    lastSecond = millis();              // Reset timer
  }

  if (screenIsOn) {
    if (dataRate > 0) {
      updateOLED();  // try putting this is in the callback and see what happens. 1-2 second lag even with flags!
    } else {
      display.clearDisplay();


      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);

      String noDataText = "No incoming data :(";
      int16_t x1, y1;
      uint16_t textWidth, textHeight;
      display.getTextBounds(noDataText.c_str(), 0, 0, &x1, &y1, &textWidth, &textHeight);

      int xPos = (SCREEN_WIDTH - textWidth) / 2;    // Center horizontally
      int yPos = (SCREEN_HEIGHT - textHeight) / 2;  // Center vertically

      display.setCursor(xPos, yPos);
      display.print(noDataText);


      display.display();
    }
  }
}

void processButton() {
  int reading = digitalRead(BUTTON_PIN);

  // If the reading has changed, reset the debounce timer
  if (reading != previousButtonState) {
    lastDebounceTime = millis();
  }

  // Update button state only if the reading is stable for the debounce period
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (reading != currentButtonState) {
      currentButtonState = reading;

      // Button pressed (active high)
      if (currentButtonState == HIGH) {
        buttonPressStartTime = millis();
        holdEventTriggered = false;  // Reset the hold flag for the new press
      }
      // Button released
      else {
        if (!holdEventTriggered) {
          unsigned long pressDuration = millis() - buttonPressStartTime;
          if (pressDuration >= HOLD_TIME) {
            handleHold();  // In case the button is released after the hold time
          } else {
            // Check for double click
            if (waitingForDoubleClick && ((millis() - buttonReleaseTime) < DOUBLE_CLICK_TIMEOUT)) {
              handleDoubleClick();
              waitingForDoubleClick = false;
            } else {
              waitingForDoubleClick = true;
              buttonReleaseTime = millis();
            }
          }
        }
      }
    }
  }

  // Check for a hold event while the button remains pressed
  if (currentButtonState == HIGH && !holdEventTriggered) {
    if (millis() - buttonPressStartTime >= HOLD_TIME) {
      handleHold();
      holdEventTriggered = true;
      waitingForDoubleClick = false;  // Cancel any pending click if hold is triggered
    }
  }

  // If waiting for a second click and timeout has expired (with the button released), register a single click
  if (waitingForDoubleClick && ((millis() - buttonReleaseTime) > DOUBLE_CLICK_TIMEOUT) && (currentButtonState == LOW)) {
    handleSingleClick();
    waitingForDoubleClick = false;
  }

  previousButtonState = reading;
}

// Function to handle a single click event
void handleSingleClick() {
  Serial.println("single click turning screen on");
  if (!screenIsOn) {
    screenIsOn = true;
    display.ssd1306_command(SSD1306_DISPLAYON);  // Turn screen on
  }
}

// Function to handle a double click event
void handleDoubleClick() {
  Serial.println("Double Click");
  //linescale->resetMaxMin(); //reset max and min
  sendResponse();
}

// Function to handle a hold event
void handleHold() {
  isUpdatingOLED = true;
  Serial.println("Hold");
  // Configure BUTTON_PIN as a wake-up source on HIGH signal (since button is active high)
  esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_PIN, HIGH);

  Serial.println("ESP32 will sleep now. Press the button to wake up.");
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  String noDataText = "Going to sleep. Press button to wake.";
  int16_t x1, y1;
  uint16_t textWidth, textHeight;
  display.getTextBounds(noDataText.c_str(), 0, 0, &x1, &y1, &textWidth, &textHeight);

  int xPos = (SCREEN_WIDTH - textWidth) / 2;    // Center horizontally
  int yPos = (SCREEN_HEIGHT - textHeight) / 2;  // Center vertically

  display.setCursor(xPos, yPos);
  display.print(noDataText);


  display.display();
  delay(5000);                                  // Short delay to ensure the message gets printed
  display.ssd1306_command(SSD1306_DISPLAYOFF);  // Turn screen off
  // Put ESP32 into deep sleep
  esp_deep_sleep_start();
}

void sendResponse() {
  responseMessage.measuredValue = 0.0;
  responseMessage.maxValue = 0.0;
  responseMessage.hertz = 40;
  strcpy(responseMessage.unit, "kN");  // Acknowledge message
  responseMessage.batteryLevel = 0;
  responseMessage.commandByte = 0xA5;  // Example command value

  Serial.println("Sending response...");


  //ESPNOW does not verify receipt when in broadcast mode, so spam 40 messages hoping that some will get through.
  for (int i = 0; i < 40; i++) {
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)&responseMessage, sizeof(responseMessage));
    //esp_err_t result = esp_now_send(senderMac, (uint8_t*)&responseMessage, sizeof(responseMessage));
    delay(1);
    if (result != ESP_OK) {
      Serial.print("Error sending message: ");
      Serial.println(esp_err_to_name(result));
    }
  }
}
