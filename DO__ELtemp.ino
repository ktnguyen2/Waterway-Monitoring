/*******
 * Wiring Diagram
 *******/

#include <Wire.h> //enable I2C.
#define DO_address 97 //default I2C ID number for EZO DO Circuit.
#include "RTClib.h" // Date and time functions using a DS1307 RTC connected via I2C and Wire lib
RTC_PCF8523 rtc;

#include <SPI.h>
#include <SD.h>

// for the data logging shield, we use digital pin 10 for the SD cs line
const int chipSelect = 10;

char logFileName[] = "dataLT.txt"; // modify logFileName to identify your experiment, for exampe PBR_01_02, datalog1

String dataString; // the main variant to store all data
char DO_data[20];
char in_char;
int time_= 1800;
int i;

void setup() {

  Serial.begin(9600); //enable serial port.
  Wire.begin(DO_address);
  
  Serial.print("RTC is...");
  if (! rtc.begin())  {
    Serial.println("RTC: Real-time clock...NOT FOUND");
    while (1);// (Serial.println("RTC: Real-time clock...FOUND"));
    }
  Serial.println("RUNNING");
  Serial.print("Real-time Clock...");
  if (! rtc.initialized()){
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
  Serial.println("WORKING");
  
  Serial.print("SD card..."); // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) { 
   Serial.println("Failed"); // don't do anything more
   return;
   }
  Serial.println("OK");
  Serial.print("Log File: ");
  Serial.print(logFileName);  
  Serial.println("...");

  File logFile = SD.open(logFileName, FILE_WRITE); // open the file. "datalog" and print the header

  if (logFile) {

    String header = "Date, Time : DO ,,, Temp";
    Serial.println(header);
    logFile.println(header);
    logFile.close();
    Serial.println("READY");
    
    }
  else { 
    Serial.println("error opening datalog"); 
    } // if the file isn't open, pop up an error
  
  delay(1000);
  }

void loop() {
  i = 0;
 
  DateTime now = rtc.now();
  dataString = String(now.year(), DEC);
  dataString += String('/');
  dataString += String(now.month(), DEC);
  dataString += String('/');
  dataString += String(now.day(), DEC);
  dataString += String(' ');
  dataString += String(now.hour(), DEC);
  dataString += String(':');
  dataString += String(now.minute(), DEC);
  dataString += String(':');
  dataString += String(now.second(), DEC);

  Wire.beginTransmission(DO_address); //call the circuit by its ID number
  Wire.write('R');
  Wire.endTransmission(); //end the I2C data transmission
  delay(time_); //wait the correct amount of time for the circuit to complete its instruction
  Wire.requestFrom(DO_address,20,1); //call the circuit and request 20 bytes
  
  while(Wire.available()) { //are there bytes to receive
  in_char = Wire.read(); //receive a byte.
  if ((in_char > 31) && (in_char <127)) { //check if the char is usable (printable)
    DO_data[i] = in_char;
    i+=1;
    }
  if(in_char==0) { //if we see that we have been sent a null command.
    int i=0; //reset the counter i to 0.
    Wire.endTransmission(); //end the I2C data transmission.
    break; //exit the while loop.
    }
  }
  
  dataString += " : ";
  dataString += String(DO_data);
  dataString += " ,,, ";

  float _voltage;
  float sensorReading;
  float _slope = 58.341;
  float _intercept = -57.0209; // intercept value taken directly from Vernier AutoID code. 
  //For some reason, AutoID does not automatically identically sensor as Extra Long Temp Probe
  
  int count;
  int i;
  int sum = 0;

  for (i = 0; i< 10; i++) { // takes average reading of voltage
    count = analogRead(A0);
    sum = sum+count;
  }
  _voltage = sum / 10 / 1024.0 * 5.0;
  sensorReading = _intercept + _voltage * _slope; //for all linear sensors
  sensorReading = Celcius2Fahrenheit(sensorReading);
  dataString += sensorReading;
  dataString += " F";
  
  File dataFile = SD.open(logFileName, FILE_WRITE); // open the file. note that only one file can be open at a time, so you have to close this one before opening another.
  
  if (dataFile) { // if the file is available, write to it
    dataFile.println(dataString);
    dataFile.close();
    Serial.println(dataString); // print to the serial port too:
  }

  else { 
    Serial.println("error opening datalog file"); 
    } // if the file isn't open, pop up an error:
    
    // clear dataString and DO_data
    dataString = "";
    i= 0;
    for (i = 0; i < 20; i++) {    
        DO_data[i] = 0; 
    }
    delay(5000); //delay 5 minutes = 5*60*1000 ms
    } //end main loop

 double Celcius2Fahrenheit(double celsius)
{
   return celsius * 9 / 5 + 32;
}
