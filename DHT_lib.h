#include "Arduino.h"

#ifndef DHT11Sensor
#define DHT11Sensor

void init_dht(char pin);

char read_dht(char pin, float* temp, float* rh);

#endif
