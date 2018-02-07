/******************************************************************************
SparkFun_MS5803_Demo.ino
Demo Program for MS5803 pressure sensors.
Casey Kuhns @ SparkFun Electronics
7/20/2014
https://github.com/sparkfun/MS5803-14BA_Breakout/
The MS58XX MS57XX and MS56XX by Measurement Specialties is a low cost I2C pressure
sensor.  This sensor can be used in weather stations and for altitude
estimations. It can also be used underwater for water depth measurements. 
Resources:
This library uses the Arduino Wire.h to complete I2C transactions.
Development environment specifics:
  IDE: Arduino 1.0.5
  Hardware Platform: Arduino Pro 3.3V/8MHz
  T5403 Breakout Version: 1.0
**Updated for Arduino 1.6.4 5/2015**
  
This code is beerware. If you see me (or any other SparkFun employee) at the
local pub, and you've found our code helpful, please buy us a round!
Distributed as-is; no warranty is given.
******************************************************************************/
#include <Wire.h>
#include <SparkFun_MS5803_I2C.h>

#define TCAADDR 0x70

// Begin class with selected address
// available addresses (selected by jumper on board) 
// default is ADDRESS_HIGH
// ADDRESS_HIGH = 0x76
// ADDRESS_LOW  = 0x77
  
/* Assign a unique ID to this sensor at the same time */
MS5803 Bar1 = MS5803(ADDRESS_HIGH);
MS5803 Bar2 = MS5803(ADDRESS_HIGH);

//Create variables to store results
float temperature_c, temperature_f;
double pressure_abs, pressure_relative, altitude_delta, pressure_baseline;

// Create Variable to store altitude in (m) for calculations;
double base_altitude = 1655.0; // Altitude of SparkFun's HQ in Boulder, CO. in (m)

// Adafruit multiplexer helper code
void tcaselect(uint8_t i) {
  if (i > 7) return;
 
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();  
}

// Sketch setup method
void setup() {
  Serial.begin(9600);
  
  //Retrieve calibration constants for conversion math.
  Serial.println("\nMS5803 Barometric Test...start\n"); 
  
  /* Initialize the 1st sensor */
  tcaselect(2);
  Bar1.reset();
  int error = Bar1.begin();
  Serial.print("Bar1 error code: ");
  Serial.println(error);
  if(error)
  {
    /* There was a problem detecting the MS5803 ... check your connections */
    Serial.println("(1) No MS5803 detected ... Check your wiring!");
    while(1);
  }
  else
  {
    pressure_baseline = Bar1.getPressure(ADC_4096);
  }
  
  
  /* Initialize the 2nd sensor */
  tcaselect(7);
  Bar2.reset();
  error = Bar2.begin();
  Serial.print("Bar2 error code: ");
  Serial.println(error);
  if(error)
  {
    /* There was a problem detecting the MS5803 ... check your connections */
    Serial.println("(2) No MS5803 detected ... Check your wiring!");
    while(1);
  }
  else
  {
    pressure_baseline = Bar2.getPressure(ADC_4096);
  }
}

// Sketch main loop
void loop() {
  
  // To measure to higher degrees of precision use the following sensor settings:
  // ADC_256 
  // ADC_512 
  // ADC_1024
  // ADC_2048
  // ADC_4096

  // Select sensor
  tcaselect(2);
  
  // Read temperature from the sensor in deg C. This operation takes about 
  temperature_c = Bar1.getTemperature(CELSIUS, ADC_512);
  
  // Read temperature from the sensor in deg F. Converting
  // to Fahrenheit is not internal to the sensor.
  // Additional math is done to convert a Celsius reading.
  temperature_f = Bar1.getTemperature(FAHRENHEIT, ADC_512);
  
  // Read pressure from the sensor in mbar.
  pressure_abs = Bar1.getPressure(ADC_4096);

  // Print sensor pressure
  Serial.print("Pressure abs (mbar)= ");
  Serial.println(pressure_abs);

  delay(100);
  
  // Select sensor
  tcaselect(7);
  
  // Read temperature from the sensor in deg C. This operation takes about 
  temperature_c = Bar2.getTemperature(CELSIUS, ADC_512);
  
  // Read temperature from the sensor in deg F. Converting
  // to Fahrenheit is not internal to the sensor.
  // Additional math is done to convert a Celsius reading.
  temperature_f = Bar2.getTemperature(FAHRENHEIT, ADC_512);
  
  // Read pressure from the sensor in mbar.
  pressure_abs = Bar2.getPressure(ADC_4096);

  // Print sensor pressure
  Serial.print("Pressure abs (mbar)= ");
  Serial.println(pressure_abs);
   
  delay(1000);
}

