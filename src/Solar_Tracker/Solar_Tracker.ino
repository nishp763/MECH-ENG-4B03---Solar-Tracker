#include <Wire.h>
#define Azimuth_Motor_Direction 7 // LOW = CW , HIGH = CCW
#define Azimuth_Motor_Clock 6
#define Azimuth_Optical_Sensor 2 // HIGH = Triggered
#define Stepper_Motor_Delay 5
#define pi 3.1415926535897932384 
#define twoPI (2.0*pi)
#define rad (pi/180.0)
#define dEarthRadius 6371.01 // in km 
#define dAstroUnit 149597890 // in km 

//Use decimal format (Latitude = 43 + 26.0181/60 = 43.434; Longitude = -1 * (79 degrees + 92.0892/60) = -80.535;) 
const double MCMASTERLATITUDE = 43.434; 
const double MCMASTERLONGITUDE = -80.535; 
 
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

struct cSunCoordinates { 
 double dZenithAngle; 
 double dAzimuth; 
}; 
 
struct cTime utcTime; 
struct cLocation utcLocation; 
struct cSunCoordinates utcSunCoordinates;

void setup()
{
	Serial.begin(9600); // initialize the serial port
  Wire.begin(); // Initialize the transmission process

	/* Print message on the serial monitor */
	Serial.println("SolarTracker v4.4");
	Serial.println("Serial Connection Initalized");
	Serial.println("");

	/* Initialize all Sensors and Actuators for the Azimuth Stage */

	// Initialize Azimuth Stage Motors as OUTPUT
	pinMode(Azimuth_Motor_Direction, OUTPUT);
	pinMode(Azimuth_Motor_Clock, OUTPUT);

	// Initialize Azimuth Stage Opto Sensor as INPUT
	pinMode(Azimuth_Optical_Sensor, INPUT);
	home_azimuth();
  //setTime();
	//move_azimuth("CCW",180);
}

void loop() 
{
  getCurrentTime();
}

void home_azimuth() // This function will place the Azimuth stage to the home position
{
	Serial.println("Moving Azimuth Stage to the Home Position");
	for (int i = 0; i <= 180; i++)
	{
  	Serial.print("i > ");
  	Serial.print(i);
  	Serial.println("");
		if(digitalRead(Azimuth_Optical_Sensor) == HIGH) // Sensor Triggered
	  {
	  	Serial.println("Azimuth Stage = Home Position");
	  	return;
	  }
	  else
	  {
	  	move_azimuth("CW", 1); // Move the Azimuth Stage with a precision of 1 degree
	  }
	}
	Serial.println("ERROR: Aziumth Stage moved from 0 - 180 degrees CW, Home Position not reached. Try cleaning the optical sensor or perhaps go in CCW position.");
}

void move_azimuth(String motor_direction, int azimuth_move_degrees) // This function will send a signal to the stepper motor to move the azimuth stage
{
	/* Convert degrees to the # of steps */
	int azimuth_steps = azimuth_move_degrees/0.05;
	Serial.print("Azimuth Steps > ");
	Serial.print(azimuth_steps);
	Serial.println("");
	// Serial.println("Moving Azimuth Motor to " + azimuth_move_degrees + " degrees with " + azimuth_steps + " steps.");

	/* Set Direction PIN on Azimuth Stage */
	if (motor_direction == "CW")
	{
  	digitalWrite(Azimuth_Motor_Direction, LOW); // LOW = Clock Wise direction
  	Serial.println("Azimuth Motor Direction -> Clockwise");
	}
	else if (motor_direction == "CCW")
	{
  	digitalWrite(Azimuth_Motor_Direction, HIGH); // HIGH = Counter Clock Wise direction
  	Serial.println("Azimuth Motor Direction -> Counter Clockwise");
	}
	else
	{
  	Serial.println("ERROR: motor direction not specified properly...setting Azimuth to CW");
  	digitalWrite(Azimuth_Motor_Direction, LOW); // LOW = Clock Wise direction
	}

	/* Send Clock Signals to the Stepper Motor to move in the direction specified */
	for(int i = 0; i < azimuth_steps; i++)
	{
  	digitalWrite(Azimuth_Motor_Clock, HIGH);
  	delay(Stepper_Motor_Delay);
  	digitalWrite(Azimuth_Motor_Clock, LOW);
  	delay(Stepper_Motor_Delay);
	}
}

void setTime() 
{
  // set the time - below corresponds to Monday, August 19, 2013, 17:40 UTC (1:40pm DST in Hamilton)
  Serial.println("Set time ");
  Wire.beginTransmission(0x68);
  Wire.write(0); // point to address of the timekeeping registers
  Wire.write(0x00); // set seconds
  Wire.write(0x26); // set minutes
  Wire.write(0x80 | 0x18); // set hours 24 mode
  Wire.write(0x04); // day of week
  Wire.write(0x22); // date
  Wire.write(0x11); // month
  Wire.write(0x17); // year 00-00
  Wire.write(0x10); // provide 1 Hz square wave on pin 7
  Wire.endTransmission();
}

byte convertHEX(byte value) { 
 //This works for decimal 0-99 
 return ((value/16*10) + (value%16)); 
} // end convertHEX

void getCurrentTime() 
{
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

void GetSunPos(struct cTime utcTime, struct cLocation utcLocation, struct cSunCoordinates *utcSunCoordinates) 
{ 
	// Main variables 
	double dElapsedJulianDays; 
	double dDecimalHours; 
	double dEclipticLongitude; 
	double dEclipticObliquity; 
	double dRightAscension; 
	double dDeclination; 
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
	liAux1 =(utcTime.iMonth-14)/12; 
	liAux2=(1461*(utcTime.iYear + 4800 + liAux1))/4 + (367*(utcTime.iMonth 
	- 2-12*liAux1))/12- (3*((utcTime.iYear + 4900 
	+ liAux1)/100))/4+utcTime.iDay-32075; 
	dJulianDate=(double)(liAux2)-0.5+dDecimalHours/24.0; 
	// Calculate difference between current Julian Day and JD 2451545.0 
	dElapsedJulianDays = dJulianDate-2451545.0; 
	} 

	// Calculate ecliptic coordinates (ecliptic longitude and obliquity of the 
	// ecliptic in radians but without limiting the angle to be less than 2*Pi 
	// (i.e., the result may be greater than 2*Pi) 
	{ 
	double dMeanLongitude; 
	double dMeanAnomaly; 
	double dOmega; 
	dOmega=2.1429-0.0010394594*dElapsedJulianDays; 
	dMeanLongitude = 4.8950630+ 0.017202791698*dElapsedJulianDays; // Radians 
	dMeanAnomaly = 6.2400600+ 0.0172019699*dElapsedJulianDays; 
	dEclipticLongitude = dMeanLongitude + 0.03341607*sin( dMeanAnomaly ) 
	+ 0.00034894*sin( 2*dMeanAnomaly )-0.0001134 
	-0.0000203*sin(dOmega); 
	dEclipticObliquity = 0.4090928 - 6.2140e-9*dElapsedJulianDays 
	+0.0000396*cos(dOmega); 
	} 

	// Calculate celestial coordinates ( right ascension and declination ) in radians 
	// but without limiting the angle to be less than 2*Pi (i.e., the result may be 
	// greater than 2*Pi) 
	{ 
	double dSin_EclipticLongitude; 
	dSin_EclipticLongitude= sin( dEclipticLongitude ); 
	dY = cos( dEclipticObliquity ) * dSin_EclipticLongitude; 
	dX = cos( dEclipticLongitude ); 
	dRightAscension = atan2( dY,dX ); 
	if( dRightAscension < 0.0 ) dRightAscension = dRightAscension + twoPI; 
	dDeclination = asin( sin( dEclipticObliquity )*dSin_EclipticLongitude ); 
	} 

	// Calculate local coordinates ( azimuth and zenith angle ) in degrees 
	{ 
	double dGreenwichMeanSiderealTime; 
	double dLocalMeanSiderealTime;
	double dHourAngle; 
	double dCos_Latitude; 
	double dSin_Latitude; 
	double dCos_HourAngle; 
  double dLatitudeInRadians;
	double dParallax; 
	dGreenwichMeanSiderealTime = 6.6974243242 + 
	0.0657098283*dElapsedJulianDays 
	+ dDecimalHours; 
	dLocalMeanSiderealTime = (dGreenwichMeanSiderealTime*15 
	+ utcLocation.dLongitude)*rad; 
	dHourAngle = dLocalMeanSiderealTime - dRightAscension; 
	dLatitudeInRadians = utcLocation.dLatitude*rad; 
	dCos_Latitude = cos( dLatitudeInRadians ); 
	dSin_Latitude = sin( dLatitudeInRadians ); 
	dCos_HourAngle= cos( dHourAngle ); 
	utcSunCoordinates->dZenithAngle = (acos( dCos_Latitude*dCos_HourAngle 
	*cos(dDeclination) + sin( dDeclination )*dSin_Latitude)); 
	dY = -sin( dHourAngle ); 
	dX = tan( dDeclination )*dCos_Latitude - dSin_Latitude*dCos_HourAngle; 
	utcSunCoordinates->dAzimuth = atan2( dY, dX ); 
	if ( utcSunCoordinates->dAzimuth < 0.0 ) 
	utcSunCoordinates->dAzimuth = utcSunCoordinates->dAzimuth + twoPI; 
	utcSunCoordinates->dAzimuth = utcSunCoordinates->dAzimuth/rad; 
	// Parallax Correction 
	dParallax=(dEarthRadius/dAstroUnit) 
	*sin(utcSunCoordinates->dZenithAngle); 
	utcSunCoordinates->dZenithAngle= (utcSunCoordinates->dZenithAngle 
	+ dParallax)/rad; 
	} 
} // end GetSunPos() 
