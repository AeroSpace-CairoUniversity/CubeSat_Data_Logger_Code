// SlaveSwapRoles

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


#define CE_PIN 9
#define CSN_PIN 8

const byte slaveAddress[5] = { 'R', 'x', 'A', 'A', 'A' };
const byte masterAddress[5] = { 'T', 'X', 'a', 'a', 'a' };

RF24 radio(CE_PIN, CSN_PIN);  // Create a Radio
struct package1 {
  float AngleX;
  float AngleY;
  float AngleZ;
  float Temp_DHT;
  float RH;
  long Temp_BMP;
  float SEA_LEVEL_PRESSURE;
};
typedef struct package1 package1;
package1 data1;
struct package2 {
  long Press;
  double Altitude_BMP;
  char lastString[15];
};
typedef struct package2 package2;
package2 data2;
char firstString[30];
String data3toreceive;
int replyData[2] = { 109, -4000 };  // the two values to be sent to the master
bool newData1 = false;
bool newData2 = false;
bool newData3 = false;

unsigned long currentMillis;
unsigned long prevMillis;
unsigned long txIntervalMillis = 1000;  // send once per second
char new_LED_BUZZER = "1";
char last_LED_BUZZER = "0";

void setup() {

  Serial.begin(9600);

  Serial.println("SlaveSwapRoles Starting");

  radio.begin();
  radio.setDataRate(RF24_2MBPS);

  radio.openWritingPipe(masterAddress);  // NB these are swapped compared to the master
  radio.openReadingPipe(1, slaveAddress);
  radio.setPALevel(RF24_PA_MAX);
  radio.setRetries(3, 5);  // delay, count
  radio.startListening();
}

//====================

void loop() {
  //LED & BUZZER
  last_LED_BUZZER = new_LED_BUZZER;
  if (last_LED_BUZZER != new_LED_BUZZER) {
    send();
  }
  if (Serial.available() > 0) {
    new_LED_BUZZER = Serial.read();
    if (new_LED_BUZZER == '1') {
      digitalWrite(4, HIGH);
    } else if (new_LED_BUZZER == '2') {
      digitalWrite(4, LOW);
    } else if (new_LED_BUZZER == '3') {
      digitalWrite(5, HIGH);
    } else if (new_LED_BUZZER == '4') {
      digitalWrite(5, LOW);
    }
  }
  getData();
  data3toreceive = String(data2.lastString) + String(firstString);
  showData();
}

//====================

void send() {
  radio.stopListening();
  bool rslt;
  rslt = radio.write(&new_LED_BUZZER, sizeof(new_LED_BUZZER));
  radio.startListening();
  if (rslt) {
    Serial.println("Acknowledge Received");
  } else {
    Serial.println("Tx failed");
  }
}

//================

void getData() {

  //while (radio.available()){
  if (radio.available()) {
    radio.read(&data1, sizeof(package1));
    newData1 = true;
    Serial.print("Data1 received: ");
  } else newData1 = false;
  delay(2);
  if (radio.available()) {
    //while(radio.available()){
    radio.read(&data2, sizeof(package2));
    newData2 = true;
    Serial.print("Data2 received: ");
  } else newData2 = false;

  delay(2);
  if (radio.available()) {
    radio.read(&firstString, sizeof(firstString));
    newData3 = true;

  } else newData3 = false;
  //}
}

//================

void showData() {
  if (newData1 == true) {
    Serial.println("Data1 received ");
    Serial.print("MPU:");
    Serial.print(data1.AngleX);
    Serial.print(",");
    Serial.print(data1.AngleY);
    Serial.print(",");
    Serial.println(data1.AngleZ);
    Serial.print("DHT:");
    Serial.print(data1.Temp_DHT);
    Serial.print(",");
    Serial.println(data1.RH);
    Serial.print("BMP:");
    Serial.print(data1.Temp_BMP);
    Serial.print(",");
    Serial.print(data1.SEA_LEVEL_PRESSURE * 100);
  }
  if (newData2 == true) {
    Serial.print("(Data2 received )");
    Serial.print(",");
    Serial.print(data2.Press);
    Serial.print(",");
    Serial.println(data2.Altitude_BMP);
  }
  if (newData3 == true) {
    Serial.println("Data3 received ");
    Serial.println(data3toreceive);
  }
}

//================
