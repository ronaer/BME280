/* INFO
---------------------------------------------------------------------
"GYBMEP (BMP/BME280) / P10 ve I²C LCD DEMO" 
ARALIK 2022/Izmir/TR                   
bilgi@ronaer.com                                      
https://www.instagram.com/dr.tronik2022/                       
https://www.youtube.com/@DrTRonik                     
---------------------------------------------------------------------
*/

/********************************************************************
  GLOBALS___GLOBALS___GLOBALS___GLOBALS___GLOBALS___GLOBALS___
 ********************************************************************/
#include <LiquidCrystal_I2C.h>  //https://github.com/ronaer/I2C_Lib_LiquidCrystal  (LCD için başka kütüphane kullanılacaksa kodlar uyarlanmalı)
#include <Wire.h>
#include <SPI.h>              //DMD kütüphanesi ile P10 panel sürmek için gerekli: https://github.com/PaulStoffregen/SPI
#include <DMD.h>              //https://github.com/ronaer/P10-Led-Tabela-RTC-DHT/raw/master/DMD-master.zip
#include <Adafruit_BME280.h>  //https://github.com/adafruit/Adafruit_BME280_Library
#include <Adafruit_Sensor.h>  //https://github.com/adafruit/Adafruit_Sensor
#include "TimerOne.h"         //https://github.com/PaulStoffregen/TimerOne
#include <SystemFont5x7.h>    //Font kütüphanesi, DMD kütüphane klasörü içinde olmalı

LiquidCrystal_I2C lcd(0x3F, 20, 4);
//Kullandığımız LCD I²C adresi (0x3F 0x27 vb. farklı olabilir, doğru girilmezse birşey görünmez) ve karakter satır sayısı

DMD dmd(2, 2);                          // yatayda:2 , dikeyde:2 panel 2x2...
#define SEALEVELPRESSURE_HPA (1019.50)  //Deniz seviyesi hava basınç düzeyi, yükseklik hesaplamak için doğru girilmeli...
// "AYNI ANDAKİ" deniz seviyesi hava basıncı girilirse yükseklik net hesaplanıyor...
// Yoksa sabit değer girildiğinde yükseklik basınç ile birlikte hep değişir...

Adafruit_BME280 bme;

int temp, hum, pressure, altitude;                    //sıcaklık, nem, basınç ve yükseklik değerlerini tutacağımız değişkenlerimiz...
float temp_lcd, hum_lcd, pressure_lcd, altitude_lcd;  //LCD için ondalıklı ve alternatif örnek için...


/********************************************************************
  VOIDs__VOIDs__VOIDs__VOIDs__VOIDs__VOIDs__VOIDs__VOIDs__VOIDs__VOIDs
********************************************************************/
/*--------------------------------------------------------------------------------------
  Timer1 (TimerOne) tarafından yönlendirilen DMD yenileme taraması için kesme işleyicisi,
  Timer1.initialize() içinde ayarlanan periyotta çağrılır;
  --------------------------------------------------------------------------------------*/
void ScanDMD() {
  dmd.scanDisplayBySPI();
}

/********************************************************************
  SETUP___SETUP___SETUP___SETUP___SETUP___SETUP___SETUP___SETUP___
 ********************************************************************/
void setup() {
  Serial.begin(9600);
  lcd.begin();  //LCD başlasın
  lcd.clear();  // ve ekran temizlensin
  Timer1.initialize(3000);
  Timer1.attachInterrupt(ScanDMD);  //Timer kesmesini ScanDMD fonksiyonu ile attach edelim...
  dmd.clearScreen(true);            //P10 ledlerin hepsini bir söndürelim

  //P10 panel açılış metni...
  dmd.selectFont(SystemFont5x7);  //Font seçimi
  dmd.drawString(2, 1, "BME/BMP280", 10, GRAPHICS_NORMAL);
  dmd.drawString(8, 13, "SICAKLIK", 8, GRAPHICS_NORMAL);
  dmd.drawString(2, 25, "NEM BASINC", 10, GRAPHICS_NORMAL);

  //LCD açılış metni...
  lcd.setCursor(0, 0);
  lcd.print(" METEOROLOJiK PANEL");
  lcd.setCursor(0, 1);
  lcd.print("DERECE, NEM, BASINC,");
  lcd.setCursor(0, 2);
  lcd.print(" YAKLASIK YUKSEKLiK");
  lcd.setCursor(0, 3);
  lcd.print("*Dr.TRonik//YouTube*");

  delay(5000);            // Ekranda 5 sn. yazı kalsın, bu arada da sensör ölçüm işlemine başlar...
  dmd.clearScreen(true);  // P10 paneli temizleyelim...
  lcd.clear();            // ve LCD ekran da temizlensin

  unsigned status;           // I²C adresimzi kütüphaneye bildirelim, çakma modüllerde adres, kütüphanedeki adresten farklı...
  status = bme.begin(0x76);  //Sensör mödüllerinde adres farklı ise parantez içinde yer almalı...

  //İlk açılış metninden sonra boş ekran gelmesin, veriler okunsun ve loop öncesi görelim...
  read_data();
  dmd_yaz();
  lcd_yaz();
}

/********************************************************************
  LOOP__LOOP__LOOP__LOOP__LOOP__LOOP__LOOP__LOOP__LOOP__LOOP__LOOP__
 ********************************************************************/
void loop() {

  static unsigned long timer = millis();

  //Dakikada bir defa, verileri sensörden alalım...
  if (millis() - timer > 1 * 60000) {
    timer = millis();
    read_data();
    dmd_yaz();
    lcd_yaz();
  }
}

/********************************************************************
  VOIDs__VOIDs__VOIDs__VOIDs__VOIDs__VOIDs__VOIDs__VOIDs__VOIDs__VOIDs
********************************************************************/

//BME280'den veri alma...
void read_data() {
  temp = bme.readTemperature();
  hum = bme.readHumidity();  //Sadece BME için...
  pressure = bme.readPressure() / 100.0F;
  altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);

  temp_lcd = bme.readTemperature();
  hum_lcd = bme.readHumidity();  //Sadece BME için...
  pressure_lcd = bme.readPressure() / 100.0F;
  altitude_lcd = bme.readAltitude(SEALEVELPRESSURE_HPA);

  // Serial.println(temp_lcd);
  // Serial.println(hum_lcd);
  // Serial.println(pressure_lcd);
  // Serial.println(altitude_lcd);
}

//P10 a veri yazdırma...
void dmd_yaz() {

  dmd.selectFont(SystemFont5x7);  //Font seçimi
  char buffer[10];                //dmd kütüphanesi ile drawString fonksiyonunda değerleri yazdırabilmek için...

  dmd.drawString(42, 0, (dtostrf(temp, 2, 0, buffer)), 2, GRAPHICS_NORMAL);  // int--> stringe çevirelim ve drawString fonksiyonunda kullanalım; spirntf'e alternatif...
  dmd.drawString(42, 8, (dtostrf(hum, 2, 0, buffer)), 2, GRAPHICS_NORMAL);
  dmd.drawString(41, 16, (dtostrf(pressure, 2, 0, buffer)), 4, GRAPHICS_NORMAL);
  dmd.drawString(42, 25, (dtostrf(altitude, 2, 0, buffer)), 2, GRAPHICS_NORMAL);

  dmd.drawCircle(56, 1, 1, GRAPHICS_NORMAL);             // derece simgesi...
  dmd.drawString(59, 0, "C", 1, GRAPHICS_NORMAL);        // C harfi...
  dmd.drawString(0, 0, "Derece:", 7, GRAPHICS_NORMAL);   // Derece...
  dmd.drawString(18, 8, "Nem:", 4, GRAPHICS_NORMAL);     // Nem...
  dmd.drawString(56, 8, "%", 1, GRAPHICS_NORMAL);        // %...
  dmd.drawString(0, 16, "hPa/mb:", 7, GRAPHICS_NORMAL);  // hectoPascal/milibar...
  dmd.drawString(55, 25, "M.", 2, GRAPHICS_NORMAL);      // Metre...
  dmd.drawString(0, 25, "Yksklk:", 7, GRAPHICS_NORMAL);  // Yükseklik: ...
}

//LCD ekrana veri yazdırma...
void lcd_yaz() {
  lcd.setCursor(0, 0);
  lcd.print(temp_lcd) + lcd.print(" 'C Derece");
  lcd.setCursor(0, 1);
  lcd.print("%") + lcd.print(hum_lcd) + lcd.print(" Nem");
  lcd.setCursor(0, 2);
  lcd.print("HavaBas:") + lcd.print(pressure_lcd) + lcd.print(" mbar");
  lcd.setCursor(0, 3);
  lcd.print("Yksklik:") + lcd.print(altitude_lcd) + lcd.print(" metre");
}

/*___İletişim:
  e-posta: bilgi@ronaer.com
  https://www.instagram.com/dr.tronik2022/
  YouTube: https://www.youtube.com/@DrTRonik
*/