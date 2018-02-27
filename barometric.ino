/******************************************************************************

******************************************************************************/

/*** Includes ****/
#include <Wire.h>
#include <SparkFun_MS5803_I2C.h>

/*** Defines ****/
#define TCAADDR 0x70          // Multiplexer address
#define NUMBEROFSENSORS 8     // Number of barometric sensors in use
#define MBARTOPSI 0.0145038   // Conversion constant from mbar to psi
 
/*** Globals ***/
MS5803 Bar[NUMBEROFSENSORS];              // Array of barometric sensor IDs
float pocketarea[NUMBEROFSENSORS] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
float temperature_c, temperature_f;
double pressure_mbar, pressure_relative, pressure_psi
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

  for(int i = 0; i < NUMBEROFSENSORS; i++)
  {
    /* Assign addresses to barometric sensors*/
    Bar[i] = MS5803(ADDRESS_HIGH);

    /*//Alternative, should we decide to assign two sensors to one port on the mux.
    if(i%2 == 0)
    {
      Bar[i] = MS5803(ADDRESS_HIGH);
    }
    else
    {
      Bar[i] = MS5803(ADDRESS_LOW);
    }*/

    /* Initialize sensors */
    tcaselect(i);
    Bar[i].reset();
    int error = Bar[i].begin();
    Serial.print("Sensor ");
    Serial.print(i);
    Serial.print(" error code: ");
    Serial.println(error);
    if(error)
    {
      /* There was a problem detecting the MS5803 ... check your connections */
      Serial.print("Error detected on port ");
      Serial.print(i);
      Serial.println(" ... Check your wiring!");
      while(1);
    }
    else
    {
      pressure_baseline = Bar1.getPressure(ADC_4096);
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

  for(int i = 0; i < NUMBEROFSENSORS; i++)
  {
    /* Select sensor */
    tcaselect(i);
    
    /* Read temperature from the sensor in deg C. This operation takes about */
    temperature_c = Bar1.getTemperature(CELSIUS, ADC_512);
    
    /* Read temperature from the sensor in deg F. Converting
     * to Fahrenheit is not internal to the sensor.
     * Additional math is done to convert a Celsius reading.
     */
     temperature_f = Bar1.getTemperature(FAHRENHEIT, ADC_512);
     
     /* Read pressure from the sensor in mbar. */
     pressure_mbar = Bar1.getPressure(ADC_4096);
   
     /* Convert pressure from mbar to psi. */
     pressure_psi = pressure_mbar * MBARTOPSI;
   
     /* Convert pressure to force in lbs. */
     force_lbs = pressure_psi * pocketarea[i];
   
     /* Convert force from lbs to N. */
     //force_N = force_lbs * something;
     
     /* Print sensor pressure */
     Serial.print("Pressure (mbar)= ");
     Serial.println(pressure_mbar);
     
     Serial.print("Pressure (psi)= ");
     Serial.println(pressure_psi);
   
     /* Print force */
     Serial.print("Force (lbs)= ");
     Serial.println(force_lbs);
     
     /* Delay between sensor readings. */
     delay(100);
  }
   
  delay(1000);
}
