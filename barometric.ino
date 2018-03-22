/******************************************************************************

******************************************************************************/

/*** Includes ****/
#include <Wire.h>
#include <SparkFun_MS5803_I2C.h>
extern "C" { 
#include "utility/twi.h"  // from Wire library, so we can do bus scanning
}

/*** Defines ****/
#define START 0     //Testing
#define STOP 3      //Testing
#define TCAADDR 0x70          // Multiplexer address
#define NUMBEROFPORTS 4       // Number of mux ports in use
#define NUMBEROFSENSORS 8     // Number of barometric sensors in use
#define MBARTOPSI 0.0145038   // Conversion constant from mbar to psi
#define LBSTON 4.4482         // Conversion constant from lbs to N
 
/*** Globals ***/
MS5803 Bar[NUMBEROFSENSORS] = {MS5803(ADDRESS_HIGH), MS5803(ADDRESS_LOW),
                                MS5803(ADDRESS_HIGH), MS5803(ADDRESS_LOW),
                                MS5803(ADDRESS_HIGH), MS5803(ADDRESS_LOW),
                                MS5803(ADDRESS_HIGH), MS5803(ADDRESS_LOW)};
MS5803 Bar0 = MS5803(ADDRESS_LOW); // Just in case the array doesn't work
MS5803 Bar1 = MS5803(ADDRESS_HIGH);  // "
MS5803 Bar2 = MS5803(ADDRESS_LOW); // "
MS5803 Bar3 = MS5803(ADDRESS_HIGH);  // "
MS5803 Bar4 = MS5803(ADDRESS_LOW); // "
MS5803 Bar5 = MS5803(ADDRESS_HIGH);  // "
MS5803 Bar6 = MS5803(ADDRESS_LOW); // "
MS5803 Bar7 = MS5803(ADDRESS_HIGH);  // "

float pocketarea[NUMBEROFSENSORS] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
float temperature_c, temperature_f;
double pressure_mbar, pressure_relative, pressure_psi;
double altitude_delta, pressure_baseline;
double force_lbs, force_N;
double base_altitude = 1655.0;            // Altitude of SparkFun's HQ in Boulder, CO. in (m)

/* Adafruit multiplexer helper method */
void tcaselect(uint8_t i) {
  if (i > 7) return;
 
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();  
}

/* Sketch setup method */
void setup() {
  Serial.begin(9600);
  
  /* Retrieve calibration constants for conversion math. */
  Serial.println("\nMS5803 Barometric Test...start\n"); 

  for(int port = START; port < STOP; port++)
  {
    int sensor = port*2;
    /* Initialize sensors */
    while (sensor <= port*2+1)
    {
      tcaselect(port);
      Bar[sensor].reset();
      int error = Bar[sensor].begin();
      Serial.print("Port ");
      Serial.print(port);
      Serial.print(" sensor ");
      Serial.print(sensor);
      Serial.print(" error code: ");
      Serial.println(error);
      if(error)
      {
        /* There was a problem detecting the MS5803 ... check your connections */
        Serial.print("Error detected...check your wiring!");
        while(1);
      }
      else
      {
        pressure_baseline = Bar[sensor].getPressure(ADC_4096);
      }
      sensor++;
    }
  }
}

/* Sketch main loop */
void loop() {
  
  /* To measure to higher degrees of precision use the following sensor settings:
   * ADC_256 
   * ADC_512 
   * ADC_1024
   * ADC_2048
   * ADC_4096
   */

  for(int port = START; port < STOP; port++)
  {
    int sensor = port*2;
    /* Initialize sensors */
    while (sensor <= port*2+1)
    {
      /* Select sensor */
    tcaselect(port);
    
    /* Read temperature from the sensor in deg C. This operation takes about */
    temperature_c = Bar[sensor].getTemperature(CELSIUS, ADC_512);
    
    /* Read temperature from the sensor in deg F. Converting
     * to Fahrenheit is not internal to the sensor.
     * Additional math is done to convert a Celsius reading.
     */
     temperature_f = Bar[sensor].getTemperature(FAHRENHEIT, ADC_512);
     
     /* Read pressure from the sensor in mbar. */
     pressure_mbar = Bar[sensor].getPressure(ADC_4096);
   
     /* Convert pressure from mbar to psi. */
     pressure_psi = pressure_mbar * MBARTOPSI;
   
     /* Convert pressure to force in lbs. */
     force_lbs = pressure_psi * pocketarea[sensor];
   
     /* Convert force from lbs to N. */
     force_N = force_lbs * LBSTON;
     
//     /* Print sensor pressure */
//     Serial.print("Pressure (mbar)= ");
//     Serial.println(pressure_mbar);
//     
//     Serial.print("Pressure (psi)= ");
//     Serial.println(pressure_psi);
//   
//     /* Print force */
//     Serial.print("Force (lbs)= ");
//     Serial.println(force_lbs);
//
//     Serial.print("Force (N)= ");
//     Serial.println(force_N);
//     
     Serial.print(pressure_mbar);
     if (sensor%2 == 0) Serial.print(", ");
     else Serial.println(" ");
     sensor++;
    }
  }
  delay(1000);
}
