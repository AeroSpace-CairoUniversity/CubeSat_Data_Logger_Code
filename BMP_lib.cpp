#include "BMP_lib.h"

short AC1 = 0;
short AC2 = 0;
short AC3 = 0;
unsigned short AC4 = 0;
unsigned short AC5 = 0;
unsigned short AC6 = 0;
short B_1 = 0;
short B2 = 0;
short MB = 0;
short MC = 0;
short MD = 0;

long X1 = 0;
long X2 = 0;
long B5 = 0;

char oss = 0;


void read_calib() {

  Wire.beginTransmission(0x77);
  Wire.write(0xAA);
  Wire.endTransmission();

  Wire.requestFrom(0x77, 22);

  if (Wire.available() == 22) {

    AC1 = read2bytes();
    AC2 = read2bytes();
    AC3 = read2bytes();
    AC4 = read2bytes();
    AC5 = read2bytes();
    AC6 = read2bytes();
    B_1 = read2bytes();
    B2 = read2bytes();
    MB = read2bytes();
    MC = read2bytes();
    MD = read2bytes();
  }
  //Serial.println(AC1);  Serial.println(AC2);Serial.println(AC3);Serial.println(AC4);Serial.println(AC5);Serial.println(AC6);
  //Serial.println(B_1);Serial.println(B2);Serial.println(MB);Serial.println(MC);Serial.println(MD);
}
void read_temp(long* temp) {
  //read uncompensated temperature
  Wire.beginTransmission(0x77);
  Wire.write(0xF4);
  Wire.write(0x2E);
  Wire.endTransmission();

  delayMicroseconds(4500);

  Wire.beginTransmission(0x77);
  Wire.write(0xF6);
  Wire.endTransmission();

  Wire.requestFrom(0x77, 2);

  long UT = 0;
  if (Wire.available() == 2) { UT = read2bytes(); }

  //calculate true temperature
  X1 = (UT - AC6) * AC5 / pow(2, 15);
  X2 = MC * pow(2, 11) / (X1 + MD);
  B5 = X1 + X2;
  *temp = (B5 + 8) / pow(2, 4);
}
void read_press(long* press) {
  //read uncompensated pressure
  Wire.beginTransmission(0x77);
  Wire.write(0xF4);
  Wire.write(0x34 + (oss << 6));
  Wire.endTransmission();

  switch (oss) {
    case 0:
      delayMicroseconds(4500);
      break;
    case 1:
      delayMicroseconds(7500);
      break;
    case 2:
      delayMicroseconds(13500);
      break;
    case 3:
      delay(25);
      delayMicroseconds(500);
      break;
  }

  Wire.beginTransmission(0x77);
  Wire.write(0xF6);
  Wire.endTransmission();

  Wire.requestFrom(0x77, 3);

  long UP = 0;
  if (Wire.available() == 3) {
    UP = read2bytes();
    UP = (UP << 8 | Wire.read()) >> (8 - oss);
  }

  //calculate true pressure
  long B6 = B5 - 4000;
  X1 = (B2 * (B6 * B6 / pow(2, 12))) / pow(2, 11);
  X2 = AC2 * B6 / pow(2, 11);
  long X3 = X1 + X2;
  long B3 = (((AC1 * 4 + X3) << oss) + 2) / 4;
  X1 = AC3 * B6 / pow(2, 13);
  X2 = (B_1 * (B6 * B6 / pow(2, 12))) / pow(2, 16);
  X3 = ((X1 + X2) + 2) / pow(2, 2);
  unsigned long B4 = AC4 * (unsigned long)(X3 + 32768) / pow(2, 15);
  unsigned long B7 = ((unsigned long)UP - B3) * (50000 >> oss);
  long p = 0;
  if (B7 < 0x80000000) {
    p = (B7 * 2) / B4;
  } else {
    p = (B7 / B4) * 2;
  }
  X1 = (p / pow(2, 8)) * (p / pow(2, 8));
  X1 = (X1 * 3038) / pow(2, 16);
  X2 = (-7357 * p) / pow(2, 16);
  *press = p + (X1 + X2 + 3791) / pow(2, 4);
}

void calculateAltitude(float currentPressure, double* altitude) {
  float SEA_LEVEL_PRESSURE = 1013.25;  //hPa
  *altitude = (44330.0 * (1.0 - pow(currentPressure / SEA_LEVEL_PRESSURE, 1 / 5.255)));
}

void calculateSeaLevelPressure(float currentPressure, double Altitude, float* SEA_LEVEL_PRESSURE) {
  *SEA_LEVEL_PRESSURE = currentPressure / pow(1 - (Altitude / 44330), 5.255);
}
