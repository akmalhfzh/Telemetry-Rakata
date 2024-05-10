#define VIN 33 // define the Arduino pin A0 as voltage input (V in)
const float VCC = 5.0; // supply voltage 5V or 3.3V. If using PCB, set to 5V only.
const int model = 0;    // enter the model (see below)
float rawvoltage;

#include <movingAvg.h>   
movingAvg avgCurr(10);  

float cutOffLimit = 1.00; // reading cutt off current. 1.00 is 1 Ampere

/*
          "ACS758LCB-050B",// for model use 0
          "ACS758LCB-050U",// for model use 1
          "ACS758LCB-100B",// for model use 2
          "ACS758LCB-100U",// for model use 3
          "ACS758KCB-150B",// for model use 4
          "ACS758KCB-150U",// for model use 5
          "ACS758ECB-200B",// for model use 6
          "ACS758ECB-200U"// for model use  7   
sensitivity array is holding the sensitivity of the  ACS758
current sensors. Do not change.          
*/
float sensitivity[] = {
    40.0, // for ACS758LCB-050B
    60.0, // for ACS758LCB-050U
    20.0, // for ACS758LCB-100B
    40.0, // for ACS758LCB-100U
    13.3, // for ACS758KCB-150B
    16.7, // for ACS758KCB-150U
    10.0, // for ACS758ECB-200B
    20.0  // for ACS758ECB-200U     
};

/*         
 *   quiescent Output voltage is a factor for VCC that appears at output       
 *   when the current is zero. 
 *   for Bidirectional sensor it is 0.5 x VCC
 *   for Unidirectional sensor it is 0.12 x VCC
 *   for model ACS758LCB-050B, the B at the end represents Bidirectional (polarity doesn't matter)
 *   for model ACS758LCB-100U, the U at the end represents Unidirectional (polarity must match)
 *    Do not change.
 */
float quiescent_Output_voltage[] = {
    0.5,  // for ACS758LCB-050B
    0.12, // for ACS758LCB-050U
    0.5,  // for ACS758LCB-100B
    0.12, // for ACS758LCB-100U
    0.5,  // for ACS758KCB-150B
    0.12, // for ACS758KCB-150U
    0.5,  // for ACS758ECB-200B
    0.12  // for ACS758ECB-200U            
};

const float FACTOR = sensitivity[model] / 1000;             // set sensitivity for the selected model
const float QOV = quiescent_Output_voltage[model] * VCC;    // set quiescent Output voltage for the selected model
float voltage;                                               // internal variable for voltage

void setup()
{
  
    //Robojax.com ACS758 Current Sensor 
    Serial.begin(9600); // initialize serial monitor
    avgCurr.begin();
}

void loop()
{
    //Robojax.com ACS758 Current Sensor 
    rawvoltage = analogRead(VIN);
    float voltage_raw = (5.0 / 4095.0) * analogRead(VIN); // diganti nanti make 4095 karena 12 bit
    voltage = voltage_raw - QOV - 0.77;
    float current = voltage / FACTOR;

    float avgcur = avgCurr.reading(current); 
    Serial.println(avgcur);
    delay(100);

    
}
