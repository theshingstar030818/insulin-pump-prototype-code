/**************************************************************************/
/*!
    @file     angle_moving_avg.ino
    @author   SOSAndroid (E. Ha.)
    @license  BSD (see license.txt)

	read over I2C bus and averaging angle

    @section  HISTORY

    v1.0 - First release
*/
/**************************************************************************/


#include <ams_as5048b.h>
#include <Wire.h>

//unit consts
#define U_RAW 1
#define U_TRN 2
#define U_DEG 3
#define U_RAD 4
#define U_GRAD 5
#define U_MOA 6
#define U_SOA 7
#define U_MILNATO 8
#define U_MILSE 9
#define U_MILRU 10

AMS_AS5048B mysensor;

int led = 38;
int led2 = 40;
int led3 = 42;
int led4 = 44;
int count = 0;
long num_loop = 1;
float encoder_calibration = 0.45;

void setup() {

  //Start serial
  Serial.begin(115200);
  while (!Serial) ; //wait until Serial ready
  Serial.flush();

  //Start Wire object. Unneeded here as this is done (optionnaly) by AMS_AS5048B object (see lib code - #define USE_WIREBEGIN_ENABLED)
  //Wire.begin();

  //init AMS_AS5048B object
  mysensor.begin();

  //set clock wise counting
  mysensor.setClockWise(true);

  //set the 0 to the sensorr
  //mysensor.zeroRegW(0x0);
  
  
  pinMode(led, OUTPUT);   
  pinMode(led2, OUTPUT);   
  pinMode(led3, OUTPUT);   
  pinMode(led4, OUTPUT); 
 // pinMode(led5, OUTPUT); 
  printMenu();
}

void loop() {
  /*
  mysensor.updateMovingAvgExp();
  float oldAngle = mysensor.angleR(U_DEG, false);
  if (oldAngle != 0.0000000000) {
    Serial.print("Angle (degrees): ");
    Serial.println(oldAngle);
  }
  */
  
  // wait for user input
  while(!Serial.available()) ;
  
  // process input
  delay(10);
  String command = Serial.readStringUntil(' ');
  Serial.print("Command: ");
  Serial.println(command);
  
  if (command.equalsIgnoreCase("menu")) {
    printMenu();
    num_loop = 1;
  }
  else if (command.equalsIgnoreCase("open")) {
    openValve();
    num_loop = 1;
  }
  else if (command.equalsIgnoreCase("close")) {
    closeValve();
    num_loop = 1;
  }
  else if (command.equalsIgnoreCase("pulse")) {
    float time = Serial.readString().toFloat();
    Serial.print("Time (ms): ");
    Serial.println(time);
    
    for (int x = 0; x < num_loop; x++) {
      if (time >= 3) {
        openValve();
        delay(time);
        closeValve();
      }
      else { // to enable finer delays when very short
        openValve();
        delayMicroseconds(time*1000);
        closeValve();
      }
      delay(500);
    }
    num_loop = 1;
  }
  else if (command.equalsIgnoreCase("dispense")) {
    float units = Serial.readString().toFloat();
    Serial.print("Amount (U): ");
    Serial.println(units);
    
    for (int x = 0; x < num_loop; x++) {
      mysensor.updateMovingAvgExp();
      float oldAngle = mysensor.angleR(U_DEG, false);
      if (oldAngle != 0.0000000000) {
        Serial.print("Angle (degrees): ");
        Serial.println(oldAngle);
      }
      else {
        Serial.println("Encoder not found");
        Serial.println();
        return;
      }
      
      float newAngle = -1;
      openValve();
      do {
        mysensor.updateMovingAvgExp();
        newAngle = mysensor.angleR(U_DEG, false);
        if (newAngle < oldAngle - 0.5) {
          newAngle = newAngle + 360;
        }
      } while ((newAngle - oldAngle)*encoder_calibration < units);
      closeValve();
      
      Serial.print("Angle (degrees): ");
      Serial.println(newAngle);
      delay(500);
    }
    num_loop = 1;
  }
  else if (command.equalsIgnoreCase("rate")) {
    float rate = Serial.readString().toFloat();
    Serial.print("Rate (U/hr): ");
    Serial.println(rate);
    
    int pulses = 1;
    if (rate > 2.5) {
      pulses = 12;
    }
    else if (rate > 1) {
      pulses = 6;
    }
    else if (rate > 0.25) {
      pulses = 3;
    }
    else {
      pulses = 1;
    }
    
    unsigned long previousMillis = millis();
    unsigned long currentMillis = millis();
    unsigned long delay_time = 3600000;
    delay_time = delay_time/pulses;
    bool first_loop = true;
    do {
      currentMillis = millis();
      if ((currentMillis - previousMillis > delay_time) || first_loop) {
        previousMillis = currentMillis;
        mysensor.updateMovingAvgExp();
        float oldAngle = mysensor.angleR(U_DEG, false);
        if (oldAngle != 0.0000000000) {
          Serial.print("Angle (degrees): ");
          Serial.println(oldAngle);
        }
        else {
          Serial.println("Encoder not found");
          Serial.println();
          num_loop = 1;
          return;
        }
        
        float newAngle = -1;
        openValve();
        do {
          mysensor.updateMovingAvgExp();
          newAngle = mysensor.angleR(U_DEG, false);
          if (newAngle < oldAngle - 0.5) {
            newAngle = newAngle + 360;
          }
        } while ((newAngle - oldAngle)*encoder_calibration < rate/pulses);
        closeValve();
        
        Serial.print("Angle (degrees): ");
        Serial.println(newAngle);
      }
      first_loop = false;
    } while (!Serial.available());
    Serial.println("Stopped rated delivery");
    num_loop = 1;
  }
  else if (command.equalsIgnoreCase("loop")) {
    num_loop = Serial.readString().toInt();
    Serial.print("Number of loops: ");
    Serial.println(num_loop);
    Serial.println("What command would you like to loop? Currently, only pulse and dispense are supported.");
    return;
  }
  delay(100);
  Serial.println("Done");
  
}

void printMenu() {
  Serial.println("Menu");
  Serial.println("Format: Command [number] --- explanation");
  Serial.println("---------------------");
  Serial.println("Menu --------- print menu");
  Serial.println("Open --------- open valve");
  Serial.println("Close -------- close valve");
  Serial.println("Pulse [t] ---- open valve for t milliseconds");
  Serial.println("Dispense [U] - dispense [U] units of water");
  Serial.println("Rate [R] ----- dispense at a rate of [R] units/hr");
  Serial.println("Loop [N] ----- loop another command [N] times with a 1/2 second delay between iterations");
  Serial.println("---------------------");
  Serial.println();
  Serial.flush();
}

void openValve() {
  digitalWrite(led, HIGH);
  digitalWrite(led2, HIGH);
  digitalWrite(led3, HIGH);
  digitalWrite(led4, HIGH);
}

void closeValve() {
  digitalWrite(led, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);
  digitalWrite(led4, LOW);
}

