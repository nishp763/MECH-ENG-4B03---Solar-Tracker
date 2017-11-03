#define Azimuth_Motor_Direction 7 // LOW = CW , HIGH = CCW
#define Azimuth_Motor_Clock 6
#define Azimuth_Optical_Sensor 2 // HIGH = Triggered
#define Stepper_Motor_Delay 5



void setup()
{
  Serial.begin(9600); // initialize the serial port

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
}

void loop() 
{
  // put your main code here, to run repeatedly:

}

void home_azimuth() // This function will place the Azimuth stage to the home position
{
  Serial.println("Moving Azimuth Stage to the Home Position");
  for (int i = 0; i <= 180; i++)
  {
  	if(digitalRead(Azimuth_Optical_Sensor) == HIGH) // Sensor Triggered
	  {
	  	Serial.println("Azimuth Stage = Home Position");
	  	return;
	  }
	  else
	  {
	  	move_azimuth("CW", 10); // Move the Azimuth Stage
	  }
  }
  Serial.println("ERROR: Aziumth Stage moved from 0 - 180 degrees CW, Home Position not reached. Try cleaning the optical sensor or perhaps go in CCW position.");
}

void move_azimuth(String motor_direction, int steps) // This function will send a signal to the stepper motor to move the azimuth stage
{
	Serial.println("Stepping Azimuth Motor...");

  /* Set Direction PIN on Azimuth Stage */
  if (motor_direction == "CW")
  {
    digitalWrite(Azimuth_Motor_Direction, LOW); // LOW = Clock Wise direction
  }
  else if (motor_direction == "CCW")
  {
    digitalWrite(Azimuth_Motor_Direction, HIGH); // HIGH = Counter Clock Wise direction
  }
  else
  {
    Serial.println("ERROR: motor direction not specified properly...setting Azimuth to CW");
    digitalWrite(Azimuth_Motor_Direction, LOW); // LOW = Clock Wise direction
  }

  /* Send Clock Signals to the Stepper Motor to move in the direction specified */
  for(int i = 0; i < steps; i++)
  {
    digitalWrite(Azimuth_Motor_Clock, HIGH);
    delay(Stepper_Motor_Delay);
    digitalWrite(Azimuth_Motor_Clock, LOW);
    delay(Stepper_Motor_Delay);
  }
}
