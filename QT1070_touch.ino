#include <Wire.h>
#include "QTouch.h"

void setup() {

  Wire.begin();
  Serial.begin(115200);
  Serial.println("I2C Touch for Atmel QT1070");

  touchReset();

  // Settings to be adaped
  touchSetNTHRForKey(1000, 0);      //Set threshold value for a key (to be tested)
  touchSetNTHRForKey(1000, 1);      //Set threshold value for a key
  touchSetNTHRForKey(1000, 2);      //Set threshold value for a key
  touchSetNTHRForKey(1000, 3);      //Set threshold value for a key
  touchsetMaxDuration (60);         //value*180ms: determines how long any key can be in touch before it recalibrates itself
  touchSetGroup(1, 0);              //group key 1 to group 0
  touchSetGroup(2, 0);              //group key 2 to group 0
  touchSetGroup(4, 255);            // Disable key
  touchSetGroup(5, 255);            // Disable key
  touchSetGroup(6, 255);            // Disable key
  delay(300);                       // wait for device to restart
}

void loop() {

  //  Tests for touch//
  for (int i = 0; i < 7; i++) {
    if (touchTouched(i)) {
      Serial.print("Key ");
      Serial.print(i);
      Serial.println(" clicked");
    }
  }

  //  Tests for longtouch//
  for (int i = 0; i < 7; i++) {
    if (touchHold(i)) {
      Serial.print("Key ");
      Serial.print(i);
      Serial.println(" held long");
    }
  }


  //  // touchTime of a certain key can be used for long touch
  //  for (int i = 0; i < 7; i++) {
  //    duration = touchTime(i);
  //    if (duration > 0) {
  //      Serial.print("Key");
  //      Serial.print(i);
  //      Serial.print(" pressed ");
  //      Serial.print(duration);
  //      Serial.println(" ms´");
  //    }
  //  }


  delay(100);
}
