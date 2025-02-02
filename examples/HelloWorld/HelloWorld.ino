
#include <lineScale.h>

LineScale* linescale;


void setup() {
    Serial.begin(115200);
    Serial.println("Initializing LineScale...");

    linescale = new LineScale();
    linescale->setDebug(true); // set the debug and print what is happening in the library
    linescale->setDataCallback(newMeasurementCallback);  // register a callback when data is updated
   
    if (linescale->connect()){
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
linescale->handleTimeout(10);

}

void newMeasurementCallback() {
    Serial.printf(
      "MAC: %s | "
      "Zero Mode: %c | "
      "Force: %.2f %s | "
      "RelForce: %.2f %s | "
      "RefZero: %.2f %s | "
      "ScanRate: %d Hz | "
      "Battery: %d%% | "
      "Min: %.2f %s | "
      "Max: %.2f %s | "
      "Rate: %d Hz\n",
      linescale->getMAC().c_str(),  // Print MAC address
      linescale->ZeroMode(),     // Working mode
      linescale->Force(),           // Force (measured value)
      linescale->Unit().c_str(),    // Current unit (e.g., "kN", "kgf", "lbf")
      linescale->RelativeForce(),   // Relative Force
      linescale->Unit().c_str(),    // Unit for relative force
      linescale->ReferenceZero(),   // Reference zero value
      linescale->Unit().c_str(),    // Unit for reference zero
      linescale->ScanRate(),        // Scan rate (speed)
      linescale->BatteryLevel(),    // Battery level
      linescale->MinForce(),        // Minimum force
      linescale->Unit().c_str(),    // Unit for min force
      linescale->MaxForce(),        // Maximum force
      linescale->Unit().c_str(),    // Unit for max force
      linescale->NotificationRate() // Notifications per second
    );
}

