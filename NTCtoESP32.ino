// Deklarasi pin sensor dan resistor
const int thermistorPin = 34; // Hubungkan thermistor ke pin analog A0
const float resistorValue = 10000.0; // Nilai resistor (10k)
const float nominalResistance = 10000.0; // Nilai resistansi pada suhu tertentu (biasanya 25°C)
const float nominalTemperature = 25.0; // Suhu nominal dalam derajat Celsius
const float betaCoefficient = 3950; // Koefisien Beta dari termistor
#include <movingAvg.h>
movingAvg avgTemp(10);

void setup() {
  Serial.begin(9600); // Mulai komunikasi serial dengan kecepatan 9600 baud
  avgTemp.begin();
}

void loop() {
  // Baca nilai tegangan pada pin thermistor
  int rawADC = analogRead(thermistorPin);

  // Konversi nilai ADC menjadi tegangan
  float voltage = rawADC * (5.0 / 4095.0); // 5.0 adalah tegangan referensi pada Arduino
  
  // Hitung resistansi termistor menggunakan pembagi tegangan
  float resistance = resistorValue * (5.0 / voltage - 1.0);
  
  // Hitung suhu menggunakan rumus Steinhart-Hart
  float steinhart;
  steinhart = resistance / nominalResistance;   // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= betaCoefficient;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (nominalTemperature + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // Konversi dari Kelvin ke Celsius

  // Tampilkan suhu dalam derajat Celsius
  float avgtemp = avgTemp.reading(steinhart);
  Serial.print("Suhu: ");
  Serial.print(avgtemp);
  Serial.println(" °C");

  delay(500); // Tunda selama 1 detik
}
