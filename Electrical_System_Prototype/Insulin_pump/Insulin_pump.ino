/*
 * Code for Insulin Pump
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
 
#include <math.h>
#include <SoftwareSerial.h>

/*  using default serial pins instead of 12 and 13
//Phone Bluetooth Serial Port
int phone_btTx = 12;  // TX-O pin of bluetooth mate, Arduino D2
int phone_btRx = 13;  // RX-I pin of bluetooth mate, Arduino D3
SoftwareSerial phone(phone_btTx, phone_btRx);
*/

//CGM Bluetooth Serial Port
int CGM_btTx = 10;  // TX-O pin of bluetooth mate, Arduino D2
int CGM_btRx = 11;  // RX-I pin of bluetooth mate, Arduino D3
SoftwareSerial CGM(CGM_btTx, CGM_btRx);


//"Global" Variables

// pump pins - multiple needed due to current demands of actuator
const int PUMP_1 = 31;
const int PUMP_2 = 33;
const int PUMP_3 = 35;
const int PUMP_4 = 37;
const int PUMP_5 = 39;

// LED pins
const int R_LED = 4;
const int G_LED = 5;
const int B_LED = 6;

// Buzzer
const int BUZZER = 7;

// start timer
unsigned long timerA = millis();

// baud rate
const int baud_rate = 9600;

const int HEIGHT = 47; //height of saline reservoir being pumped from

int iterator = 0; //Iterator for determining pump times

byte serialA;

String data = "";


/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
//Classes

//Personal Information (which influences the insulin data)
class Info{
  //Delcare all the friends here
  private:
    int height, weight, age; //height(in), weight(lb)
    bool gender;             //Returns true is pump user is male
  public:
    //Constructor
    Info() : height(0), weight(0), age(0), gender(0){}    
    void update(int h_in, int w_in, int age_in, int male){
      height = h_in;
      weight = w_in;
      age = age_in; 
      gender = (male == 1);
    } 
    int in() { return height; }
    int lbs() { return weight; }
    int years() { return age; }
};



/////////////////////////////////////////////////////////////////////////////////////////////////////


//Standard Pumping Variables (All Determined with Aid from a Doctor)
class Std_Pump{
  //Declare all the friends here
  private:
    int itoc_ratio, correction_factor;
    int total_dose;
    int min_basal, av_basal, max_basal;
  public:
    //Constructor
    Std_Pump() : itoc_ratio(0), correction_factor(0), total_dose(0),
                min_basal(0), av_basal(0), max_basal(0){}
    void update(int itoc, int correction, int total, int min_b, int av_b, int max_b){
      itoc_ratio = itoc; correction_factor = correction; total_dose = total;
      min_basal = min_b; av_basal = av_b;  max_basal = max_b;
    }
    int ratio() { return itoc_ratio; } 
    int correction() { return correction_factor; } 
    int total() { return total_dose; } 
    int min_b() { return min_basal; } 
    int av_b() { return av_basal; } 
    int max_b() { return max_basal; } 
};
//int itoc_ratio           //Units of Insulin required to reduce X Grams of Carbohydrates
//                         // www.diabeteshealth.com/blog/insulin-to-carbohydrate-ratios/
//int min_basal_rate       //Hourly Dosing which is administered every XX minutes
//int av_basal_rate        //Varies with the time of the day, exercise
//int max_basal_rate
//int correction_factor    //Corrects high/low pre-meal blood sugar
//                         // ccsmed.com/insulin-correction-factor.aspx
//int total_dose           //Total Insulin Dosage Required for the Day


/////////////////////////////////////////////////////////////////////////////////////////////////////

//Information Required for Standard Bolus
class Bolus {
  //Delcare all the friends here
  private:
    int gram_carbs, blood_glucose;
  public:
    //Constructor
    Bolus() : gram_carbs(0), blood_glucose(0){}
    void update(int carbs_in, int b_sugar){
      gram_carbs = carbs_in; blood_glucose = b_sugar;
    }
    int carbs() { return gram_carbs; } 
    int glucose() { return blood_glucose; } 
};



/////////////////////////////////////////////////////////////////////////////////////////////////////

//Saves all the Old CGM Data
class Hist{
 private: 
   int arr[288]; //24 hours of data to sent to phone
   int predict[24]; //2 hours of data for recent use
 public:
   Hist(){
     for(int i = 0; i < 288; ++ i){
       arr[i] = 0;
     }
     for(int i = 0; i < 24; ++ i){
       predict[i] = 0;
     }
   }
   void update(int place, int datum){
     arr[place] = datum;
   }
};



/////////////////////////////////////////////////////////////////////////////////////////////////////

// Not used due to change in bluetooth setup

//String phone_serial = "C,BCF5AC3665E4";
//String cgm_serial = "C,301501061759";

/*

class Btooth{
  private:
    String address;
    int LED;
  public:
    //Constructor
    Btooth(int Tx, int Rx) : address(""), LED(0), serial(Tx, Rx){
      serial.begin(baud_rate);
    }
    //Unable to Change Phone Serial
    void update(String in, int pin){
      address = "C,";
      address += in; //Reads in the CGM Bluetooth "Code"
      LED = pin;
    }
    String s_address(){
      return address;
    }
    SoftwareSerial serial;
    //Commands To Send/Receive Data
    //Receive
    //Initialize     (Z)
    //Bolus          (B)
    //Calibrate CGM  (K)
    //View           (V)

    //Controls connecting to/from bluetooths
    void connect_btooth(){         //Connect with the Bluetooth with the command "serial_code"
      serial.print("$$$");      // Print three times individually to enter command mode
      delay(100);                  // Short delay, wait for the Mate to send back CMD
      serial.println(address);   // Connect to phone
      delay(5000);                 // delay
      digitalWrite(LED, HIGH);
    }
   void disconnect_btooth(){      //Disconnect from Bluetooth Serial
                                   // Do I need to re-enter "Command Mode"
        serial.println("e");    // Sends Command to disconnect
        delay(1000);               // Short delay
        digitalWrite(LED, LOW);
    }
};
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////


Info info();
Std_Pump std_pump();
Bolus bolus();

//Btooth CGM(10, 11);
//Btooth phone(12, 13);


/////////////////////////////////////////////////////////////////////////////SETUP
void setup()
{
  // initialize the serial communication:
  Serial.begin(baud_rate); //baud rate - make sure it matches that of the module you got:
  // DEFAULT BAUD RATES: RN-42 = 115200, HC-06 = 9600
  // handled in class: bluetooth.begin(9600);  // Start bluetooth serial at 9600bps
  
  //Serial.begin(baud_rate);
  CGM.begin(baud_rate);
  
  //Setup Pins
  pinMode(PUMP_1, OUTPUT);
  pinMode(PUMP_2, OUTPUT);
  pinMode(PUMP_3, OUTPUT);
  pinMode(PUMP_4, OUTPUT);
  pinMode(PUMP_5, OUTPUT);
  
  pinMode(R_LED, OUTPUT);
  pinMode(G_LED, OUTPUT);
  pinMode(B_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  
  // Active low LEDs
  digitalWrite(R_LED, HIGH);
  digitalWrite(G_LED, HIGH);
  digitalWrite(B_LED, HIGH);
  
  //Setup the Bluetooth & phone connections
  //phone.update("BCF5AC3665E4", Phone_LED);
  //CGM.update("301501061759", CGM_LED);
  
  
  //Initializes: Connection, Pump Data, and CGM Calibration
  //initialize_system();
  
  // PUMP ON LED FLASH + BUZZER:
  delay(300); digitalWrite(G_LED, LOW);
  delay(300); digitalWrite(G_LED, HIGH);
  digitalWrite(BUZZER, HIGH);
  delay(300); digitalWrite(G_LED, LOW);
  delay(300); digitalWrite(G_LED, HIGH);
  digitalWrite(BUZZER, LOW);
  delay(300); digitalWrite(G_LED, LOW);
  delay(300); digitalWrite(G_LED, HIGH);
 

}


////////////////////////////////////////////////////////////////////////////// LOOP
//Cycles though for a "5 minute" loop -- for validation, we used 10 second loop
void loop() {
  // reset data
  serialA = 0;
  data = "";
  
  // receive data form CGM
  while (CGM.available() > 0)
  {
    data = CGM.readStringUntil('x');
    Serial.print(data); // prints to phone
    //phone.print(data);  // prints to phone (alt. setup)                                  
    
    // reset timer
    timerA = 0;
    timerA = millis();
  }
  
  // if 20 seconds go by without CGM data, alert user using buzzer and red light
  if (timerA > 20000) 
  {
     digitalWrite(BUZZER, HIGH);
     digitalWrite(R_LED, LOW);  // active low LED
     delay(1000);
  }  
  else
  {
     digitalWrite(BUZZER, LOW);
     digitalWrite(R_LED, HIGH);
     delay(1000);
  }
  
  
  delay(100);
  
  // receive data from phone
  while (Serial.available() > 0)
  {
    serialA = Serial.read();
  }
  
  delay(100);
  
  // process data from phone
  switch(serialA)
  {
    case 3:
      pump_fluid();
      break;
    default:
      break;
   }
  
  delay(100);
  
}



////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
//Main/Large Functions

////////////////////////////////////////////////////////////////////////////////////////

//Material Properties:
//11.945tsp (1/4 cup) NaCl per 12floz Tap Water
//Density NaCl = 2.17 g/cm^3
//Density Saline = 1.18927 g/cm^3 (assuming complete solubility)

//Equation 1: Actual Voltage Read from Voltage Divider
  //val = (Rsal)/(ra+Rsal) * Vin
  //val is automatically run with map(val, 0, 5, 0, 1023)
  //Voltage to CGM Readings: constrain(val, 600, 900)
  //Voltage to CGM Readings: map(val, 600, 900, 60, 300)
//Equation 2: Total Mass Added
  //Mtot(Saline) = Md(ml)*e^((294.11 - val)/137)
  //val is from 0 tp 1023, Mtot is mass of total saline pumped
  //Md is the starting ml of Distilled water (50ml)
  //Current GLucose - Goal Glucose = delta Glucose (dG)
  //dG ~ amount val must change ~ additional saline neededd
//Equation 3: Pump Actuation Time
  //Setup Using a Gravitational Potential Energy
  //PE = KE ... mgh = (0.5)mv^2 ... v = sqrt(2gh)
  //Volume_Pumped (m^3) = velocity (m/s) * area (m^2) * time(s) [valve is open]
  //Eq.2 (& voltage) is based on mass pumped Mass = Density_Saline * Volume_Pumped
  // Mass = 50(ml) * (e^((294.11 - cur)/137) - e^((294.11 - want)/137))
  // time(s)[valve is open] = (safety_factor)*Mass * 1/(Density_Saline * area * sqrt(2gh))
  
// Bolus pump function
void pump_fluid(){
  //Equation Constants
  int mass, time;             //Units: mg, s
  int height = HEIGHT;        //Units: cm (height over tube outlet)
  
  int density_sal = 1.8927;   //Units: g/cm^3
  int area = 0.04453;         //Units: cm^2
  int gravity = 981;          //Units: cm/s^2
  int SF = 0.75;              //Safety Factor of 4/3
  
  int target_glucose = 100;   //Target Glucose Levels
  int cur_glucose = 200;      //Current Glucose Levels //Read in from Class Hist
  
// ---Basal not used for demo purposes---
//  //Decide Which Basal to Use. [Changes throughout Day]
//  int basal = av_basal;
//  //Target Dosing of Insulin Based on the Correction Factor
//  int insulin_dose = (cur_glucose - target_glucose) / correction_factor;
//  //How many basal dosings are required
//  int basal_dosing = insulin_dose / basal;  
  
  // Target Dosing of Insulin Based on Saline Calculations //Re-map to fit Saline Equations
  map(cur_glucose, 60, 300, 600, 900); map(target_glucose, 60, 300, 600, 900); 
  mass = 50 * (exp((294.11 - cur_glucose)/137) - exp((294.11 - target_glucose)/137));
  // The time to open valve based on the mass needed and other parameters
  time = (SF) * (mass/1000) * 1/(density_sal * area * sqrt(2*gravity*height));
 
  //Actuate Mechanism
  digitalWrite(PUMP_1, HIGH);
  digitalWrite(PUMP_2, HIGH);
  digitalWrite(PUMP_3, HIGH);
  digitalWrite(PUMP_4, HIGH);
  digitalWrite(PUMP_5, HIGH);
  
  // LED on
  digitalWrite(B_LED, LOW);
  
  delay(time * 1000);  // should be time * 1000
  
  digitalWrite(PUMP_1, LOW);
  digitalWrite(PUMP_2, LOW);
  digitalWrite(PUMP_3, LOW);
  digitalWrite(PUMP_4, LOW);
  digitalWrite(PUMP_5, LOW);
  
  // LED off
  digitalWrite(B_LED, HIGH);
   
}

