// Created :  10 / 10 / 2020
// Modified : 14 / 06 / 2020 
// Written by: Jack Gibson for EEE527 Project 
// Description: Code for Arduino Nano 33 IOT - Module in Car
//                                           - Reads Data from DHT and logs
//                                             to thingspeak server  
//                                           - If Wakeword HIGH value (1) 
//                                              read from thingspeak server
//                                              turn DC Motor on   
//                                           - Logs Value of DC Motor ON(1) 
//                                           or OFF(0) to thingspeak server   


// SPI library used to process data and send it over WiFiNINA   
#include <SPI.h>            
// WiFiNINA Library used to connect Nano 33 IOT to Wifi 
#include <WiFiNINA.h>    
// ThingSpeak used to analyse data    
#include<ThingSpeak.h>    



const char ssid[] = "____________";       //  WiFi Name (ssid) defined 
const char password[] = "_________";   // Wifi Password defined 
int status = WL_IDLE_STATUS;               //status innitially defined as idle (available for connection) 
WiFiClient client;                        // WiFi client defined as client for use in WiFiNINA library 

int tempPin = A1;   // A1 defined as Vout pin of TEMP36GZ sensor 

unsigned long CarHeaterChannelNumber =  1224137;   // set Channel ID for Car Heater  
const char * myWriteAPIKey = "1CAZ6CZYEX1GBLKY";   // Set Write API Key for Car Heater channel 

unsigned long wakewordChannelNumber = 1221517;     //Wakeword channel number 
unsigned int wakewordFieldNumber = 1;              // field 1 in channel 
const char * myReadAPIKey = "IRXL27JIMVHMU1BB";    //API key for Wakeword Channel 

const int transistorPin = 9;    // transistor pin used for controling DC motor defined
                 

void setup() {

  pinMode(transistorPin, OUTPUT); //Transistor used for running DC motor defined as OUTPUT
  Serial.begin(9600);             // Baud rate defined as 9600 

  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");  // Attempt to connect to Wifi network:
    Serial.println(ssid);
  
  status = WiFi.begin(ssid, password);   // Connect to WPA/WPA2 network:
  delay(10000);                          // Wait 10 seconds for connection:
  
  ThingSpeak.begin(client);                             // Begin Thingspeak library 
  Serial.println("You're connected to the network");    // Once connected Disply serial messege 
  
  analogReference(AR_DEFAULT);        // AREF has been set to default 3.3V 
  analogReadResolution(8);            // ADC bits have been set to 8 
  
}

}

void loop(){
  
 int statusCode = 0;    // Define innitial statusCode to 0 to show correct error code in debugging or display correct value from server    
 
 int wakewordStatus = ThingSpeak.readIntField(wakewordChannelNumber, wakewordFieldNumber);    //Wakeword Status defined by reading from Wakeword Channel on Thingspeak server 

 int Qlevel;                      // local int Qlevel defined 
 float volt, temperature ;        // Local float value set up for Voltage passing though Vs and Vout and calculated Temperature value 


 statusCode = ThingSpeak.getLastReadStatus();      // Read the last value sent to the server in the Wakeword Channel 
 if(statusCode == 200){                           
   Serial.println("Wakeword Status is  " + String(wakewordStatus));   // If Status code is 200, Print "Wakeword Status is (Wakeword value)" 
 }
 else{
   Serial.println("Problem reading channel. HTTP error code " + String(statusCode));  // print the error code if channel can't be read correctly 
 }
 

  int outputValueON = 200;          // Set speed of DC motor to 200 (ON)
  int outputValueOFF = 0;           // Set Speed of DC motor to 0 (OFF)
  int Z;                            // Set Z as intager to define output state of motor to send to Thingspeak server 
 
if (wakewordStatus == 1){
  analogWrite(transistorPin, outputValueON);    // If Wakeword Value is HIGH (1) Set DC motor ON    
  (Z = 1);                                      // Define int Z as 1   
 }
 if (wakewordStatus == 0) {                
  analogWrite(transistorPin, outputValueOFF);   // If Wakeword Value is LOW (0) Set DC motor OFF    
  (Z = 0);                                      // Define int Z as 0 
 }



Qlevel = analogRead(tempPin);                  // Q level is read from Vout of the Temp. sensor 
volt = (( Qlevel * 3.3) / 255);                // Voltage is calculated by multiplying the Qlevel by the input voltage (3.3V) and deviding by the number of levels for 8 bits  
temperature = ((volt - 0.5)/0.01);             // Temperature defined by simple algorithm derived from data sheet of TEP36QZ 
Serial.print("Temperature value is:  ");       // Print Temperature value is: 
Serial.print(temperature);                     // Print the temperature value 
Serial.print('\n');                            // Take a new line 


  
  Serial.print(" Motor Binary Val:  ");       
  Serial.print(Z);                         //Print Binary Value of DC Motor: (Z) and take new line
  Serial.print('\n');


ThingSpeak.setField(2,temperature);                             // Write Temperarture (t) to Field 2 in Channel  
ThingSpeak.setField(3,Z);                                       // Write Motor State  (Z) to Field 3 in Channel 
ThingSpeak.writeFields(CarHeaterChannelNumber, myWriteAPIKey);  // Define Channel number and API Key to Write values to 

 delay(1000);    // Delay for 1 sec, Only need 1 new value for each time the sever updates (every 15 or 20 seconds)  

}
