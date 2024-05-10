#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <movingAvg.h>

#define VIN 33 // define the Arduino pin A0 as voltage input (V in)
const int thermistorPin = 34; // Hubungkan thermistor ke pin analog A0
const float resistorValue = 10000.0; // Nilai resistor (10k)
const float nominalResistance = 10000.0; // Nilai resistansi pada suhu tertentu (biasanya 25°C)
const float nominalTemperature = 25.0; // Suhu nominal dalam derajat Celsius
const float betaCoefficient = 3950; // Koefisien Beta dari termistor

const float VCC = 5.0; // supply voltage 5V or 3.3V. If using PCB, set to 5V only.
const int model = 0;    // enter the model (see below)
float rawvoltage;
movingAvg avgCurr(10);  
float cutOffLimit = 1.00; // reading cutt off current. 1.00 is 1 Ampere

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

const float FACTOR = sensitivity[model] / 1000;             
const float QOV = quiescent_Output_voltage[model] * VCC;    
float voltage;                                               

File dataFile;

void setup() {
  Serial.begin(9600);

  if(!SD.begin()) {
    Serial.println("Gagal inisialisasi SD card!");
    return;
  }
  Serial.println("SD card siap!");

  // Create and open file for writing
  dataFile = SD.open("/data.csv", FILE_WRITE);
  if(!dataFile) {
    Serial.println("Gagal membuka file untuk menulis");
    return;
  }

  // Write header to file
  dataFile.println("Waktu, Suhu (C), Arus (A), Tegangan (V)");

  // Close file
  dataFile.close();

  // Inisialisasi sensor arus
  avgCurr.begin();
}

void loop() {
  // Baca nilai tegangan pada pin thermistor
  int rawADC = analogRead(thermistorPin);
  float voltage = rawADC * (5.0 / 4095.0); // 5.0 adalah tegangan referensi pada Arduino
  float resistance = resistorValue * (5.0 / voltage - 1.0);
  float steinhart;
  steinhart = resistance / nominalResistance;   // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= betaCoefficient;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (nominalTemperature + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // Konversi dari Kelvin ke Celsius
  
  // Baca nilai arus
  rawvoltage = analogRead(VIN);
  float voltage_raw = (5.0 / 4095.0) * analogRead(VIN); // diganti nanti make 4095 karena 12 bit
  voltage = voltage_raw - QOV - 0.77;
  float current = voltage / FACTOR;
  float avg = avgCurr.reading(current); 

  // Open file for appending
  dataFile = SD.open("/data.csv", FILE_APPEND);
  if(!dataFile) {
    Serial.println("Gagal membuka file untuk menambahkan data");
    return;
  }

  // Get current time
  unsigned long currentTime = millis();

  // Write data to file
  dataFile.print(currentTime);
  dataFile.print(",");
  dataFile.print(steinhart);
  dataFile.print(",");
  dataFile.println(avg);

  // Close file
  dataFile.close();

  // Output to serial monitor
  Serial.print("Waktu: ");
  Serial.print(currentTime);
  Serial.print(", Suhu: ");
  Serial.print(steinhart);
  Serial.print(" °C, Arus: ");
  Serial.print(avg);
  Serial.println(" A");

  delay(1000); // Tunda selama 1 detik
}