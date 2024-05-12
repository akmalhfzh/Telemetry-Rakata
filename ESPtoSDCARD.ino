#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <movingAvg.h>

#define VIN 33
#define VDIN 15
const int thermistorPin = 34;

// Variabel untuk menyimpan nilai daya sebelumnya
float prevPower = 0.0;

// Deklarasi fungsi
void saveDataToCSV(float time, float temperature, float current, float voltage, float power);

const float resistorValue = 10000.0; // Nilai resistor (10k)
const float nominalResistance = 10000.0; // Nilai resistansi pada suhu tertentu (biasanya 25°C)
const float nominalTemperature = 25.0; // Suhu nominal dalam derajat Celsius
const float betaCoefficient = 3950; // Koefisien Beta dari termistor

const float VCC = 5.0; 
const int model = 0;    // enter the model (see below)
float rawvoltage;
movingAvg avgCurr(10);  
movingAvg avgTemp(10);
movingAvg avgVol(10);
float cutOffLimit = 1.00; // reading cutt off current. 1.00 is 1 Ampere

// Floats for ADC voltage & Input voltage
float adc_voltage = 0.0;
float in_voltage = 0.0;
// Floats for resistor values in divider (in ohms)
float R1 = 45000.0;
float R2 = 5000.0; 
// Integer for ADC value
int adc_value = 0;


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
  dataFile.println("Waktu, Suhu (C), Arus (A), Tegangan (V), Power (W)");

  // Close file
  dataFile.close();

  // Inisialisasi sensor arus
  avgCurr.begin();
  // Inisialisasi sensor suhu
  avgTemp.begin();
  // Inisialisasi sensor tegangan
  avgVol.begin();
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
  float avgtemp = avgTemp.reading(steinhart)-14; // kalibrasi
  
  // Baca nilai arus
  rawvoltage = analogRead(VIN);
  float voltage_raw = (5.0 / 4095.0) * analogRead(VIN); // diganti nanti make 4095 karena 12 bit
  voltage = voltage_raw - QOV - 0.77;
  float current = voltage / FACTOR;
  float avgcurr = avgCurr.reading(current); 

  // Baca nilai tegangan
  adc_value = analogRead(VDIN);
  adc_voltage  = (adc_value * VCC) /4095.0; 
  in_voltage = adc_voltage / (R2/(R1+R2)) ; 
  float avgvol = avgVol.reading(in_voltage); 

  // Hitung daya
  float power = avgvol * avgcurr;

  // Akumulasi nilai daya sebelumnya dengan nilai daya baru
  power += prevPower;
    
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
  dataFile.print(avgtemp);
  dataFile.print(",");
  dataFile.print(avgcurr);
  dataFile.print(",");
  dataFile.print(avgvol);
  dataFile.print(",");
  dataFile.println(power);

  // Close file
  dataFile.close();

  // Output to serial monitor
  Serial.print("Waktu: ");
  Serial.print(currentTime);
  Serial.print(", Suhu: ");
  Serial.print(avgtemp);
  Serial.print(" °C, Arus: ");
  Serial.print(avgcurr);
  Serial.println(" A, Tegangan: ");
  Serial.print(avgvol);
  Serial.println(" V, Daya: ");
  Serial.print(power);
  Serial.println(" W");

  delay(200); 
}
