#ifndef LINESCALE_H
#define LINESCALE_H

#include <BLEDevice.h>
#include <Arduino.h>

class LineScale {
public:
  LineScale();

  void setDebug(bool enable); // Method to enable/disable debugging

  bool connect(const std::string& mac = "");

  bool isConnected();
  void disconnect();
  void sendCommand(const std::string& command);

  void keepthebalance(int timeoutSeconds=30);  // Function to handle the home screen process
  void handleTimeout(int timeoutSeconds=30);  // Function to handle the home screen process

  typedef void (*LineScaleDataCallback)();
  typedef void (*ConnectionCallback)();     // Callback for successful connection
  typedef void (*DisconnectionCallback)();  // Callback for disconnect
  

  void setDataCallback(LineScaleDataCallback callback);
  void setConnectionCallback(ConnectionCallback callback);
  void setDisconnectionCallback(DisconnectionCallback callback);

    // Getter for current MAC address
  std::string getMAC() const;
  
  void setMAC(const std::string& mac);  // Function to set the MAC address
  

  void setScanRate(int speedValue);  // Function to set the speed
  void setUnitKN();
  void setUnitKGF();
  void setUnitLBF();



    void powerButton();
    void zeroButton();
    void setRelativeZeroMode();
    void setAbsoluteZeroMode();
    void setAbsoluteZero();
    void startDataStream();  // Function to start the data stream
    void stopDataStream();   // Function to stop the data stream

// Function to reset max and min force values
void resetMaxMin();

  int NotificationRate();  // Get last notification rate (notifications per second)
  char WorkingMode();      // Get current working mode
  char ZeroMode();         // Get current zero mode
  float Force();           // Get current force value
  float RelativeForce();   // Get relative force
  float ReferenceZero();   // Get reference zero value
  std::string Unit();      // Get current unit
  int ScanRate();          // Get the scan rate (speed)
  float MaxForce();        // Get maximum force in current unit
  float MinForce();        // Get minimum force in current unit
  int BatteryLevel();      // Get the battery level

private:
  static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);
  bool scanAndConnect();
  float convertToKN(float value, const char* unit);

  static LineScale* instance;

  static LineScaleDataCallback dataCallback;
  static ConnectionCallback connectionCallback;
  static DisconnectionCallback disconnectionCallback;

  BLEClient* pClient;
  BLERemoteCharacteristic* pTXCharacteristic;
  BLERemoteCharacteristic* pRXCharacteristic;

  BLEAddress* pServerAddress;


 bool debug;  // Debug flag
     void debugPrint(const String& message, bool newline = true);  // Helper function for printing debug messages


std::string macAddress;  // Store the MAC address

  bool checkConnection;  // This flag will enable connection checks

  bool connected;

  unsigned long lastCountTime;
  int notificationCount;
  int lastNotificationRate;  // Store last notifications per second

  char workingMode;
  char zeroMode;
  float measuredValue;
  float relativeForce;
  float referenceZero;
  std::string unit;
  int speed;
  float maxMeasuredValue;
  float minMeasuredValue;
  int batteryLevel;
  unsigned long lastNotifyTime;


  

  bool handleTimeoutInProgress = false;  // Ensure it's initialized to false
};

#endif