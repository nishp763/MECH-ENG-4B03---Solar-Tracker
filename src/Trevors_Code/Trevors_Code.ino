#include <Wire.h>
//Define Pins
#define OPTOPINAZ 2 //Optical Sensor Azimuth
#define OPTOPINZE 3 //Optical Sensor Zenith
#define MOTZECWCCW 5 //Motor Zenith Direction
#define MOTZECLK 4 //Motor Zenith Pulse
#define MOTAZCWCCW 7 //Motor Azimuth Direction
#define MOTAZCLK 6 //Motor Aziumuth Pulse
//Constants
#define ASCIIZERO 48 // '0' = 48
#define pi 3.1415926535897932384
#define twoPI (2.0*pi)
#define rad (pi/180.0)
#define dEarthRadius 6371.01 // in km
#define dAstroUnit 149597890 // in km
//Paramaters
#define MAXAZHOMESTEP 7200 // 1 Rev * 200 Steps / rev * 20:1 worm
#define MAXZEHOMESTEP 550 // 0.5 Rev * 200 Steps / rev * 5.5:1 gear
#define ANGLEAZHOME 22.5 // AZ home position = East = +90 degrees
#define ANGLEZEHOME 0 // ZE home position = Horizon = +90 degrees
#define STEPDLY 2 //Default 5
#define StepDelayHomeAZ 1
#define StepDelayHomeZE 5
#define StepDelayMoveAZ 3
#define StepDelayMoveZE 10
#define STEPSPERDEGAZ 20.0 // 1.8 deg per step and 36:1 reduction worm gear for Azimuth
#define STEPSPERDEGZE 3.055 // 1.8 deg per step and 5.5:1 gear reduction for Zenith
#define NormalControlFrequency 10000
#define DebugControlFrequency 100
#define DebugMultiplier 2
//Initialize structures
struct cTime {
int iYear;
int iMonth;
int iDay;
double dHours;
double dMinutes;
double dSeconds;
};
struct cLocation {
double dLongitude;
double dLatitude;
};
struct cSphereCoordinates {
double dZenithAngle;
double dAzimuth;
};
//Initialize Variables
int iErrorAZFlag; // error flag homing AZ
int iErrorZEFlag; // error flag homing ZE
double dAngleAZ; //Current AngleAZ
double dAngleZE; //Current AngleZE
int iDeltaStepsAZ;
int iDeltaStepsZE;
int Mode = 0;
const double MCMASTERLATITUDE = 43.260181;
const double MCMASTERLONGITUDE = -79.920892;
int ControlFrequency = NormalControlFrequency;
struct cTime utcTime;
struct cLocation utcLocation;
struct cSphereCoordinates utcSunCoordinates;
struct cSphereCoordinates utcTargetCoordinates;
struct cSphereCoordinates utcHelioCoordinates;

void setup()
{
// setup serial communication
Serial.begin(9600);
Wire.begin();
Serial.println("SolarTracker");
Serial.println("Serial Connection initalized");
Serial.println("");
//Setup I/O
pinMode(MOTAZCWCCW, OUTPUT); // AZ motor
pinMode(MOTAZCLK, OUTPUT);
pinMode(OPTOPINAZ, INPUT); // opto slot sensor AZ
pinMode(MOTZECWCCW, OUTPUT); // EL motor
pinMode(MOTZECLK, OUTPUT);
pinMode(OPTOPINZE, INPUT); // opto slot sensor ZE
//Set Coordinates
utcLocation.dLatitude = MCMASTERLATITUDE;
utcLocation.dLongitude = MCMASTERLONGITUDE;
Serial.println("Location: McMaster University, Hamilton, ON");
Serial.print(" Latitude (Degrees): "); Serial.println(utcLocation.dLatitude);
Serial.print(" Longitude (Degrees): "); Serial.println(utcLocation.dLongitude);
//Set time
//setTime();

// home the AZ stepper by looking for blocked opto slot, when home = East = 90 degrees = 1800 steps
homeAzimuth();
// home the Zenith stepper
//homeZenith();
//Init Time
getCurrentTime();
setDebugMode();
Mode = 1;
} // end setup()
void loop() {
if (iErrorAZFlag || iErrorZEFlag) {
Serial.println("One of the stages failed to home");
}
else {
getCurrentTime();
Tracking();
}
delay(ControlFrequency);
}
void setDebugMode(){
Mode=1;
ControlFrequency=DebugControlFrequency;
utcTime.dMinutes=0;
utcTime.dHours=0x0B;
utcTime.dSeconds=0;
utcTime.iDay = 0x08;
utcTime.iMonth = 0x06;
}
void setTime() {
// set the time - below corresponds to Monday, August 19, 2013, 17:40 UTC (1:40pm DST in Hamilton)
Serial.println("Set time ");
Wire.beginTransmission(0x68);
Wire.write(0); // point to address of the timekeeping registers
Wire.write(0x00); // set seconds
Wire.write(0x37); // set minutes
Wire.write(0x80 | 0x15); // set hours 24 mode
Wire.write(0x03); // day of week
Wire.write(0x21); // date
Wire.write(0x11); // month
Wire.write(0x15); // year 00-00
Wire.write(0x10); // provide 1 Hz square wave on pin 7
Wire.endTransmission();
}
void homeAzimuth() {
// Serial.println("Homing the Azimuth-tracking stage to 90 degrees (East of North)");
digitalWrite(MOTAZCWCCW, HIGH); // always go home CCW = HIGH
for (int iCount = 0; iCount <= MAXAZHOMESTEP; iCount++)
{
if (digitalRead(OPTOPINAZ)) {
//Serial.println("Azimuth-tracking stage home");
dAngleAZ = ANGLEAZHOME;
iErrorAZFlag = 0;
break; // HIGH is blocked (home)
}
//Take a step
digitalWrite(MOTAZCLK, HIGH); // STEP 1.8 DEG (with 36 reduction = 0.05 deg)
delay(StepDelayHomeAZ);
digitalWrite(MOTAZCLK, LOW);
delay(StepDelayHomeAZ);
if (iCount == MAXAZHOMESTEP) {
iErrorAZFlag = 1;
Serial.println("Azimuth-tracking stage didn't get home");
}
}
}
void homeZenith() {
//Serial.println("Homing the Zenith-tracking stage to 90 degrees (Horizon)");
digitalWrite(MOTZECWCCW, LOW); // always go home CW = LOW
for (int iCount = 0; iCount <= MAXZEHOMESTEP; iCount++)
{
if (digitalRead(OPTOPINZE)) {
//Serial.println("Zenith-tracking stage home");
dAngleZE = ANGLEZEHOME;
iErrorZEFlag = 0;
break;
}
//Take a step
digitalWrite(MOTZECLK, HIGH);
delay(StepDelayHomeZE);
digitalWrite(MOTZECLK, LOW);
delay(StepDelayHomeZE);
if (iCount == MAXZEHOMESTEP) {
iErrorZEFlag = 1;
Serial.println("Zenith-tracking stage didn't get home");
}
}
}
byte convertHEX(byte value) {
//This works for decimal 0-99
return ((value / 16 * 10) + (value % 16));
} // end convertHEX
void getCurrentTime() {
if (Mode) {
Serial.println("Debug Mode");
utcTime.dMinutes += DebugMultiplier;
if (utcTime.dMinutes > 59) {
utcTime.dMinutes -= 60;
utcTime.dHours++;
}
if (utcTime.dHours > 23) {
utcTime.dHours -= 24;
utcTime.iDay++;
}
}
else {
Serial.println("Normal Mode");
Wire.beginTransmission(0x68);
Wire.write(0); // point to address of the timekeeping registers
Wire.endTransmission();
Wire.requestFrom(0x68, 7); // request 7 bytes from DS1307
utcTime.dSeconds = convertHEX(Wire.read());
utcTime.dMinutes = convertHEX(Wire.read());
utcTime.dHours = convertHEX(Wire.read());
Wire.read(); // disregard the day of the week
utcTime.iDay = convertHEX(Wire.read());
utcTime.iMonth = convertHEX(Wire.read());
utcTime.iYear = 2000 + convertHEX(Wire.read());
}
Serial.println("-----------------------------------------------------");
Serial.println("Universal Coordinate Time");
Serial.print(" Time (Hh:Mm:Ss): ");
if ((int)utcTime.dHours < 10) Serial.print("0");
Serial.print((int)utcTime.dHours, DEC);
Serial.print(":");
if ((int)utcTime.dMinutes < 10) Serial.print("0");
Serial.print((int)utcTime.dMinutes, DEC);
Serial.print(":");
if (utcTime.dSeconds < 10) Serial.print("0");
Serial.println((int)utcTime.dSeconds, DEC);
Serial.print(" Date (Dd/Mm/YYYY) ");
if (utcTime.iDay < 10) Serial.print("0");
Serial.print(utcTime.iDay, DEC);
Serial.print("/");
if (utcTime.iMonth < 10) Serial.print("0");
Serial.print(utcTime.iMonth, DEC);
Serial.print("/");
if (utcTime.iYear < 10) Serial.print("0");
Serial.println(utcTime.iYear, DEC);
Serial.println("");
} // end getCurrentTime()
void Tracking() {
SunPos(utcTime, utcLocation, &utcSunCoordinates); // get the solar vector
Serial.println("Solar Vector:");
Serial.print(" Azimuth = "); Serial.println(utcSunCoordinates.dAzimuth);
Serial.print(" Zenith = "); Serial.println(utcSunCoordinates.dZenithAngle);
if (utcSunCoordinates.dZenithAngle > 80.0)
{
Serial.println(" The sun has set - no update");
homeAzimuth();
homeZenith();
}
else
{
iDeltaStepsAZ = (int)((utcSunCoordinates.dAzimuth - dAngleAZ) * STEPSPERDEGAZ);
Serial.print(" iDeltaStepsAZ= "); Serial.println(iDeltaStepsAZ);
MoveMotorAZ(iDeltaStepsAZ);
iDeltaStepsZE = (int)((utcSunCoordinates.dZenithAngle - dAngleZE) * STEPSPERDEGZE);
Serial.print(" iDeltaStepsZE= "); Serial.println(iDeltaStepsZE);
MoveMotorZE(iDeltaStepsZE);
}
}
void MoveMotorAZ(int iDeltaStepsAZ) {
int iCount;
//Serial.print("Moving Azimuth motor this many steps: ");
//Serial.println(iDeltaStepsAZ);
if (iDeltaStepsAZ == 0)
{
return;
} // end if
if (iDeltaStepsAZ > 0)
{
digitalWrite(MOTAZCWCCW, LOW); // positive CW = LOW
}
else
{
iDeltaStepsAZ = -iDeltaStepsAZ;
digitalWrite(MOTAZCWCCW, HIGH); // negative CCW = HIGH
} // end if
delay(10);
for (iCount = 0; iCount < iDeltaStepsAZ; iCount++)
{
digitalWrite(MOTAZCLK, HIGH); // STEP 1.8 DEG (with 36 reduction = 0.05 deg)
delay(StepDelayMoveAZ);
digitalWrite(MOTAZCLK, LOW);
delay(StepDelayMoveAZ);
}
dAngleAZ = utcSunCoordinates.dAzimuth;
} // end MoveMotorAZ()
void MoveMotorZE(int iDeltaStepsZE) {
int iCount;
//Serial.print("Moving Zenith motor this many steps: ");
//Serial.println(iDeltaStepsZE);
if (iDeltaStepsZE == 0) {
return;
} // end if
if (iDeltaStepsZE > 0) {
digitalWrite(MOTZECWCCW, HIGH); // positive CW = LOW
}
else {
iDeltaStepsZE = -iDeltaStepsZE;
digitalWrite(MOTZECWCCW, LOW); // negative CCW = HIGH
} // end if
delay(10);
for (iCount = 0; iCount < iDeltaStepsZE; iCount++)
{
digitalWrite(MOTZECLK, HIGH); // STEP 1.8 DEG (with 36 reduction = 0.05 deg)
delay(StepDelayMoveZE);
digitalWrite(MOTZECLK, LOW);
delay(StepDelayMoveZE);
} // end for
dAngleZE = utcSunCoordinates.dZenithAngle;
} // end MoveMotorZE()
void CurrentTime() {
int iBytesAvail; // number of bytes available from time-of-day clock
// read the time
Wire.beginTransmission(0x68);
Wire.write(0); // point to address of the timekeeping registers
Wire.endTransmission();
Wire.requestFrom(0x68, 7); // request 7 bytes from DS1307
iBytesAvail = Wire.available();
//Serial.print("Wire.available: ");
//Serial.println(iBytesAvail);
utcTime.dSeconds = convertHEX(Wire.read());
utcTime.dMinutes = convertHEX(Wire.read());
utcTime.dHours = convertHEX(Wire.read());
Wire.read(); // disregard the day of the week
utcTime.iDay = convertHEX(Wire.read());
utcTime.iMonth = convertHEX(Wire.read());
utcTime.iYear = 2000 + convertHEX(Wire.read());
Serial.println("");
Serial.println("Universal Coordinated Time");
Serial.print(" Time (HH:MM:SS): ");
if ((int)utcTime.dHours < 10) Serial.print("0");
Serial.print((int)utcTime.dHours, DEC);
Serial.print(":");
if ((int)utcTime.dMinutes < 10) Serial.print("0");
Serial.print((int)utcTime.dMinutes, DEC);
Serial.print(":");
if (utcTime.dSeconds < 10) Serial.print("0");
Serial.println((int)utcTime.dSeconds, DEC);
Serial.print(" Date (DD/MM/YYYY) ");
if (utcTime.iDay < 10) Serial.print("0");
Serial.print(utcTime.iDay, DEC);
Serial.print("/");
if (utcTime.iMonth < 10) Serial.print("0");
Serial.print(utcTime.iMonth, DEC);
Serial.print("/");
if (utcTime.iYear < 10) Serial.print("0");
Serial.println(utcTime.iYear, DEC);
} // end CurrentTime()
void SunPos(struct cTime utcTime, struct cLocation utcLocation, struct cSphereCoordinates *utcSunCoordinates)
{
// Main variables
double dElapsedJulianDays;
double dDecimalHours;
double dEclipticLongitude;
double dEclipticObliquity;
double dRightAscension;
double dDeclination;
// Auxiliary variables
double dY;
double dX;
// Calculate difference in days between the current Julian Day
// and JD 2451545.0, which is noon 1 January 2000 Universal Time
{
double dJulianDate;
long int liAux1;
long int liAux2;
// Calculate time of the day in UT decimal hours
dDecimalHours = utcTime.dHours + (utcTime.dMinutes
+ utcTime.dSeconds / 60.0 ) / 60.0;
// Calculate current Julian Day
liAux1 = (utcTime.iMonth - 14) / 12;
liAux2 = (1461 * (utcTime.iYear + 4800 + liAux1)) / 4 + (367 * (utcTime.iMonth
- 2 - 12 * liAux1)) / 12 - (3 * ((utcTime.iYear + 4900
+ liAux1) / 100)) / 4 + utcTime.iDay - 32075;
dJulianDate = (double)(liAux2) - 0.5 + dDecimalHours / 24.0;
// Calculate difference between current Julian Day and JD 2451545.0
dElapsedJulianDays = dJulianDate - 2451545.0;
}
// Calculate ecliptic coordinates (ecliptic longitude and obliquity of the
// ecliptic in radians but without limiting the angle to be less than 2*Pi
// (i.e., the result may be greater than 2*Pi)
{
double dMeanLongitude;
double dMeanAnomaly;
double dOmega;
dOmega = 2.1429 - 0.0010394594 * dElapsedJulianDays;
dMeanLongitude = 4.8950630 + 0.017202791698 * dElapsedJulianDays; // Radians
dMeanAnomaly = 6.2400600 + 0.0172019699 * dElapsedJulianDays;
dEclipticLongitude = dMeanLongitude + 0.03341607 * sin( dMeanAnomaly )
+ 0.00034894 * sin( 2 * dMeanAnomaly ) - 0.0001134
- 0.0000203 * sin(dOmega);
dEclipticObliquity = 0.4090928 - 6.2140e-9 * dElapsedJulianDays
+ 0.0000396 * cos(dOmega);
}
// Calculate celestial coordinates ( right ascension and declination ) in radians
// but without limiting the angle to be less than 2*Pi (i.e., the result may be
// greater than 2*Pi)
{
double dSin_EclipticLongitude;
dSin_EclipticLongitude = sin( dEclipticLongitude );
dY = cos( dEclipticObliquity ) * dSin_EclipticLongitude;
dX = cos( dEclipticLongitude );
dRightAscension = atan2( dY, dX );
if (dRightAscension < 0.0)
{
dRightAscension = dRightAscension + twoPI;
}
dDeclination = asin( sin( dEclipticObliquity ) * dSin_EclipticLongitude );
}
// Calculate local coordinates (azimuth and zenith angle) in degrees
{
double dGreenwichMeanSiderealTime;
double dLocalMeanSiderealTime;
double dLatitudeInRadians;
double dHourAngle;
double dCos_Latitude;
double dSin_Latitude;
double dCos_HourAngle;
double dParallax;
dGreenwichMeanSiderealTime = 6.6974243242 +
0.0657098283 * dElapsedJulianDays
+ dDecimalHours;
dLocalMeanSiderealTime = (dGreenwichMeanSiderealTime * 15
+ utcLocation.dLongitude) * rad;
dHourAngle = dLocalMeanSiderealTime - dRightAscension;
dLatitudeInRadians = utcLocation.dLatitude * rad;
dCos_Latitude = cos( dLatitudeInRadians );
dSin_Latitude = sin( dLatitudeInRadians );
dCos_HourAngle = cos( dHourAngle );
utcSunCoordinates->dZenithAngle = (acos( dCos_Latitude * dCos_HourAngle
* cos(dDeclination) + sin( dDeclination ) * dSin_Latitude));
dY = -sin( dHourAngle );
dX = tan( dDeclination ) * dCos_Latitude - dSin_Latitude * dCos_HourAngle;
utcSunCoordinates->dAzimuth = atan2( dY, dX );
if (utcSunCoordinates->dAzimuth < 0.0)
{
utcSunCoordinates->dAzimuth = utcSunCoordinates->dAzimuth + twoPI;
}
utcSunCoordinates->dAzimuth = utcSunCoordinates->dAzimuth / rad;
// Parallax Correction
dParallax = (dEarthRadius / dAstroUnit)
* sin(utcSunCoordinates->dZenithAngle);
utcSunCoordinates->dZenithAngle = (utcSunCoordinates->dZenithAngle
+ dParallax) / rad;
}
} // end GetSunPos()
