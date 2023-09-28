#include "HardwareSerial.h"
#include "Arduino.h"
#include "DHT_lib.h"

void init_dht(char pin) {
  pinMode(pin, INPUT);
  digitalWrite(pin, HIGH);
}

char read_dht(char pin, float* temp, float* rh) {
  char data[40] = {};

  pinMode(pin, OUTPUT);

  //start signal
  digitalWrite(pin, LOW);
  delay(18);
  digitalWrite(pin, HIGH);

  //wait for response
  pinMode(pin, INPUT);
  delayMicroseconds(40);

  //check response
  if (digitalRead(pin) == HIGH) {
    return 1;
  }

  while (digitalRead(pin) == LOW)
    ;
  while (digitalRead(pin) == HIGH)
    ;

  //Read data
  for (int i = 0; i < 40; i++) {
    while (digitalRead(pin) == LOW)
      ;
    delayMicroseconds(30);
    if (digitalRead(pin) == HIGH) {
      data[i] = 1;
      while (digitalRead(pin) == HIGH)
        ;
    }
  }

  int rh_int = 0;  //0b 0000 0000
  for (int i = 0; i < 8; i++) {
    rh_int = rh_int << 1;
    if (data[i] == 1) {
      rh_int |= 1;
    }
  }
  int rh_dec = 0;  //0b 0000 0000
  for (int i = 8; i < 16; i++) {
    rh_dec = rh_dec << 1;
    if (data[i] == 1) {
      rh_dec |= 1;
    }
  }
  int temp_int = 0;  //0b 0000 0000
  for (int i = 16; i < 24; i++) {
    temp_int = temp_int << 1;
    if (data[i] == 1) {
      temp_int |= 1;
    }
  }
  int temp_dec = 0;  //0b 0000 0000
  for (int i = 24; i < 32; i++) {
    temp_dec = temp_dec << 1;
    if (data[i] == 1) {
      temp_dec |= 1;
    }
  }
  int sum_check = 0;  //0b 0000 0000
  for (int i = 32; i < 40; i++) {
    sum_check = sum_check << 1;
    if (data[i] == 1) {
      sum_check |= 1;
    }
  }

  //Check the sum
  if (sum_check == rh_int + rh_dec + temp_int + temp_dec) {

    float deci = rh_dec;
    while (deci >= 1) {
      deci /= 10.0;
    }
    *rh = rh_int + deci;
    deci = temp_dec;
    while (deci >= 1) {
      deci /= 10.0;
    }
    *temp = temp_int + deci;
    return 2;
  }
  return 3;
}