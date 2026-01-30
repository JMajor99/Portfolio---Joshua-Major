#include <Stepper.h>

// 17 rotations from start to end to fully lift scissor design

int stepsPerRevolution = 2048;  // 28BYJ-48 = 2048 steps/rev
int rpm = 20;
Stepper myStepper(stepsPerRevolution, 4, 18, 5, 19);

String inputString = "";
boolean inputComplete = false;

void setup() {
  Serial.begin(115200);   // open serial monitor at 115200 baud
  myStepper.setSpeed(rpm);

  Serial.println("Enter: <rotations> <direction>");
  Serial.println("Example: 5 1   --> 5 rotations clockwise");
  Serial.println("Example: 3 -1  --> 3 rotations counterclockwise");
}

void loop() {
  if (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\n') { // end of line
      inputComplete = true;
    } else {
      inputString += c;
    }
  }

  if (inputComplete) {
    int rotations = 0;
    int direction = 1;

    // parse input
    sscanf(inputString.c_str(), "%d %d", &rotations, &direction);

    Serial.print("Moving ");
    Serial.print(rotations);
    Serial.print(" rotations in direction ");
    Serial.println(direction);

    long stepsToMove = (long)stepsPerRevolution * rotations * direction;
    myStepper.step(stepsToMove);

    // reset
    inputString = "";
    inputComplete = false;
    Serial.println("Done. Enter next command:");
  }
}
