#include "Arduino.h"
#include <Wire.h>

#ifndef BMP_Sensor
#define BMP_Sensor

#define read2bytes() ((long)Wire.read() << 8 | (long)Wire.read())

extern char oss;

void read_calib();
void read_temp(long* temp);
void read_press(long* press);
void calculateAltitude(float currentPressure, double* altitude);
void calculateSeaLevelPressure(float currentPressure, double Altitude, float* SEA_LEVEL_PRESSURE);

#endif