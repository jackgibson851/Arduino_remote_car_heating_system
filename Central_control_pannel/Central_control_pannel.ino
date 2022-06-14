// Created  :  2 / 11 / 2020 
// Modified :  14/ 06 / 2022
// Written by: Jack Gibson for EEE527 Project 
// Description: Code for Arduino MKR 1000 - Module mounted to wall in case in the home   
//                                        - Reads Data from external LED from Arduin Nano 33 BLE Sense and compares with temperature values sends output to Thingspeak server    
//                                        - If LED value is HIGH (<1010) AND temperature read from Thingspeak <= ThresholdValue, Send HIGH value (1) to Wakeword Channel on Thingspeak     
//                                        - Uses output of Wakeword status to display messeges to the user via LCD    


#include <SPI.h>             // SPI library used to process data from photoresistor sensor and send it over WiFi-101  
#include <WiFi101.h>         // WiF-101 Library used to connect MKR 1000 to Wifi 
#include<ThingSpeak.h>       // ThingSpeak used to analyse data 
#include<LiquidCrystal.h>    // Liquid Crystal library used to simplify code required to output messeges to the LCD screen 

const char ssid[] = "___________";        //  WiFi Name (ssid) defined  
const char password[] = "__________";    // Wifi Password defined 
int status = WL_IDLE_STATUS;               // status innitially defined as idle (available for connection) 
WiFiClient client;                         // WiFi client defined as client for use in WiFi-101 library 

unsigned long wakewordChannelNumber =  1221517;    // set Channel ID for Wakeword  
const char * myWriteAPIKey = "IRXL27JIMVHMU1BB";   // Set Write API Key for Wakeword Channel    

unsigned long CarHeaterChannelNumber = 1224137;    // Channel number for Car heater 
unsigned int temperatureFieldNumber = 2;           // Temp values stored in Field no. 2 
const char * myReadAPIKey = "1CAZ6CZYEX1GBLKY";    // API key for Car Heater Channel 

const int  ledG = 7;         // Green LED at pin 7
const int ledR = 6;          // Red LED at pin 6 
int lightSensor = A2;        // Light Sensor data to pin A2
int val = 0;                 // Innital Value for photoresistor set to 0    
static byte A = 0;           // used to show status of Trigger (wakeword) 1-ON,  0-OFF
unsigned long start_time;    // start time defined for timer 
int ThresholdVal = 90;       // Threshold value set for temperature readings

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;    // define pins of LCD
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);                     // configure pins to ports on LCD


void setup() {

  pinMode(ledG, OUTPUT);   //Green LED defined as OUTPUT
  pinMode(ledR, OUTPUT);   // Red LED defined as OUTPUT
  start_time = millis();   // Start_time set to arduino's internal timer 

  lcd.begin(16, 2);                // dimention of LCD defined as [16 x 2] and library begin 
  lcd.print("    Wait for   ");    
  lcd.setCursor(0,1);              // Prints "Wait for WiFi Connection" across 2 lines in LCD while Wifi isn't connected  
  lcd.print("WiFi Connection");
  Serial.begin(9600);              // Baud Rate set to 9600
/*
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");  // Attempt to connect to Wifi network:
    Serial.println(ssid);
*/ 
    status = WiFi.begin(ssid, password);    // Connect to WiFi network:
    delay(10000);                           // Wait 10 seconds for connection:
  
  ThingSpeak.begin(client);                             // Begin Thingspeak Library 
  Serial.println("You're connected to the network");    // Print to Serial Monitor once connected to WiFi Network 
  
  lcd.setCursor(0,0);
  lcd.print("   Connected!  ");    // Print messege to LCD across 2 lines to start heater 
  lcd.setCursor(0,1);
  lcd.print(" State Command "); 
 
  }


void loop() {

  int statusCode = 0;    // Define innitial statusCode to 0 to show correct error code in debugging or display correct value from server  
  
float temperatureInC = ThingSpeak.readFloatField(CarHeaterChannelNumber, temperatureFieldNumber);    //Temperature value defined by reading from Car Heater Channel on Thingspeak server

 statusCode = ThingSpeak.getLastReadStatus();    // Read the last value sent to the server in the Car Heater Channel 
 if(statusCode == 200){
   Serial.println("Car Temperature is  " + String(temperatureInC) + " deg C");    // If Status code is 200, Print "Car temp. is (temp value) deg C" in Serial Monitor 
 }
 else{
   Serial.println("Problem reading channel. HTTP error code " + String(statusCode));    // print the error code if channel can't be read correctly 
 }
 

    val = analogRead(lightSensor);  // Read value of photoresistor 
/*  Serial.print(val);
    Serial.print("\n");
  */
  
  if (val < 1010 && temperatureInC <= (ThresholdVal)) {    // Check if LED is lit on BLE side (if HIGH analog value will be < 1010) AND temperature is less than or equal to threashold value
    digitalWrite(ledG, HIGH); // turn Green LED on         // Turn on Green LED
    lcd.setCursor(0,0);
    lcd.print("    FROST!!    ");                          // Print to LCD - "FROST!!   Heating ON" across 2 lines 
    lcd.setCursor(0,1);
    lcd.print("  Heating ON   ");
    (A = 1);                                               // Set A (wakeword status) to 1 
  }
  
  if (val < 1010 && temperatureInC > (ThresholdVal)) {         // check if LED is lit on BLE side (if HIGH analog value will be < 1010) AND temperature is greater than threashold value 
    digitalWrite(ledR, HIGH); // turn Red LED on               // Turn on Red LED 
    lcd.setCursor(0,0);
    lcd.print("Temp is " + String(temperatureInC) + "deg");    // Print to LCD - "Temp is [temperatureInC] deg No Heating" across 2 lines  
    lcd.setCursor(0,1);
    lcd.print("  No Heating   ");
    (A = 0);                                                   // Set A (wakeword status) to 0
  }
  
  if(millis()-start_time > 600000) {    // If internal timer passes 10 mins (600000ms)  
   start_time = millis(); 
   A = 0;                               // Reset A to 0 
   lcd.setCursor(0,0);
   lcd.print(" Program reset ");        // Print to LCD "Program Reset Press Restart" across 2 lines      
   lcd.setCursor(0,1);
   lcd.print(" Press Restart ");
   digitalWrite(ledG, LOW);             // Set Green LED to LOW
   digitalWrite(ledR, LOW);             // Set Red LED to LOW 
   }
  
  Serial.print(A);       // Print A (wakeword status) to Serial Monitor 
  Serial.print("\n");    // Take a new line 

  ThingSpeak.setField(1,A);                                        // Write Wakeword Status (A) to field 1 in Channel 
  ThingSpeak.writeFields(wakewordChannelNumber, myWriteAPIKey);    // Define Channel number and API Key to Write value to 

  delay(1000);    // Delay for 1 sec to update serial montitor every second and every 15th value is sent to Thingspeak server  

}
