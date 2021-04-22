// Include the libraries
#include <SPI.h>
#include "RF24.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;

// nrf radio pins
RF24 myRadio (7, 8);
// nrf radio connectivity channels
byte addresses[][6] = {"0"};

// boolean flag to check for new input availability
boolean newData = false;

int pair[2];

// Structure for x and y coords
struct package
{
  int inputY;
  int inputX; 
};

// initialize temp structure
typedef struct package Package;
Package data;

void setup()
{
  // Used to display information
  Serial.begin(9600);
  delay(1000);

  while (!Serial){
    Serial.println("Serial failed");
  }
  
  //MPU
  // Try to initialize!
  while (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    delay(10);
  }
  
  Serial.println("MPU6050 connected!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
    case MPU6050_RANGE_2_G:
      Serial.println("+-2G");
      break;
    case MPU6050_RANGE_4_G:
      Serial.println("+-4G");
      break;
    case MPU6050_RANGE_8_G:
      Serial.println("+-8G");
      break;
    case MPU6050_RANGE_16_G:
      Serial.println("+-16G");
      break;
  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
    case MPU6050_RANGE_250_DEG:
      Serial.println("+- 250 deg/s");
      break;
    case MPU6050_RANGE_500_DEG:
      Serial.println("+- 500 deg/s");
      break;
    case MPU6050_RANGE_1000_DEG:
      Serial.println("+- 1000 deg/s");
      break;
    case MPU6050_RANGE_2000_DEG:
      Serial.println("+- 2000 deg/s");
      break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
    case MPU6050_BAND_260_HZ:
      Serial.println("260 Hz");
      break;
    case MPU6050_BAND_184_HZ:
      Serial.println("184 Hz");
      break;
    case MPU6050_BAND_94_HZ:
      Serial.println("94 Hz");
      break;
    case MPU6050_BAND_44_HZ:
      Serial.println("44 Hz");
      break;
    case MPU6050_BAND_21_HZ:
      Serial.println("21 Hz");
      break;
    case MPU6050_BAND_10_HZ:
      Serial.println("10 Hz");
      break;
    case MPU6050_BAND_5_HZ:
      Serial.println("5 Hz");
      break;
  }


  
  // while radio is not initialized
  while (!myRadio.begin()) {
    Serial.println("Radio not working");
    myRadio.begin();
  }

  // set radio channels and pipes
  myRadio.setChannel(115);
  myRadio.setPALevel(RF24_PA_MAX);
  myRadio.setDataRate( RF24_250KBPS ) ;
  myRadio.openWritingPipe( addresses[0]);

  // ready for input
  Serial.println("Ready to initiialize input...");
  delay(1000);
} //end void setup


void loop()
{
  /* Get new sensor events with the readings */
  grabMPUData();

  /*// check for serial input in serial monitor
  Serial.println("Enter command >>> ");
  if (Serial.available() && newData == false) {
    //waitForInput();
  }*/

  // if recieved new serial input
  if (newData == true) {
    sendDataToRadio();
    Serial.println("Ready for next input");
    newData = false;
  }

  delay(500);

}


void grabMPUData() {
  //Set Input
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  pair[0] = -9 * a.acceleration.y;
  pair[1] = 9 * a.acceleration.x;

  /* Print out the values */
  Serial.print("Left-Right : ");
  Serial.print(pair[1]);
  Serial.print(", Forward-Backward : ");
  Serial.print(pair[0]);
  Serial.println("");
  newData = true;
  
}


// send input through the radio
void sendDataToRadio() {
  Serial.print("InputX: ");
  Serial.println(pair[1]);
  Serial.print("InputY: ");
  Serial.println(pair[0]);

  myRadio.write(&pair, sizeof(pair));

  Serial.print("Sending input: ");
  Serial.print(pair[1]);
  Serial.print(", ");
  Serial.print(pair[0]);
  Serial.print("\n\n");
}
