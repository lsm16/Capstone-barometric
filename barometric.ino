/******************************************************************************

******************************************************************************/

/*** Includes ****/
#include <Wire.h>
#include <SparkFun_MS5803_I2C.h>
extern "C" { 
#include "utility/twi.h"  // from Wire library, so we can do bus scanning
}

/*** Defines ****/
#define START 3     
#define STOP 0      
#define TCAADDR 0x70          // Multiplexer address
#define NUMBEROFPORTS 4       // Number of mux ports in use
#define NUMBEROFSENSORS 8     // Number of barometric sensors in use
#define MBARTOPSI 0.0145038   // Conversion constant from mbar to psi
#define LBSTON 4.4482         // Conversion constant from lbs to N
#define MBARTONPSQM 100       // Conversion constant from mbar to N/sqm
#define SQCMTOSQM 0.0001      // Conversion constant from square cm to square m
#define SQCMTOSQIN 0.155      // Conversion constant from square cm to square in
#define CUCMTOCUIN 0.0610237  // Conversion constat from cubic cm to cubic in
 
/*** Globals ***/
MS5803 Bar[NUMBEROFSENSORS] = {MS5803(ADDRESS_HIGH), MS5803(ADDRESS_LOW),
                                MS5803(ADDRESS_HIGH), MS5803(ADDRESS_LOW),
                                MS5803(ADDRESS_HIGH), MS5803(ADDRESS_LOW),
                                MS5803(ADDRESS_HIGH), MS5803(ADDRESS_LOW)};
int centroidX[NUMBEROFSENSORS] = {1, 1, -1, -1, -1, -1, 1, 1};
int centroidY[NUMBEROFSENSORS] = {-1, -2, -2, -1, 1, 2, 2, 1};
float area_sqin[NUMBEROFSENSORS] = {0.0, 0.0, 0.0, 0.0, 
                                    0.0, 0.0, 0.0, 0.0};
float area_sqcm[NUMBEROFSENSORS] = {35.81, 31.57, 31.24, 32.55, 
                                    35.86, 34.59, 35.71, 38.92};
float volume_cuin[NUMBEROFSENSORS] = {0.0, 0.0, 0.0, 0.0,
                                      0.0, 0.0, 0.0, 0.0};
float volume_cucm[NUMBEROFSENSORS] = {14.50, 12.11, 11.96, 12.90,
                                      14.52, 13.41, 13.98, 16.05};
float temperature[NUMBEROFSENSORS] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
float baseline[NUMBEROFSENSORS] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
float mbar[NUMBEROFSENSORS] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
float psi[NUMBEROFSENSORS] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
float lbs[NUMBEROFSENSORS] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
float npsqm[NUMBEROFSENSORS] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
float newtons[NUMBEROFSENSORS] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
float magnitudeLBS = 0.0;
float magnitudeN = 0.0;
float centroidXsumLBS = 0.0;
float centroidYsumLBS = 0.0;
float centroidXsumN = 0.0;
float centroidYsumN = 0.0;
float centroidLBS[2] = {0.0, 0.0};
float centroidN[2] = {0.0, 0.0};

/* Adafruit multiplexer helper method */
void tcaselect(uint8_t i) {
  if (i > 7) return;
 
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();  
}

/* Method to check for sensor connection errors */
int sensorconnectivity(MS5803 sensor, int port) {
  int reps = 5;
  double difference = 0.0;
  double pressureB = 0.0;

  tcaselect(port);
  double pressureA = sensor.getPressure(ADC_4096);
  
  for (int i = reps; i>0; i--)
  {
    tcaselect(port);
    pressureB = sensor.getPressure(ADC_4096);
    difference = pressureA - pressureB;
    if (difference != 0)
    {
      return 0;
    }
    else
    {
      pressureA = pressureB;
    }
  }
  return 1;
}

/* Sketch setup method */
void setup() {
  unsigned long a, b;
  a = millis();
   
  /* Set baud rate and pin modes */
  Serial.begin(9600);
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);

  /* Convert area and volume from Metric to English */
  for(int i = 0; i<NUMBEROFSENSORS; i++)
  {
    area_sqin[i] = area_sqcm[i] * SQCMTOSQIN;
    volume_cuin[i] = volume_cucm[i] * CUCMTOCUIN;
  }
  
  /* Retrieve calibration constants for conversion math. */
  Serial.println("\nMS5803 Barometric Test...start\n"); 

  for(int port = START; port >= STOP; port--)
  {
    int sensor = port*2+1;
    /* Initialize sensors */
    while (sensor >= port*2)
    {
      tcaselect(port);
      Bar[sensor].reset();
      int error = Bar[sensor].begin();
      //error = sensorconnectivity(Bar[sensor], port);
      if(error)
      {
        /* There was a problem detecting the MS5803 ... check your connections */
        Serial.print("\nError detected on port ");
        Serial.print(port);
        Serial.print(" sensor ");
        Serial.print(sensor);
        Serial.println("...check your wiring!");
        while(1);
      }
      
      baseline[sensor] = Bar[sensor].getPressure(ADC_4096);
      sensor--;
    }
  }

  b = millis() - a;

  Serial.print("Setup Time: ");
  Serial.print(b);
  Serial.println("\n");
}

/* Sketch main loop */
void loop() {
  unsigned long a, b; 
  a = millis();
  
  /* Reset magnitude to 0 at begining of main loop */
  magnitudeLBS = 0.0;
  magnitudeN = 0.0;

  /* Reset centroid calculation variables */
  centroidXsumLBS = 0.0;
  centroidYsumLBS = 0.0;
  centroidXsumN = 0.0;
  centroidYsumN = 0.0;

  /* Loop through all ports and sensors */
  for(int port = START; port >= STOP; port--)
  {
    int sensor = port*2+1;
    /* Initialize sensors */
    while (sensor >= port*2)
    {
      /* Sensor connectivity check */
      int error = 0;//sensorconnectivity(Bar[sensor], port);
      if(error)
      {
        /* There was a problem detecting the MS5803 ... check your connections */
        Serial.print("\nError detected on port ");
        Serial.print(port);
        Serial.print(" sensor ");
        Serial.print(sensor);
        Serial.println("...check your wiring!");
        while(1);
      }
      
      /* Select sensor */
    tcaselect(port);
    
    /* Read temperature from the sensor in deg C. This operation takes about */
    temperature[sensor] = Bar[sensor].getTemperature(CELSIUS, ADC_512);
         
     /* Read pressure from the sensor in mbar. */
     mbar[sensor] = Bar[sensor].getPressure(ADC_4096);
     //mbar[sensor] = Bar[sensor].getPressure(ADC_4096) - baseline[sensor];
   
     /* English Route: Convert pressure from mbar to lbs. */
     psi[sensor] = mbar[sensor] * MBARTOPSI;
     lbs[sensor] = psi[sensor] * area_sqin[sensor];

     /*Metric Route: Convert pressure from mbar to Newtons. */
     npsqm[sensor] = mbar[sensor] * MBARTONPSQM;
     newtons[sensor] = npsqm[sensor] * area_sqcm[sensor] * SQCMTOSQM;

     /* Sum all sensor values for force magnitude calculation. */
     magnitudeLBS += lbs[sensor];
     magnitudeN += newtons[sensor];

     /* Running sum of the product of the sensor coordinates and 
        the force for the centroid calculation. */
        centroidXsumLBS += (lbs[sensor] * centroidX[sensor]);
        centroidYsumLBS += (lbs[sensor] * centroidY[sensor]);
        centroidXsumN += (newtons[sensor] * centroidX[sensor]);
        centroidYsumN += (newtons[sensor] * centroidY[sensor]);
     
     // Decrement sensor number
     sensor--;
    }
  }

  /* Calculate centroid coordinates */
  centroidLBS[0] = centroidXsumLBS / magnitudeLBS;
  centroidLBS[1] = centroidYsumLBS / magnitudeLBS;
  centroidN[0] = centroidXsumN / magnitudeN;
  centroidN[1] = centroidYsumN / magnitudeN;

  b = millis() - a;
  
  Serial.print("Loop Time: ");
  Serial.print(b);
  Serial.println("\n");

  for(int i=0; i<NUMBEROFSENSORS; i++)
  {
    Serial.print(mbar[i]);
    Serial.print(", ");
  }
  Serial.print(magnitudeLBS);
  Serial.print(", ");
  Serial.print(magnitudeN);
  Serial.print("\n");

  //delay(100);
}
