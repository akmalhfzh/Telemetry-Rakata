#include <Wire.h>
#include "DFRobot_INA219.h"
#include <SPI.h>
#include <SD.h>

DFRobot_INA219_IIC ina219(&Wire, INA219_I2C_ADDRESS4);

// Kalibrasi sensor
float ina219Reading_mA = 1000;
float extMeterReading_mA = 1000;

// Konfigurasi untuk microSD
const int chipSelect = 10;
File dataFile;

void setup(void) 
{
    // Inisialisasi Serial Monitor
    Serial.begin(115200);
    while (!Serial);

    // Inisialisasi INA219
    Serial.println();
    while (ina219.begin() != true) {
        Serial.println("INA219 begin failed");
        delay(2000);
    }
    ina219.linearCalibrate(ina219Reading_mA, extMeterReading_mA);
    Serial.println();

    // Inisialisasi SD Card
    if (!SD.begin(chipSelect)) {
        Serial.println("Initialization of SD card failed!");
        return;
    }
    Serial.println("SD card initialized successfully.");

    // Buat dan buka file untuk menulis
    dataFile = SD.open("/dataproto.csv", FILE_WRITE);
    if (dataFile) {
        // Tulis header ke file
        dataFile.println("Waktu (ms), BusVoltage (V), ShuntVoltage (mV), Current (mA), Power (mW)");
        dataFile.close();
    } else {
        Serial.println("Error opening datalog.csv");
    }
}

void loop(void)
{
    // Baca data dari sensor INA219
    float busVoltage = ina219.getBusVoltage_V();
    float shuntVoltage = ina219.getShuntVoltage_mV();
    float current_mA = ina219.getCurrent_mA();
    float power_mW = ina219.getPower_mW();
    


    // Tampilkan data di Serial Monitor
    Serial.print("BusVoltage:   ");
    Serial.print(busVoltage, 2);
    Serial.println("V");
    Serial.print("ShuntVoltage: ");
    Serial.print(shuntVoltage, 3);
    Serial.println("mV");
    Serial.print("Current:      ");
    Serial.print(current_mA, 1);
    Serial.println("mA");
    Serial.print("Power:        ");
    Serial.print(power_mW, 1);
    Serial.println("mW");
    Serial.println("");

    // Dapatkan waktu saat ini
    unsigned long currentTime = millis();


    dataFile.print(currentTime);
    dataFile.print(",");
    dataFile.print(busVoltage, 2);
    dataFile.print(",");
    dataFile.print(shuntVoltage, 3);
    dataFile.print(",");
    dataFile.print(current_mA, 1);
    dataFile.print(",");
    dataFile.println(power_mW, 1);

        // Tutup file setelah menulis data
    dataFile.close();

    // Delay 1 detik sebelum pengukuran berikutnya
    delay(1000);
}
