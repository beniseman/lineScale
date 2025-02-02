
#include <lineScale.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

LineScale* linescale;

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


//button click variables
const int BUTTON_PIN = 2;               // Button connected to digital pin 2 (active high)
int currentButtonState = LOW;           // Current reading from the button
int previousButtonState = LOW;          // Previous reading from the button

unsigned long lastDebounceTime = 0;     // Last time the button state changed
const unsigned long DEBOUNCE_DELAY = 10; // Debounce time in ms

unsigned long buttonPressStartTime = 0; // Timestamp when the button was pressed
bool holdEventTriggered = false;        // Flag indicating if a hold event has been triggered

bool waitingForDoubleClick = false;     // Flag to indicate a potential double click
unsigned long buttonReleaseTime = 0;    // Timestamp when the button was released
const unsigned long DOUBLE_CLICK_TIMEOUT = 250; // Time window to detect a double click (ms)

const unsigned long HOLD_TIME = 3000;   // Time in ms required to trigger a hold event





//void OnDataSent(const uint8_t* mac_addr, esp_now_send_status_t status) {
  //Serial.print("\r\nLast Packet Send Status:\t");
  //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
//}

void OnDataRecv(const uint8_t* mac, const uint8_t* incomingData, int len) {
  Serial.println("ESP-NOW message received! Resetting min/max values...");
  if (linescale && myData.commandByte == 0xA5) {
    linescale->resetMaxMin();  // Reset min/max values
  }
}

void setupESPNOW(){
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
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
 
}

void sendESPNOW(){
    myData.measuredValue = linescale->Force();
    myData.maxValue = linescale->MaxForce();
    myData.hertz = linescale->ScanRate();  // Replace with actual Hz value
    
    strncpy(myData.unit, linescale->Unit().c_str(), sizeof(myData.unit) - 1);  // Use c_str() to convert std::string to const char*
    myData.unit[sizeof(myData.unit) - 1] = '\0';  // Ensure null termination
    //strncpy(myData.unit, linescale->Unit(), sizeof(myData.unit) - 1);
    //myData.unit[sizeof(myData.unit) - 1] = '\0';         // Ensure null termination
    myData.batteryLevel = linescale->BatteryLevel();  // Replace with actual battery percentage

    myData.commandByte = 0xA5;   // Example command value

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)&myData, sizeof(myData));

    if (result == ESP_OK) {
      //Serial.println("Sent with success");
    } else {
      Serial.println("Error sending the data");
    }

}

void newMeasurementCallback() {

  sendESPNOW();
}

void updateOLED() {
  if (isUpdatingOLED) return;  // If update is already in progress, exit early
  isUpdatingOLED = true;  // Set the flag to indicate OLED is being updated

  display.clearDisplay();  // Clear the screen

  // Display battery percentage at top-left
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(linescale->BatteryLevel());
  display.print("%");

  // Display last 8 characters of MAC address at top-center
  String mac = String(linescale->getMAC().c_str());  
  mac = mac.substring(mac.length() - 8);  
  display.setTextSize(1);
  int16_t x1, y1;
  uint16_t textWidth, textHeight;
  display.getTextBounds(mac.c_str(), 0, 0, &x1, &y1, &textWidth, &textHeight);
  int xPosMac = (SCREEN_WIDTH - textWidth) / 2;  
  display.setCursor(xPosMac, 0);
  display.print(mac);

  // Display scan rate at top-right
  display.setTextSize(1);
  String scanRateText = String(linescale->ScanRate()) + "Hz";
  int16_t x2, y2;
  uint16_t textWidth2, textHeight2;
  display.getTextBounds(scanRateText.c_str(), 0, 0, &x2, &y2, &textWidth2, &textHeight2);
  int xPosScanRate = SCREEN_WIDTH - textWidth2 - 1;
  display.setCursor(xPosScanRate, 0);
  display.print(scanRateText);

  // Display force measurement in the center
  char measuredBuffer[10];
  if (strcmp(linescale->Unit().c_str(), "kN") == 0) {
    snprintf(measuredBuffer, sizeof(measuredBuffer), "%.2f", linescale->Force());
  } else {
    snprintf(measuredBuffer, sizeof(measuredBuffer), "%d", (int)round(linescale->Force()));
  }

  display.setTextSize(3);
  display.getTextBounds(measuredBuffer, 0, 0, &x1, &y1, &textWidth, &textHeight);
  int xPosForce = (SCREEN_WIDTH - textWidth) / 2;
  int yPosForce = (SCREEN_HEIGHT - textHeight) / 2;
  display.setCursor(xPosForce, yPosForce);
  display.print(measuredBuffer);

  // --- Display "MAX" Label and Max Force on the Bottom Line ---
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  
  // Create max force text
  String maxForceText;
  if (strcmp(linescale->Unit().c_str(), "kN") == 0) {
    maxForceText = String(linescale->MaxForce(), 2);
  } else {
    maxForceText = String(int(round(linescale->MaxForce())));
  }
  maxForceText +=  String(linescale->Unit().c_str()); // Append unit

  // Get width of max force text
  display.getTextBounds(maxForceText.c_str(), 0, 0, &x1, &y1, &textWidth, &textHeight);
  
  int maxRectWidth = 26;  // Width of "MAX" label box
  int maxRectHeight = 14; // Height of label box
  int spacing = 8;        // Space between "MAX" and max value
  
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
  Serial.println("Initializing LineScale...");
  
  setupESPNOW();
  setupOLED();


  pinMode(BUTTON_PIN, INPUT); // Ensure an external pulldown resistor is used for active high wiring


  // Initialize the LineScale object
  linescale = new LineScale();
  linescale->setDataCallback(newMeasurementCallback);  // register a callback function so you can do what you want with the data

  if (linescale->connect()) {
    Serial.println("Connected to LineScale!");
  } else {
    Serial.println("Failed to connect.");
  }
}

void loop() {
  if (!linescale->isConnected()) {
    Serial.println("Connection lost. Reconnecting...");
    linescale->connect();
  }
  if (screenIsOn){
    updateOLED(); // try putting this is in the callback and see what happens. 1-2 second lag even with flags!
  }
  processButton();
  linescale->keepthebalance(10);  // Example function to check and maintain balance (customizable)
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
  Serial.println("single click, wake screen");
  if (!screenIsOn){
    screenIsOn=true;
    display.ssd1306_command(SSD1306_DISPLAYON);  // Turn screen on
  }
}

// Function to handle a double click event
void handleDoubleClick() {
  Serial.println("Double Click");
  linescale->resetMaxMin(); //reset max and min
}

// Function to handle a hold event
void handleHold() {
  Serial.println("Hold");
  if (screenIsOn) {
    screenIsOn=false;
    display.ssd1306_command(SSD1306_DISPLAYOFF); // Turn screen off
  }
}

