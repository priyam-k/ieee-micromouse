//#include <StandardCplusplus.h>

//#include <iostream>
//#include <string>
//#include <vector>

#include <f401reMap.h>

#include <SparkFun_TB6612.h>
#include "Simple_MPU6050.h"

#define MPU6050_DEFAULT_ADDRESS     0x68

Simple_MPU6050 mpu;

Motor motor24 = Motor(10, 4, 5, 1, 9);
Motor motor13 = Motor(7, 8, 6, 1, 9);

int speed13 = 0, speed24 = 0;
float angle = 0;

int orientation = 0;

const float wheel_diam = 6.7;
const float pi = 3.14159;
const float encoder_slots = 18;

int clicksR = 0, clicksL = 0;
bool cRrising = false, cLrising = false;
float dist = 0;

void updateClicks() {
  int cL = digitalRead(2), cR = digitalRead(3);
  if (cL == 0 && cLrising) cLrising = false;
  else if (cL == 1 && !cLrising) {
    cLrising = true;
    clicksL++;
  }
  if (cR == 0 && cRrising) cRrising = false;
  else if (cR == 1 && !cRrising) {
    cRrising = true;
    clicksR++;
  }
  dist = (wheel_diam * pi) / encoder_slots * (clicksR + clicksL) / 2;
}

void updateAngle(int16_t *gyro, int16_t *accel, int32_t *quat) {
  Quaternion q;
  VectorFloat gravity;
  float ypr[3] = { 0, 0, 0 };
  float xyz[3] = { 0, 0, 0 };
  mpu.GetQuaternion(&q, quat);
  mpu.GetGravity(&gravity, &q);
  mpu.GetYawPitchRoll(ypr, &q, &gravity);
  mpu.ConvertToDegrees(ypr, xyz);
  angle = xyz[0];
  if (xyz[0] < 0) angle = 360 + xyz[0];
  Serial.print("angle: ");
  Serial.print(angle);
  Serial.println();
}

void readGyro() {
  mpu.dmp_read_fifo(false);
}

void setSpeeds(int newspeed13, int newspeed24) {
  if (speed13 != newspeed13) {
    speed13 = newspeed13;
    motor13.drive(speed13);
  }
  if (speed24 != newspeed24) {
    speed24 = newspeed24;
    motor24.drive(speed24);
  }
}

void straight(float d) {
  dist = 0;
  clicksR = 0;
  clicksL = 0;
  cRrising = false;
  cLrising = false;
  float a = orientation, r = 1;
  while (dist < d) {
    readGyro();
    updateClicks();
    float newspeed13 = speed13, newspeed24 = speed24;
    float an = angle;
    if (orientation == 0 && an > 180) an -= 360;
    if (an > a + r) {
      newspeed24 = 185;
      newspeed13 = 135;
    }
    else if (an < a - r) {
      newspeed13 = 185;
      newspeed24 = 135;
    }
    else {
      newspeed13 = 135;
      newspeed24 = 135;
    }
    setSpeeds(newspeed13, newspeed24);
  }
  setSpeeds(0, 0);
  delay(100);
}

void left() {
  orientation -= 90;
  if (orientation < 0) orientation += 360;
  float t = millis(), a = orientation != 270 ? orientation : -90, r = 0.5;
  while (millis() - t < 1500) {
    readGyro();
    float newspeed13 = speed13, newspeed24 = speed24;
    float an = angle;
    if ((orientation == 270 || orientation == 0) && an > 180) an -= 360;
    if (an > a + r) {
      newspeed24 = 175;
      newspeed13 = -175;
    }
    else if (an < a - r) {
      newspeed24 = -125;
      newspeed13 = 125;
    }
    else {
      newspeed13 = 0;
      newspeed24 = 0;
    }
    setSpeeds(newspeed13, newspeed24);
  }
  setSpeeds(0, 0);
  delay(100);
}

void right() {
  orientation += 90;
  if (orientation >= 360) orientation = 0;
  float t = millis(), a = orientation, r = 0.5;
  while (millis() - t < 1500) {
    readGyro();
    float newspeed13 = speed13, newspeed24 = speed24;
    float an = angle;
    if ((orientation == 0 || orientation == 90) && an > 180) an -= 360;
    if (an > a + r) {
      newspeed24 = 175;
      newspeed13 = -175;
    }
    else if (an < a - r) {
      newspeed24 = -125;
      newspeed13 = 125;
    }
    else {
      newspeed13 = 0;
      newspeed24 = 0;
    }
    setSpeeds(newspeed13, newspeed24);
  }
  setSpeeds(0, 0);
  delay(100);
}

void calibrateGyro() {
  mpu.begin();
  Serial.println("beginned");
  mpu.Set_DMP_Output_Rate_Hz(200);
  Serial.println("hertzd");
  mpu.CalibrateMPU();
  Serial.println("calibrated");
  mpu.load_DMP_Image();
  Serial.println("imaged");
  mpu.on_FIFO(updateAngle);
  Serial.println("angled");
}

volatile int lastEncoded = 0; // Here updated value of encoder store.
volatile long encoderValue = 0; // Raw encoder value


void updateEncoder(){
  int MSB = digitalRead(12); //MSB = most significant bit
  int LSB = digitalRead(13); //LSB = least significant bit

  int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue --;
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue ++;

  lastEncoded = encoded; //store this value for next time
  //Serial.print("encoder: ");
  Serial.println(encoderValue);
}





void setup() {
  Serial.begin(115200);

  calibrateGyro();
  Serial.println("done stuff");

  motor24.drive(255);
  motor13.drive(255);
  
  
}


void loop() {
  //readGyro();
  updateEncoder();
}
