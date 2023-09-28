#include "DHT_lib.h"
#include <Wire.h>
#include "BMP_lib.h"
#include "MPU6050_tockn.h"
#include "GPS_lib.h"
#include <SPI.h>
#include <SD.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Arduino.h>

#define CE_PIN 4
#define CSN_PIN 8
RF24 radio(CE_PIN, CSN_PIN);  // Create a Radio
struct package1 {
  float angleX = 10.5;
  float angleY = 13.3;
  float angleZ = 13.3;
  float Temp_DHT = 30.36;
  float RH = 50.3;
  long Temp_BMP = 300.1;
  float SEA_LEVEL_PRESSURE = 1325;
};
typedef struct package1 package1;
package1 data1;
struct package2 {
  long Press = 132500.56;
  double Altitude_BMP = 20.6;
  char lastString[15];
};
typedef struct package2 package2;
package2 data2;
char firstString[30];
unsigned long currentMillis;
unsigned long prevMillis;
unsigned long txIntervalMillis = 10;  // send once per second
const byte slaveAddress[5] = { 'R', 'x', 'A', 'A', 'A' };
const byte masterAddress[5] = { 'T', 'X', 'a', 'a', 'a' };

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
//float angelX;
//float angelY;
//float angelZ = 0;
//GPS
String GPSdata;
char gpsDataArray[43];
//LED & BUZZER
char LED_BUZZER = 0;

void setup() {
  Serial.begin(9600);

  Serial.println("MasterSwapRoles Starting");

  radio.begin();
  radio.setDataRate(RF24_2MBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.openWritingPipe(slaveAddress);
  radio.openReadingPipe(1, masterAddress);

  radio.setRetries(3, 5);  // delay, count
                           //   send(); // to get things started
  prevMillis = millis();   // set clock
  Wire.begin();
  //SD Card
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }
  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  //if (!SD.begin(chipSelect)) {
   // Serial.println("Card failed, or not present");
    // don't do anything more:
    //while (1) ;}
  Serial.println("card initialized.");

  init_dht(3);      //DHT11 pin
  oss = 0;          //BMP180
  mpu6050.begin();  //MPU6050
  mpu6050.calcGyroOffsets(true);
  pinMode(6, OUTPUT);  //LED
  pinMode(5, OUTPUT);  //BUZZER
  digitalWrite(6, HIGH);
  digitalWrite(5, HIGH);
  delay(1000);
  digitalWrite(6, LOW);
  digitalWrite(5, LOW);
}

void loop() {
  getData();
  if (LED_BUZZER == '1')
    digitalWrite(6, HIGH);
  else if (LED_BUZZER == '2')
    digitalWrite(6, LOW);
  else if (LED_BUZZER == '3')
    digitalWrite(5, HIGH);
  else if (LED_BUZZER == '4')
    digitalWrite(5, LOW);
  //MPU
  mpu6050.update();
  data1.angleX = mpu6050.getAngleX();
  data1.angleY = mpu6050.getAngleY();
  data1.angleZ = mpu6050.getAngleZ();


  //DHT11
  read_dht(3, &Temp_DHT, &RH);
  data1.Temp_DHT = Temp_DHT;
  data1.RH = RH;
  //BMP180
  read_calib();
  read_temp(&Temp_BMP);
  data1.Temp_BMP = Temp_BMP * 0.1;
  read_press(&Press);
  data2.Press = Press / 100;  //hPa
  double Altitude_BMP = 0;
  calculateAltitude(data2.Press, &Altitude_BMP);
  data2.Altitude_BMP = Altitude_BMP;
  float SEA_LEVEL_PRESSURE = 0;
  calculateSeaLevelPressure(data2.Press, Altitude_BMP, &SEA_LEVEL_PRESSURE);
  data1.SEA_LEVEL_PRESSURE = SEA_LEVEL_PRESSURE * 100;

  //GPS
  GPSdata = read_GPS();
  GPSdata.toCharArray(gpsDataArray, sizeof(gpsDataArray));
  for (byte i = 30; i < 42; i++) {
    data2.lastString[i - 30] = gpsDataArray[i];
  }
  data2.lastString[14] = "\0";
  for (byte i = 0; i < 30; i++) {
    firstString[i] = gpsDataArray[i];
  }
  firstString[30] = "\0";
  currentMillis = millis();
  if (currentMillis - prevMillis >= txIntervalMillis) {
    send();
    prevMillis = millis();
  }
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
//===============
void send() {

  radio.stopListening();
  bool rslt1;
  bool rslt2;
  bool rslt3;
  rslt1 = radio.write(&data1, sizeof(package1));
  rslt2 = radio.write(&data2, sizeof(package2));
  rslt3 = radio.write(&firstString, sizeof(firstString));
  radio.startListening();
  Serial.print("Data1 Sent: ");
  Serial.print("Data2 Sent: ");
  Serial.print("Data3 Sent: ");
  if (rslt1) {
    Serial.println("  Acknowledge1 received");
  } else {
    Serial.println("  Tx1 failed");
  }
  if (rslt2) {
    Serial.println("  Acknowledge2 received");
  } else {
    Serial.println("  Tx2 failed");
  }
  if (rslt3) {
    Serial.println("  Acknowledge3 received");
  } else {
    Serial.println("  Tx3 failed");
  }
}
//============
void getData() {
  if (radio.available()) {
    radio.read(&LED_BUZZER, sizeof(LED_BUZZER));
    Serial.print("Data1 received: ");
    Serial.println(LED_BUZZER);
  }
}