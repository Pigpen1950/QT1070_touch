/*
    QTouch Atmel QT1070

    Author: Christian Schulz
    2021-19-20

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    For wiring see ATMELs documentation. Tested on ESP32
*/


#define ADDR_QTOUCH             0x1B                               // address of QTouchIC
#define QTOUCH_REG_CHIPID       0x00                               // read chip id
#define QTOUCH_REG_VERSION      0x01                               // Firmware Version
#define QTOUCH_REG_DETSTATUS    0x02                               // detection status
#define QTOUCH_REG_KEYSTATUS    0x03                               // key status
#define QTOUCH_REG_SIGMSBK0     4                                  // MSByte of KEY0
#define QTOUCH_REG_SIGLSBK0     5                                  // LSByte of KEY0
#define QTOUCH_REG_REFMSBK0     18                                 // Reference data 0 MSByte
#define QTOUCH_REG_REFLSBK0     19                                 // Reference data 0 LSByte
#define QTOUCH_REG_NTHRK0       32                                 // Negative Threshold level for key 0
#define QTOUCH_REG_AVEASK0      39                                 // Adjacent key suppression level for key 0
#define QTOUCH_REG_DIK0         46                                 // Detection integrator counter for key 0
#define QTOUCH_REG_LPMODE       54                                 // low power mode
#define QTOUCH_REG_MAXDURA      55                                 // max on duration
#define QTOUCH_REG_CALIBRATE    56                                 // calibrate
#define QTOUCH_REG_RESET        57                                 // reset
#define QTOUCH_HOLD             400                                // ms for hold trigger
int lasthold = -1;



void touchWriteReg(unsigned char addr_reg, unsigned char dta) {    // write register
  Wire.beginTransmission(ADDR_QTOUCH);
  Wire.write(addr_reg); // register to read
  Wire.write(dta);
  Wire.endTransmission();
}

unsigned char touchReadReg(unsigned char addr_reg) {               // read register
  Wire.beginTransmission(ADDR_QTOUCH);
  Wire.write(addr_reg);                                            // register to read
  Wire.endTransmission();
  Wire.requestFrom(ADDR_QTOUCH, 1);                                // read a byte
  while (Wire.available()) {
    return Wire.read();
  }
}

void touchSetNTHRForKey(char nthr, char pin) {
  touchWriteReg(QTOUCH_REG_NTHRK0 + pin, nthr);
}

int touchGetSignalForKey(char pin) {                               // get the signal level for a certain pin
  return ((touchReadReg(QTOUCH_REG_SIGMSBK0 + pin * 2) << 8) + touchReadReg(QTOUCH_REG_SIGLSBK0 + pin * 2));
}

int touchGetRefDataForKey(char pin) {
  return ((touchReadReg(QTOUCH_REG_REFMSBK0 + pin * 2) << 8) + touchReadReg(QTOUCH_REG_REFLSBK0 + pin * 2));
}

void touchSetLowPowerMode(int val) {                               // set low power mode: value*8ms, default 2
  touchWriteReg(QTOUCH_REG_LPMODE, val);
}

void touchReset() {                                                // reset
  touchWriteReg(QTOUCH_REG_RESET, 0x55);                           // write a non-zero value to reset
}

void touchsetMaxDuration(char val) {                               // set maximum duration
  touchWriteReg(QTOUCH_REG_MAXDURA, val);
}

void touchcalibrate() {                                            // calibrate
  touchWriteReg(QTOUCH_REG_CALIBRATE, 0x55);                       // write a non-zero value to re-calibrate the device
}

int touchSetGroup(uint8_t key, uint8_t group) {                    // Set group (0-3) for a key or disable the key (255)
  uint8_t value;
  if (key > 6) {
    return -1;
  }
  if (group < 4) {
    value = touchReadReg(QTOUCH_REG_AVEASK0 + key) & 0xfc;
    if (value == 0) {
      value = 8 << 2;                                              //AVE default
    }
  } else if (group == 0xff) {
    value = 0;                                                     //Key disabled
    group = 1;                                                     //AKS default
  } else {
    return -1;
  }
  touchWriteReg(QTOUCH_REG_AVEASK0 + key, value | group);
  return 0;
}

int touchChipPresent(void) {                                       // check if the chip is present
  if (touchReadReg(QTOUCH_REG_CHIPID) == 0x2e) {
    return 1;
  }
  return 0;
}


unsigned char touchGetState() {                                    // return all key state, bit0 for key0, bit1 for key1....
  return touchReadReg(QTOUCH_REG_KEYSTATUS);
}


bool touchTouched(int key) {                                       // if certain key touched, return true
  if (key > 6) {
    return false;                                                  // err output
  }
  if (touchGetState() == 0) {
    lasthold = -1;                                                 //reset last touched if no key is pressed
  }
  return (bitRead(touchGetState(), key));                          // if touched return 1, else return 0
}


unsigned long touchTime(uint8_t key) {                             //return true if key is held
  unsigned long duration = 0;
  if (touchTouched(key)) {                                         //key is touched
    unsigned long startTime = millis();
    while (touchTouched(key)) {
      duration = millis() - startTime;
    }
    touchcalibrate();
  }
  return duration;
}


bool touchHold(uint8_t key) {                                      //return true if key is held longer than
  unsigned long duration = 0;
  if (touchTouched(key) && key != lasthold) {                      //key is touched
    unsigned long startTime = millis();
    while (touchTouched(key)) {
      duration = millis() - startTime;
      if (duration > QTOUCH_HOLD) {
        lasthold = key;                                            //avoid multiple hold
        break;
      }
    }
  }
  return (duration > QTOUCH_HOLD);
}
