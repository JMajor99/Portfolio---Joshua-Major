#include <PS4Controller.h>
#include <ps4.h>
#include <ps4_int.h>
#include <PS4Controller.h>
#include <ESP32Servo.h>

Servo myServo;
int servoPin = 4;
int pwmPin = 14;
int angle = 75;
int debug;
int pwmChannel = 0;
int frequence = 500;
int resolution = 8;
int reversePin = 18;
int drivePin = 19;

unsigned long lastTimeStamp = 0;

void notify()
{
  char messageString[200];
  sprintf(messageString, "%4d,%4d,%4d,%4d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d",
  PS4.LStickX(),
  PS4.LStickY(),
  PS4.RStickX(),
  PS4.RStickY(),
  PS4.Left(),
  PS4.Down(),
  PS4.Right(),
  PS4.Up(),
  PS4.Square(),
  PS4.Cross(),
  PS4.Circle(),
  PS4.Triangle(),
  PS4.L1(),
  PS4.R1(),
  PS4.L2(),
  PS4.R2(),  
  PS4.Share(),
  PS4.Options(),
  PS4.PSButton(),
  PS4.Touchpad(),
  PS4.Charging(),
  PS4.Audio(),
  PS4.Mic(),
  PS4.Battery());

  //Only needed to print the message properly on serial monitor. Else we dont need it.
  if (millis() - lastTimeStamp > 50)
  {
    Serial.println(messageString);
    lastTimeStamp = millis();
  }
}

void onConnect()
{
  Serial.println("Connected!.");
}

void onDisConnect()
{
  Serial.println("Disconnected!.");    
}

void setup() 
{
  Serial.begin(115200);
  PS4.attach(notify);
  PS4.attachOnConnect(onConnect);
  PS4.attachOnDisconnect(onDisConnect);
  PS4.begin();
  Serial.println("Ready.");
  //ledcAttach(pwmPin, frequence, resolution);
  //pinMode(pwmPin, OUTPUT);
  // if you want to attach a specific channel, use the following instead
  ledcAttachChannel(pwmPin, frequence, resolution, 2);
  ledcAttachChannel(reversePin, frequence, resolution, 3);
  ledcAttachChannel(drivePin, frequence, resolution, 4);
  myServo.attach(servoPin);
  myServo.write(angle);
}

void loop() 
{
  myServo.write(angle + ((-1 * PS4.LStickX()) >> 1));
  int acceleration = map(PS4.RStickY(), -128, 127, -255, 255);
  if(acceleration > 10)
  {
    analogWrite(pwmPin, acceleration);
    analogWrite(drivePin, acceleration);
  }
  else if (acceleration < -10)
  {
    analogWrite(reversePin, abs(acceleration));
    analogWrite(pwmPin, abs(acceleration));
  }
  else 
  {
    analogWrite(pwmPin, 0);
    analogWrite(reversePin, 0);
    analogWrite(drivePin, 0);
  }
  //Serial.println(abs(acceleration));
  delay(20);
}
