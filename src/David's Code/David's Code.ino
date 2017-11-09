#include <Wire.h>
//#include
//#include
//#include
#define ASCIIZERO 48 // '0' = 48
#define pi 3.1415926535897932384
#define twoPI (2.0*pi)
#define rad (pi/180.0)
#define dEarthRadius 6371.01 // in km
#define dAstroUnit 149597890 // in km
#define OPTOPINAZ 2
#define OPTOPINZE 3
#define MOTZECWCCW 5
#define MOTZECLK 4
#define MOTAZCWCCW 7
#define MOTAZCLK 6
#define MAXAZHOMESTEP 6250
#define MAXZEHOMESTEP 10000 // ZE home max subject to revision
#define STEPSAZHOME 1800 // AZ home position = East = +90 degrees = 1800 steps ***Update for final version => Home = 22.5 degrees***
#define ANGLEZEHOME 90.0 // ZE home position = Horizon = +90 degrees
#define STEPDLY 5
#define ZENITHHOMENUTPOSITION 24
#define STEPSPERDEGAZ 20.0 // 1.8 deg per step and 36:1 reduction worm gears
#define STEPSPERMMZE 100 // temporary demo
//Latitude and Longitude for McMaster (JHE) = 43.260181 (N), 79.920892 (W). Latitude is considered positive to the North and longitude to the East.
//Use decimal format (Latitude = 43 + 26.0181/60 = 43.434; Longitude = -1 * (79 degrees + 92.0892/60) = -80.535;)
const double MCMASTERLATITUDE = 43.434;
const double MCMASTERLONGITUDE = -80.535;


//azimuth angle should be between 0-180 degrees



//****************************************************************************************************************************************************************
void setup()
 {
  // setup serial communication
  Serial.begin(9600);
  Serial.println("SolarTracker v4.4");
  Serial.println("Serial Connection initalized");
  Serial.println("");
  pinMode(MOTAZCWCCW,OUTPUT); // AZ motor
  pinMode(MOTAZCLK, OUTPUT);
  pinMode(OPTOPINAZ, INPUT); // opto slot sensor AZ
  //digitalWrite(MOTAZCWCCW,HIGH); // always go home CCW = HIGH
  pinMode(MOTZECWCCW,OUTPUT); // EL motor
  pinMode(MOTZECLK, OUTPUT);
  pinMode(OPTOPINZE, INPUT); // opto slot sensor ZE
  //digitalWrite(MOTZECWCCW,HIGH); // always go home CCW = HIGH
  Serial.println("");
  // home the AZ stepper by looking for blocked opto slot, when home = East = 90 degrees = 1800 steps
  // home the Azimuth stepper
  homeAzimuth();
  trackSun();

  
} // end setup()

//****************************************************************************************************************************************************************
void loop(){
  int iCount;
  Serial.println("loop");
  delay(2000);
  int optsensAZ;
  int optsensZE;
  optsensAZ = digitalRead(OPTOPINAZ);
  optsensZE = digitalRead(OPTOPINZE);
  if (optsensAZ == HIGH){
    Serial.println("AZ triggered");
    }
  if (optsensZE == HIGH){
    Serial.println("ZE triggered");
    }
  
}

//****************************************************************************************************************************************************************



void trackSun(){
  //might want to change parameter to degrees instead of steps for stepAzimuth and stepZenith
  stepAzimuth("CW", 2700); //45 degrees b/c 1 step is 0.05 for azimuth because 1 step is 1.8 degrees of the worm drive which has a GR of 36
  stepZenith("CCW", 20); //20 steps * 1.8 = 36 degrees
  }

//****************************************************************************************************************************************************************
void homeAzimuth(){
  Serial.println("Moving the azimuth to the home position");
  int i;
  int optsensAZ;
  for (i = 0; i <= 180; i++){
    optsensAZ = digitalRead(OPTOPINAZ);
    if (optsensAZ == HIGH){
      Serial.println("***AZIMUTH OPTICAL SENSOR TRIGGERED***");
      break;
      }

    stepAzimuth("CW", 10);
    
    }
  Serial.println("Azimuth angle moved 180 degrees without reaching optical sensor.");  
  }

//****************************************************************************************************************************************************************

void stepAzimuth(String dir, int steps){
  Serial.println("Stepping Azimuth motor...");
  int i;
  if (dir == "CW"){
    digitalWrite(MOTAZCWCCW, LOW);
    }
  else if (dir == "CCW"){
    digitalWrite(MOTAZCWCCW, HIGH);
    }

  else{
    Serial.println("ERROR: direction not specified properly... setting zenith to CW");
    digitalWrite(MOTAZCWCCW, LOW);
    }
    
  for (i = 0; i < steps; i++){     // STEP 1.8 DEG (with 36 reduction = 0.05 deg) 
    Serial.println(i);
    digitalWrite(MOTAZCLK, HIGH);
    delay(STEPDLY);
    digitalWrite(MOTAZCLK, LOW);
    delay(STEPDLY);
    }
  }
  
//******************************************************************************************************************************************************************
void stepZenith(String dir, int steps){
  Serial.println("Stepping Zenith motor...");
  int i;
  if (dir == "CW"){
    digitalWrite(MOTZECWCCW, LOW);
    }
  else if (dir == "CCW"){
    digitalWrite(MOTZECWCCW, HIGH);
    }

  else{
    Serial.println("ERROR: direction not specified properly... setting azimuth to CW");
    digitalWrite(MOTZECWCCW, LOW);
    }
    
  for (i = 0; i < steps; i++){     // STEP 1.8 DEG (with 36 reduction = 0.05 deg)
    Serial.println(i);
    digitalWrite(MOTZECLK, HIGH);
    delay(STEPDLY);
    digitalWrite(MOTZECLK, LOW);
    delay(STEPDLY);
    }
  }
