#include "DHT_lib.h"
#include <Wire.h>
#include "BMP_lib.h"
#include "MPU6050_tockn.h"
#include "GPS_lib.h"
#include <SPI.h>
#include <SD.h>

//SD Card
const int chipSelect = 10;
//DHT11
float Temp_DHT = 0;
float RH = 0;
//BMP180
long Press = 0;
long Temp_BMP = 0;
//MPU6050
MPU6050 mpu6050(Wire);
float angelX = 0;
float angelY = 0;
float angelZ = 0;
//GPS
String GPSdata;
char gpsDataArray[43];
//LED & BUZZER
char LED_BUZZER = 0;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  //SD Card
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }
  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    //while (1);
  }
  Serial.println("card initialized.");
 
  init_dht(3);      //DHT11 pin
  oss = 0;          //BMP180
  mpu6050.begin();  //MPU6050
  mpu6050.calcGyroOffsets(true);
  pinMode(4, OUTPUT);  //LED
  pinMode(5, OUTPUT);  //BUZZER
  digitalWrite(4, HIGH);
  digitalWrite(5, HIGH);
  delay(1000);
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
}

void loop() {
  //LED & BUZZER
  if (Serial.available() > 0) {
    LED_BUZZER = Serial.read();
    if (LED_BUZZER == '1') {
      digitalWrite(4, HIGH);
    } else if (LED_BUZZER == '2') {
      digitalWrite(4, LOW);
    } else if (LED_BUZZER == '3') {
      digitalWrite(5, HIGH);
    } else if (LED_BUZZER == '4') {
      digitalWrite(5, LOW);
    }
  }

  //MPU
  mpu6050.update();
  angelX = mpu6050.getAngleX();
  angelY = mpu6050.getAngleY();
  angelZ = mpu6050.getAngleZ();
  Serial.print("MPU:");
  Serial.print(angelX);
  Serial.print(",");
  Serial.print(angelY);
  Serial.print(",");
  Serial.println(angelZ);


  //DHT11
  Serial.print("DHT:");
  switch (read_dht(3, &Temp_DHT, &RH)) {
    case 1:
      Serial.println("SENSOR ISN'T RESPONSE.");
      break;
    case 2:
      Serial.print(Temp_DHT);
      Serial.print(",");
      Serial.println(RH);
      break;
    case 3:
      Serial.println("ERROR IN READING DATA FROM THE SENSOR.");
      break;
  }

  //BMP180
  read_calib();
  read_temp(&Temp_BMP);
  Temp_BMP = Temp_BMP * 0.1;
  read_press(&Press);

  float currentPressure = Press / 100;  //hPa
  double Altitude_BMP = 0;
  calculateAltitude(currentPressure, &Altitude_BMP);

  float SEA_LEVEL_PRESSURE = 0;
  calculateSeaLevelPressure(currentPressure, Altitude_BMP, &SEA_LEVEL_PRESSURE);
  SEA_LEVEL_PRESSURE = SEA_LEVEL_PRESSURE * 100;

  Serial.print("BMP:");
  Serial.print(Temp_BMP);
  Serial.print(",");
  Serial.print(Press);
  Serial.print(",");
  Serial.print(Altitude_BMP);
  Serial.print(",");
  Serial.println(SEA_LEVEL_PRESSURE);

  //GPS
  GPSdata = read_GPS();
  //GPSdata.toCharArray(gpsDataArray, sizeof(gpsDataArray));
  Serial.print("GPS:");
  Serial.println(GPSdata);

  //SD Card
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("DATALOG.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.print("DHT:");
    dataFile.print(Temp_DHT);
    dataFile.print(",");
    dataFile.println(RH);
    dataFile.print("BMP:");
    dataFile.print(Temp_BMP);
    dataFile.print(",");
    dataFile.print(Press);
    dataFile.print(",");
    dataFile.print(Altitude_BMP);
    dataFile.print(",");
    dataFile.println(SEA_LEVEL_PRESSURE);
    dataFile.print("MPU:");
    dataFile.print(mpu6050.getAngleX());
    dataFile.print(",");
    dataFile.print(mpu6050.getAngleY());
    dataFile.print(",");
    dataFile.println(mpu6050.getAngleZ());
    dataFile.print("GPS:");
    dataFile.println(gpsDataArray);
    dataFile.close();
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
}
//data will send is
//DHT
//(Temp_DHT,RH)
//BMP
//(Temp_BMP,Press,Altitude_BMP,SEA_LEVEL_PRESSURE)
//MPU
//(angelX,angelY,angelZ)
//GPS
//(GPSdata)