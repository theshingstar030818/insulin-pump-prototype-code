/*
 * Code for CGM
 * ENGR 490, Winter 2015
 *
 *
 * Paul Giessner (EECS)
 * giessnerl@umich.edu
 * Ryan Lanigan (MSE)
 * rlanigan@umich.edu
 * April 14, 2015
 * 
 */

#include <SoftwareSerial.h>

// Bluetooth communication setup
const int bluetoothTx = 10;  // TX-O pin of bluetooth mate, Arduino D2
const int bluetoothRx = 11;  // RX-I pin of bluetooth mate, Arduino D3
SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);

//Variable for bluetooth communication
String data = ""; 

//Timing variables
const int glucose_read_time = 100; //100 miliseconds
const int btooth_read_time = 1000; //1 second
const int full_cycle = 15000;   //15 seconds
int iteration = 0;

//Running every 5 minutes - for validation, 10 seconds
//Saves 2 hours of CGM data to send to the pump (5 min * 24 = 120 minutes)
int calibration = 0; //When calibrating use this as a temp value
int saved_data[24];  //Array of data to be saved
int index = 0;       //Point to begin reading from (read down to 0)
int lost_data = 0;   //Number of data points lost

//Variables for connection establishment
int max_wait = 0; //MUST!!! (max_wait*btooth_read_time) < full_cycle

//Variable for glucose readings
const int V_OUT = 2; //Output pin
const int LED = 3;
const int G_LEVEL = A5;  //Analog Input

// insulin range
const int toLOW = 60;
const int toHIGH = 250;

int val = 0; //Variable for Analog signal

byte serialA;

/////////////////////////////////////////////////////////////////SETUP
void setup()
{
  Serial.begin(9600);  // Begin the serial monitor at 9600bps
  bluetooth.begin(9600);  // Begin the bluetooth module baud rate
      //Need to build some sort of initialization proceedure
  //Glucose reading i/o pins
  pinMode(V_OUT, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(G_LEVEL, INPUT);
  
  delay(300); digitalWrite(LED, LOW);
  delay(300); digitalWrite(LED, HIGH);
  delay(300); digitalWrite(LED, LOW);
  delay(300); digitalWrite(LED, HIGH);
  delay(300); digitalWrite(LED, LOW);
  delay(300); digitalWrite(LED, HIGH);
}

////////////////////////////////////////////////////////////////LOOP
void loop()   // 10 seconds between readings
{
  //Read Glucose
  //read_glucose();
  //Sends data (if possible)
  //send_data();
  //Calibrates (if possible)
  //if(iteration > 143){
  //  calibrate();
  //}
  //Wait for rest of a full cycle and rest parameters
  //delay(full_cycle - (max_wait*btooth_read_time));
  //max_wait = 0;
  //++iteration;
  
  
  digitalWrite(LED, HIGH); // indicate data is being sent
  data = String(read_glucose()); // get reading
  data += 'x';  // add character so pump can know at what point the transmission ends.
  
  // send data
  bluetooth.print(data); 
  digitalWrite(LED, LOW);
  
  // delay interval between data transmission
  delay(10000);
  
}

// returns measurement - for validation, resistance of saline solution
int read_glucose(){
  digitalWrite(V_OUT, HIGH); //Sends voltage
  delay(glucose_read_time);  //Length of time for signal to be sent through the system
  val = analogRead(G_LEVEL); //Reads in the 
  digitalWrite(V_OUT, LOW);  //Stops voltage
  
  // map the reading from (0, 1023) to predefined glucose ranges
  map(val, 0, 1023, toLOW, toHIGH);
  
  saved_data[index] = val;
  
  // data storage
  if(index == 23){
    index = 0;
    ++lost_data;
  }
  else{
    ++index;
    if(lost_data > 0) ++lost_data;
  }
  
  return val;
}
