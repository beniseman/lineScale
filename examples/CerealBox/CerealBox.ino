#include "LineScale.h"

LineScale* linescale;

bool printDataToSerial=false;

void setup() {
    Serial.begin(115200);
    Serial.println("Initializing LineScale...");

    linescale = new LineScale();
    linescale->setDebug(true); // set the debug and print what is happening in the library
    linescale->setCallback(newMeasurementCallback);  // register a callback when data is updated
   
    if (linescale->connect()){
        Serial.println("Connected to LineScale!");
        printCommandList();
    } else {
        Serial.println("Failed to connect.");
    }
}

void loop() {
    if (!linescale->isConnected()) {
        Serial.println("Connection lost. Reconnecting...");
        linescale->connect();
    }

handleSerialInput();
linescale->handleTimeout(10);
}

void handleSerialInput() {
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n'); // Read input
        Serial.print("\nReceived sequence: \"");
        Serial.print(command);
        Serial.println("\"");

        if (!linescale) return;

       
        // If the command contains a space, toggle data printing
        if (command.indexOf(' ') != -1) {
            printDataToSerial = !printDataToSerial;
            Serial.println(printDataToSerial ? "Enabled Data Printing" : "Disabled Data Printing");
            if (!printDataToSerial){
              printCommandList();
            }
            
        }

        // If the command contains 'Q', reset max/min values
        if (command.indexOf('Q') != -1) {
            Serial.println("Resetting Max/Min values...");
            linescale->resetMaxMin();
           
        }

        // Send the remaining command sequence
        if (command.length() > 0) {
            linescale->sendCommand(command.c_str());
        }
        printCommandList();
    }
}

void printCommandList() {
    Serial.println("\n (space) - Toggle serial monitor data printing");
    
    
    Serial.println("\nWorks on locked screen, but not in menus:");
    Serial.println(" A - Start Data Stream (Bluetooth icon on LineScale will be highlighted)");
    Serial.println(" E - Stop Data Stream");

    Serial.println("\nDoes not work on locked screen:");
    Serial.println(" Q - Reset tracked max and min variables - (variables will be reset, but nothing happens on the device if the screen is locked) calls resetMaxMin()");

    Serial.println("\nDocumented Commands (Case sensitive and does not work when screen is locked or when navigating menus):");
    Serial.println(" O - Power Button");
    Serial.println(" Z - Zero Button");
    Serial.println(" N - Switch to kN");
    Serial.println(" G - Switch to kgf");
    Serial.println(" B - Switch to lbf");
    Serial.println(" S - Speed Slow (10Hz)");
    Serial.println(" F - Speed Fast (40Hz)");
    Serial.println(" L - Toggle Zero Mode");
    Serial.println(" X - Set Relative Zero Mode");
    Serial.println(" Y - Set Absolute Zero Mode");
    Serial.println(" T - Set Current Value as Zero in both Relative and Absolute modes");
    Serial.println(" C - Peak Clearing Operation -- doesn't seem to do anything\n");    

    Serial.println("Multiple commands can be sent at once (e.g., \"YZN\" sets absolute zero mode and resets max min).");
    Serial.println("Other commands will still be sent for testing purposes.");
}
