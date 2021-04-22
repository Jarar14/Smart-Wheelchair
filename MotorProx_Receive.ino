//Libraries
#include <SPI.h>
#include <L298N.h>
#include "RF24.h"

// Structure for x and y coords
struct package{
  int inputY;
  int inputX; };

// initialize temp structure
typedef struct package Package;
Package data;

//Motor Pins
const unsigned int motor1_IN1 = 30;
const unsigned int motor1_IN2 = 32;
const unsigned int motor1_EN = 9;
const unsigned int motor2_IN1 = 22;
const unsigned int motor2_IN2 = 24;
const unsigned int motor2_EN = 4;

//Motor Instances
L298N motor1(motor1_EN, motor1_IN1, motor1_IN2);
L298N motor2(motor2_EN, motor2_IN1, motor2_IN2);

//NRF Radio Pins + Connectivity Channels
RF24 myRadio (7, 8);
byte addresses[][6] = {"0"}; // nrf radio connectivity channels

//HC-SRO4 Proximity Sensors + LED
int trigPinF = 11;    // Trigger
int echoPinF = 12;    // Echo
int trigPinB = 2;    // Trigger
int echoPinB = 3;    // Echo
long duration, cm, inches;
int led = 13; // the pin the LED is connected to

//Input Pair from Gyroscope Transmitter Module
int pair[2];

void setup()
{
  // Used to display information
  Serial.begin(9600);
  delay(1000);

  // Wait for Serial to initialize
  while (!Serial)
  {
    Serial.println("Serial didnt work");
  }

  // while radio is not initialized
  while (!myRadio.begin()) {
    Serial.println("Radio not working");
    myRadio.begin();
    delay(500);
  }

  // set radio channels and pipes
  myRadio.setChannel(115);
  myRadio.setPALevel(RF24_PA_MAX);
  myRadio.setDataRate( RF24_250KBPS ) ;
  myRadio.openReadingPipe(1, addresses[0]);

  // Set initial motor speeds
  motor1.setSpeed(200);  //200 For Balanced Rotation
  motor2.setSpeed(250);  //250

  // open listening phase for radio
  myRadio.startListening();
  Serial.println("Ready for listening to input...");

 
  //Define HC-SR04 inputs and outputs
  pinMode(trigPinF, OUTPUT);
  pinMode(echoPinF, INPUT);
  pinMode(trigPinB, OUTPUT);
  pinMode(echoPinB, INPUT);

  //LED Output for testing Prox
  pinMode(led, OUTPUT); // Declare the LED as an output

}


void loop()
{
  // if radio is available and something is recieved
  if ( myRadio.available())
  {
    while (myRadio.available())
    {
      // read the input recieved and acknowledge
      myRadio.read( &pair, sizeof(pair) );
    }
    //Serial.print("Input received: ");
    //Serial.println(pair[0]);
    //Serial.print(", ");
    //Serial.println(pair[1]);
    //Serial.print("\n\n");
  }

  //No Movement
  if ((pair[0] < 30 &&  pair[0] > -30) && (pair[1] < 30 && pair[1] > -30)) { 
    // Tell the motor to go forward (may depend by your wiring)
    motor1.stop();
    motor2.stop(); 
    Serial.print("Motor Stopped /n");
    digitalWrite(led, HIGH); // Turn the LED on
    delay(150);
    digitalWrite(led, LOW); // Turn the LED on
    delay(150);
  }

  //STRAIGHT FORWARD
  else if ((pair[0] > 30) && (pair[1] > -30 && pair[1] < 30)) { 
    // Tell the motor to go forward (may depend by your wiring)
    int dist = distStopF();
    if ( (dist < 15) && (dist > 0) ){
      stopMotor();
      } 
    else {
      motor1.forward();
      motor2.backward(); }
  }

  //STRAIGHT BACKWARD
  else if ((pair[0] < -30) && (pair[1] > -30 && pair[1] < 30)) {
    // Tell the motor to go back (may depend by your wiring)
    int dist = distStopB();
    if ( (dist < 15) && (dist > 0) ){
      stopMotor();
      } 
    else {
      motor1.backward();
      motor2.forward(); }
  }

  //FORWARD RIGHT
  else if ((pair[1] > 30) && (pair[0] > 30)) {
    // Tell the motor to go right (may depend by your wiring)
    int dist = distStopF();
    if ( (dist < 15) && (dist > 0) ){
      stopMotor();
      } 
    else {
      motor1.stop();
      motor2.backward(); }
  }

  // FORWARD LEFT
  else if ((pair[1] < -30) && (pair[0] > 30)) {
    // Tell the motor to go left (may depend by your wiring)
    int dist = distStopF();
    if ( (dist < 15) && (dist > 0) ){
      stopMotor();
      } 
    else {
      motor1.forward();
      motor2.stop(); }
  }
  
  //BACKWARD RIGHT
  else if ((pair[1] > 30) && (pair[0] < -30)) {
    // Tell the motor to go right (may depend by your wiring)
    int dist = distStopB();
    if ( (dist < 15) && (dist > 0) ){
      stopMotor();
      } 
    else {
      motor1.stop();
      motor2.forward(); }
  }

  // BACKWARD LEFT
  else if ((pair[1] < -30) && (pair[0] < -30)) {
    // Tell the motor to go left (may depend by your wiring)
    int dist = distStopB();
    if ( (dist < 15) && (dist > 0) ){
      stopMotor();
      } 
    else {
      motor1.backward();
      motor2.stop(); }
  }

  //print the motor status in the serial monitor
  printRadioInfo();

} //end void loop

//Stops all motors, and blinks an LED
void stopMotor() {
  motor1.stop();
  motor2.stop(); 
    
  Serial.println("Obstacle in Proximity");

  digitalWrite(led, HIGH); // Turn the LED on
  delay(150);
  digitalWrite(led, LOW); // Turn the LED on  
  delay(150);
}

// For Prox Sensor Distance Computation
long average(long duration[10]) {
  int sum = 0; // initialize sum  
  
    // Iterate through all elements  
    // and add them to sum  
    for (int i = 0; i < 10; i++)  
    sum += duration[i];  
  
    return sum/10;  
}

//Outputs distance in-front of vehicle
int distStopF() 
{
  long duration[10]; 
  for (int i = 0; i < 10; i++) {
    // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
    // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
    digitalWrite(trigPinF, LOW);
    delayMicroseconds(5);
    digitalWrite(trigPinF, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPinF, LOW);
   
    // Read the signal from the sensor: a HIGH pulse whose
    // duration is the time (in microseconds) from the sending
    // of the ping to the reception of its echo off of an object.
    pinMode(echoPinF, INPUT);
    duration[i] = pulseIn(echoPinF, HIGH);
  }

  long avgdur = average(duration);
  
  // Convert the time into a distance
  cm = (avgdur/2) / 29.1;     // Divide by 29.1 or multiply by 0.0343
  //inches = (avgdur/2) / 74;   // Divide by 74 or multiply by 0.0135

  Serial.print(cm);
  Serial.print("cm forward");
  Serial.println();

  //delay(250);
  return cm;
}

//Outputs distance behind vehicle
int distStopB() 
{
  long duration[10]; 
  
  for (int i = 0; i < 10; i++) {
    // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
    // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
    digitalWrite(trigPinB, LOW);
    delayMicroseconds(5);
    digitalWrite(trigPinB, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPinB, LOW);
   
    // Read the signal from the sensor: a HIGH pulse whose
    // duration is the time (in microseconds) from the sending
    // of the ping to the reception of its echo off of an object.
    pinMode(echoPinB, INPUT);
    duration[i] = pulseIn(echoPinB, HIGH);
  }

  long avgdur = average(duration);
  
  // Convert the time into a distance
  cm = (avgdur/2) / 29.1;     // Divide by 29.1 or multiply by 0.0343
  //inches = (avgdur/2) / 74;   // Divide by 74 or multiply by 0.0135

  Serial.print(cm);
  Serial.print("cm backward");
  Serial.println();

  //delay(250);
  return cm;
  }
  
// print off radio input recieved for motors
void printRadioInfo()
{
  Serial.print("Motor1 is moving = ");
  Serial.print(motor1.isMoving());
  //Serial.print(" at speed = ");
  //Serial.println(motor1.getSpeed());

  Serial.print("/nMotor2 is moving = ");
  Serial.print(motor2.isMoving());
  //Serial.print(" at speed = ");
  //Serial.println(motor2.getSpeed());
}
