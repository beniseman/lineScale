#include "LineScale.h"

static BLEUUID serviceUUID("00001000-0000-1000-8000-00805f9b34fb");
static BLEUUID charUUID_RX("00001001-0000-1000-8000-00805f9b34fb");
static BLEUUID charUUID_TX("00001002-0000-1000-8000-00805f9b34fb");

LineScale::LineScaleDataCallback LineScale::dataCallback = nullptr;
LineScale::ConnectionCallback LineScale::connectionCallback = nullptr;
LineScale::DisconnectionCallback LineScale::disconnectionCallback = nullptr;


// Static instance of LineScale
LineScale* LineScale::instance = nullptr;

LineScale::LineScale()
  : pClient(nullptr), pTXCharacteristic(nullptr), pRXCharacteristic(nullptr),
    pServerAddress(nullptr), connected(false),
    maxMeasuredValue(0), minMeasuredValue(99999), lastNotifyTime(0),
    lastCountTime(0), notificationCount(0), lastNotificationRate(0),
    handleTimeoutInProgress(false),
    checkConnection(false),
    macAddress(""),  // Initialize macAddress to an empty string
    debug(false)     // Initialize debug flag to false
{
  BLEDevice::init("");
  instance = this;
}



void LineScale::setDebug(bool enable) {
  debug = enable;
}

void LineScale::debugPrint(const String& message, bool newline) {
  if (debug) {
    if (newline) {
      Serial.println(message);  // Adds newline after the message
    } else {
      Serial.print(message);  // Just prints the message without newline
    }
  }
}

void LineScale::keepthebalance(int timeoutSeconds) {
  handleTimeout(timeoutSeconds);
}

void LineScale::handleTimeout(int timeoutSeconds) {
  if (timeoutSeconds > 0 && timeoutSeconds < 10) {
    timeoutSeconds = 10;
  }
  if (checkConnection && millis() - lastNotifyTime >= 2000 && !isConnected()) {
    // 2 seconds have passed since the last notification, checkConnection is enabled, and device is not connected

    checkConnection = false;  // Disable the flag to prevent repeated checking
    // If not connected, disconnect or handle accordingly
    debugPrint("Disconnected from LineScale!");
    // Optionally, trigger the disconnection callback here
    if (disconnectionCallback) {
      disconnectionCallback();
    }
    return;
  }



  if ((millis() - lastNotifyTime < timeoutSeconds * 1000) || handleTimeoutInProgress || timeoutSeconds == 0 || !isConnected()) {
    return;  // Exit if either condition is true
  }

  handleTimeoutInProgress = true;  // Mark as in progress

  debugPrint("Trying to return to measurement screen: Sending powerButton and startDataStream alternately up to 4 times until data is received...");

  unsigned long startTime = millis();
  unsigned long timeoutMillis = timeoutSeconds * 1000;  // Convert seconds to milliseconds
  delay(200);                                           // Avoid sending power button immediately

  int attempts = 0;
  const int maxAttempts = 4;  // Limit to 4 total attempts

  while (attempts < maxAttempts) {
    if (millis() - startTime > timeoutMillis) {
      debugPrint("Timeout reached, no data received.");
      break;
    }

    if (lastNotifyTime > startTime) {   //problem resolved data stream resumed
      handleTimeoutInProgress = false;  // End the process
      debugPrint("Data received!");
      return;
    }

    sendCommand("O");  // Even attempts send power button
    debugPrint("Power button command sent ", false);
    debugPrint(" attempt #" + String(attempts + 1), false);
    debugPrint("/", false);
    debugPrint(String(maxAttempts));
    delay(200);  // Small delay between commands

    sendCommand("A");
    debugPrint("StartDataStream command send ", false);
    debugPrint(String(attempts + 1), false);
    debugPrint("/", false);
    debugPrint(String(maxAttempts));


    attempts++;
    delay(2000);
  }

  if (attempts == maxAttempts) {
    debugPrint("Max attempts reached. Data stream not detected. Trying again in " + String(timeoutSeconds) + " seconds");
    handleTimeoutInProgress = false;  // End the process and wait another timeout period before trying again
    lastNotifyTime = millis();
  }
}


void LineScale::setDataCallback(LineScaleDataCallback callback) {
  debugPrint("User Notify Callback function called");
  dataCallback = callback;
}
void LineScale::setConnectionCallback(ConnectionCallback callback) {
  debugPrint("Connection callback function");
  connectionCallback = callback;
}

void LineScale::setDisconnectionCallback(DisconnectionCallback callback) {
  debugPrint("Disconnection callback function");
  disconnectionCallback = callback;
}

int LineScale::NotificationRate() {
  return lastNotificationRate;
}

char LineScale::WorkingMode() {
  return workingMode;
}

char LineScale::ZeroMode() {
  return zeroMode;
}

float LineScale::Force() {
  return measuredValue;  // Return the measured force
}

float LineScale::RelativeForce() {
  return relativeForce;
}

float LineScale::ReferenceZero() {
  return referenceZero;
}

std::string LineScale::Unit() {
  return unit;
}

int LineScale::ScanRate() {
  return speed;
}

float LineScale::MaxForce() {
  // Convert max force (stored in kN) to the current unit
  if (unit == "kN") return maxMeasuredValue;
  if (unit == "kgf") return maxMeasuredValue * 101.9716213;
  if (unit == "lbf") return maxMeasuredValue * 224.809;
  return maxMeasuredValue;
}

float LineScale::MinForce() {
  // Convert min force (stored in kN) to the current unit
  if (unit == "kN") return minMeasuredValue;
  if (unit == "kgf") return minMeasuredValue * 101.9716213;
  if (unit == "lbf") return minMeasuredValue * 224.809;
  return minMeasuredValue;
}

int LineScale::BatteryLevel() {
  return batteryLevel;
}

void LineScale::setScanRate(int speedValue) {
  debugPrint("User Notify Callback function called", false);
  debugPrint(String(speedValue), false);
  debugPrint(" Hz");
  if (speedValue == 10) {
    sendCommand("S");  // Sends the command "S" for speed 10
  } else if (speedValue == 40) {
    sendCommand("F");  // Sends the command "F" for speed 40
  } else {
    debugPrint("Invalid speed value. Only 10 and 40 are allowed.");
  }
}




void LineScale::powerButton() {
  debugPrint("Power Button Push");
  sendCommand("O");
}
void LineScale::zeroButton() {
  debugPrint("Zero Button Push");
  sendCommand("Z");
}
void LineScale::setRelativeZeroMode() {
  debugPrint("Set Relative Zero Mode");
  sendCommand("X");
}
void LineScale::setAbsoluteZeroMode() {
  debugPrint("Set Absolute Zero Mode");
  sendCommand("Y");
}
void LineScale::setAbsoluteZero() {
  debugPrint("Set absolute/reference zero");
  sendCommand("T");
}

void LineScale::setUnitKN() {
  debugPrint("Set Unit to kN");
  sendCommand("N");
}

void LineScale::setUnitKGF() {
  debugPrint("Set Unit to kgf");
  sendCommand("G");
}
void LineScale::setUnitLBF() {
  debugPrint("Set Unit to lbf");
  sendCommand("B");
}

void LineScale::startDataStream() {
  debugPrint("Start Data Stream");
  sendCommand("A");  // Sends the command "A" to start the data stream
}

void LineScale::stopDataStream() {
  debugPrint("Stop Data Stream");
  sendCommand("E");  // Sends the command "E" to stop the data stream
}


void LineScale::resetMaxMin() {
  maxMeasuredValue = 0;      // Reset max to 0
  minMeasuredValue = 99999;  // Reset min to a very large value (or a more appropriate value)
  debugPrint("Max and Min values reset.");
  sendCommand("YZ");
}

std::string LineScale::getMAC() const {
  if (pServerAddress) {
    return std::string(pServerAddress->toString().c_str());  // Convert Arduino String to std::string
  }
  return "";  // Return empty string if no address is set
}

void LineScale::setMAC(const std::string& mac) {
  if (mac.empty()) {
    macAddress.clear();  // Clear the stored MAC address
    debugPrint("MAC address cleared.");
  } else {
    macAddress = mac;  // Store the MAC address
    debugPrint("MAC address set to: ", false);
    debugPrint(mac.c_str());
  }
}

bool LineScale::connect(const std::string& mac) {

  // 1. Check if a MAC address is passed in the connect function
  if (!mac.empty()) {
    debugPrint("Connect via: connect(", false);
    debugPrint(String(mac.c_str()), false);
    debugPrint(")");
    pServerAddress = new BLEAddress(mac.c_str());
  }
  // 2. Check if a stored MAC address exists and use it
  else if (!macAddress.empty()) {
    debugPrint("Connect via stored mac address ->setMac ", false);
    debugPrint(String(macAddress.c_str()), false);
    debugPrint(")");
    pServerAddress = new BLEAddress(macAddress.c_str());
  }
  // 3. Perform a scan to find a device if no MAC address is passed or stored
  else {
    return scanAndConnect();
  }

  debugPrint("Connecting to: ", false);
  debugPrint(pServerAddress->toString().c_str());

  if (pClient == nullptr) {
    pClient = BLEDevice::createClient();
  }

  if (!pClient->connect(*pServerAddress)) {
    debugPrint(" - Connection failed!");
    return false;
  }

  debugPrint("Connected to BLE server");
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (!pRemoteService) {
    debugPrint("Service not found!");
    pClient->disconnect();
    return false;
  }

  pTXCharacteristic = pRemoteService->getCharacteristic(charUUID_TX);
  pRXCharacteristic = pRemoteService->getCharacteristic(charUUID_RX);

  if (!pTXCharacteristic || !pRXCharacteristic) {
    debugPrint("Characteristics not found!");
    pClient->disconnect();
    return false;
  }

  pTXCharacteristic->registerForNotify(notifyCallback);
  connected = true;

  lastNotifyTime = millis();  // reset value for timeouts
                              //assume data stream is desired
  startDataStream();
  delay(200);
  startDataStream();

  // Trigger connection callback
  if (connectionCallback) {
    connectionCallback();
  }

  // Set checkConnection to true after successful connect
  checkConnection = true;

  return true;
}

bool LineScale::scanAndConnect() {
  debugPrint("Scannign for LineScales based on serviceUUID");
  BLEScan* pScan = BLEDevice::getScan();
  pScan->setActiveScan(true);
  BLEScanResults* results = pScan->start(5);

  for (int i = 0; i < results->getCount(); i++) {
    BLEAdvertisedDevice device = results->getDevice(i);
    if (device.haveServiceUUID() && device.getServiceUUID().equals(serviceUUID)) {
      debugPrint("Found matching LineScale device!");
      pServerAddress = new BLEAddress(device.getAddress());
      return connect(std::string(pServerAddress->toString().c_str()));
    }
  }

  debugPrint("No LineScales found.");
  return false;
}

void LineScale::notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  if (length != 20 || !instance) return;  // Ignore incorrect lengths or null instance

  // Count valid notifications
  instance->notificationCount++;

  // Check if one second has passed
  unsigned long currentMillis = millis();
  if (currentMillis - instance->lastCountTime >= 1000) {
    instance->lastCountTime = currentMillis;
    instance->lastNotificationRate = instance->notificationCount;  // Store notification rate for display
    instance->notificationCount = 0;
  }

  char receivedData[21] = { 0 };  // +1 for null termination
  for (int i = 0; i < 20; i++) {
    receivedData[i] = isprint(pData[i]) ? (char)pData[i] : '.';
  }

  instance->workingMode = receivedData[0];

  // Parse measured value
  char tempMeasuredValue[7] = { 0 };
  strncpy(tempMeasuredValue, &receivedData[1], 6);
  float parsedValue = atof(tempMeasuredValue);

  instance->zeroMode = receivedData[7];

  // Reference Zero
  char tempReferenceZero[7] = { 0 };
  strncpy(tempReferenceZero, &receivedData[8], 6);
  instance->referenceZero = atof(tempReferenceZero);

  // Determine the unit
  switch (receivedData[15]) {
    case 'N': instance->unit = "kN"; break;
    case 'G': instance->unit = "kgf"; break;
    case 'B': instance->unit = "lbf"; break;
    default: instance->unit = "?"; return;
  }

  // Determine speed
  switch (receivedData[16]) {
    case 'S': instance->speed = 10; break;
    case 'F': instance->speed = 40; break;
    default: instance->speed = -1; return;
  }

  // Handle Zero Mode
  if (instance->zeroMode == 'N') {
    instance->measuredValue = parsedValue;
    instance->relativeForce = 0;
  } else if (instance->zeroMode == 'Z') {
    instance->relativeForce = parsedValue;
    instance->measuredValue = instance->relativeForce + instance->referenceZero;
  }

  // Convert to kN for min/max tracking
  float measuredValueKN = instance->convertToKN(instance->measuredValue, instance->unit.c_str());

  // Track max/min in kN
  if (measuredValueKN > instance->maxMeasuredValue) {
    instance->maxMeasuredValue = measuredValueKN;
  }
  if (measuredValueKN < instance->minMeasuredValue) {
    instance->minMeasuredValue = measuredValueKN;
  }

  // Convert stored kN max/min back to current unit for display
  float displayMax = (instance->unit == "kN") ? instance->maxMeasuredValue : (instance->unit == "kgf") ? instance->maxMeasuredValue * 101.9716213
                                                                           : (instance->unit == "lbf") ? instance->maxMeasuredValue * 224.809
                                                                                                       : instance->maxMeasuredValue;

  float displayMin = (instance->unit == "kN") ? instance->minMeasuredValue : (instance->unit == "kgf") ? instance->minMeasuredValue * 101.9716213
                                                                           : (instance->unit == "lbf") ? instance->minMeasuredValue * 224.809
                                                                                                       : instance->minMeasuredValue;

  // Battery Level
  instance->batteryLevel = ((int)receivedData[14] - 0x20) * 2;

  instance->lastNotifyTime = millis();


  if (dataCallback) {
    dataCallback();  // Call the user's function
  }
}

float LineScale::convertToKN(float value, const char* unit) {
  if (strcmp(unit, "kN") == 0) return value;
  if (strcmp(unit, "kgf") == 0) return value / 101.9716213;
  if (strcmp(unit, "lbf") == 0) return value / 224.809;
  return value;
}

bool LineScale::isConnected() {
  return connected && pClient->isConnected();
}

void LineScale::disconnect() {
  if (pClient && connected) {
    pClient->disconnect();
    connected = false;
  }
  // Trigger disconnection callback
  if (disconnectionCallback) {
    disconnectionCallback();
  }
}

void LineScale::sendCommand(const std::string& command) {
  if (!pRXCharacteristic || !connected) {
    //debugPrint("Error: Not connected or RX characteristic unavailable.");
    return;
  }

  size_t commandLength = command.length();

  for (size_t i = 0; i < commandLength; i++) {
    uint8_t firstByte = static_cast<uint8_t>(command[i]);
    uint8_t secondByte = 0x0D;
    uint8_t thirdByte = 0x0A;
    uint8_t checksum = firstByte + secondByte + thirdByte;

    uint8_t commandPacket[4] = { firstByte, secondByte, thirdByte, checksum };


    char formattedMessage[32];  // Increased size to 32
    sprintf(formattedMessage, "Sending command: %02X %02X %02X %02X",
            commandPacket[0], commandPacket[1], commandPacket[2], commandPacket[3]);

    debugPrint(String(formattedMessage), true);

    pRXCharacteristic->writeValue(commandPacket, 4);

    if (i < commandLength - 1) {
      delay(200);
    }
  }
}
