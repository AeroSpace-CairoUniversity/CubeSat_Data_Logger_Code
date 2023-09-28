#include "GPS_lib.h"

char Massege_ID[6];
char UTC_Position[11];
char LATITUDE[10];
char N_S[2];
char LONGITUDE[11];
char E_W[2];
char Position_Fix_Indicator[2];
char Satellites_Used[3];
char HDOP[4];
char MSL_Altitude[7];

String read_GPS() {
  while (1 == 1) {
    if (Serial.read() == '$') {
      Serial.readBytesUntil(',', Massege_ID, 6);

      //check massege ID
      if (Massege_ID[2] == 'G' && Massege_ID[3] == 'G' && Massege_ID[4] == 'A') {

        //read data
        Serial.readBytesUntil(',', UTC_Position, 11);
        Serial.readBytesUntil(',', LATITUDE, 10);
        Serial.readBytesUntil(',', N_S, 2);
        Serial.readBytesUntil(',', LONGITUDE, 11);
        Serial.readBytesUntil(',', E_W, 2);
        Serial.readBytesUntil(',', Position_Fix_Indicator, 2);
        Serial.readBytesUntil(',', Satellites_Used, 3);
        Serial.readBytesUntil(',', HDOP, 4);
        Serial.readBytesUntil(',', MSL_Altitude, 7);

        //null char
        Massege_ID[5] = '\0';
        UTC_Position[10] = '\0';
        LATITUDE[9] = '\0';
        N_S[1] = '\0';
        LONGITUDE[10] = '\0';
        E_W[1] = '\0';
        Position_Fix_Indicator[1] = '\0';
        Satellites_Used[2] = '\0';
        HDOP[3] = '\0';
        MSL_Altitude[6] = '\0';

        String gpsData = String(Massege_ID) + "," + String(LATITUDE) + "," + String(N_S) + "," + String(LONGITUDE) ;

        return gpsData;
      }
    }
  }
}