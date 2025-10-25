/*Generated using ChatGPT  Prompt: Functional requirements: 
A. Using a servo motor, the sensor should collect distance to the nearest object in different 
directions (e.g., front, right, and left). Anything closer than 20 cm should be detected as an obstacle / wall. 
B. The following situations should be distinguished: a. “Facing wall” b. “Wall on the left” c. “Wall on the right” d. “Corner / Dead end” e. “Corridor” 
f. “Open space” C. The system should continuously update the estimated situation (at least every 2 seconds) based on the sensor measurement and display it in the serial monitor. This is part 2, this is mostly built-in to the code I gave you already, I just need you to remove erroneous code for this part. Additionally, We just want the servo to continuously swing back and forth between the upper and lower bounds set as variables for the servo. 
It should then give a final assessment of the "environment" being updated 
continuously after each 180 of the servo. Comment what each block of code does*/

#include <Servo.h>  // Include Servo library

// -----------------------------
// Pin definitions
// -----------------------------
const int trigPin  = 8;   // Ultrasonic trigger pin
const int echoPin  = 7;   // Ultrasonic echo pin
const int servoPin = 6;   // Servo control pin

// -----------------------------
// Servo settings
// -----------------------------
Servo myServo;
const int minAngle = -40;   // Rightmost limit (approx 0° physical)
const int maxAngle = 200;   // Leftmost limit (approx 180° physical)
int currentAngle = minAngle;
int step = 10;              // Step size per movement (smaller = smoother)
bool sweepingForward = true; // Sweep direction flag

// -----------------------------
// Distance readings for classification
// -----------------------------
float dLeft = 0;
float dCenter = 0;
float dRight = 0;

// ---------------------------------------------------
// Measure distance (averages 5 valid samples, ignores 0 cm)
// ---------------------------------------------------
float getDistance() {
  long duration;
  float distance;
  float sum = 0;
  int validSamples = 0;

  for (int i = 0; i < 5; i++) {  // take 5 samples
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // measure echo pulse (timeout = 30 ms)
    duration = pulseIn(echoPin, HIGH, 30000);
    distance = duration * 0.034 / 2;  // convert to cm

    // ✅ ignore invalid 0 cm readings
    if (distance > 0) {
      sum += distance;
      validSamples++;
    }
    delay(10);
  }

  // if no valid readings, return large dummy value
  if (validSamples == 0) return 9999.0;

  // return average of valid readings
  return sum / validSamples;
}

// ---------------------------------------------------
// Classify environment based on 3 key distances
// ---------------------------------------------------
String classifyEnvironment(float dLeft, float dCenter, float dRight) {
  const float wallThreshold = 20.0;  // closer than 20 cm = wall

  if (dCenter < wallThreshold && dLeft < wallThreshold && dRight < wallThreshold)
    return "Corner / Dead end";
  else if (dCenter < wallThreshold && dLeft > wallThreshold && dRight > wallThreshold)
    return "Facing wall";
  else if (dLeft < wallThreshold && dCenter > wallThreshold && dRight > wallThreshold)
    return "Wall on left";
  else if (dRight < wallThreshold && dCenter > wallThreshold && dLeft > wallThreshold)
    return "Wall on right";
  else if (dLeft < wallThreshold && dRight < wallThreshold && dCenter > wallThreshold)
    return "Corridor";
  else
    return "Open space";
}

// ---------------------------------------------------
// Setup: initialize components
// ---------------------------------------------------
void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  myServo.attach(servoPin);
  myServo.write(currentAngle);

  Serial.println("System Initialized: Beginning continuous scanning...");
}

// ---------------------------------------------------
// Main loop: sweep servo, measure distances, classify environment
// ---------------------------------------------------
void loop() {
  // --- Move servo gradually ---
  myServo.write(currentAngle);
  delay(50);  // adjust for slower sweep speed (was 10)

  // --- Get filtered distance at current angle ---
  float distance = getDistance();

  // --- Save readings near left, center, and right positions ---
  if (currentAngle <= minAngle + 5) dRight = distance;  // near rightmost
  if (currentAngle >= (minAngle + maxAngle) / 2 - 5 &&
      currentAngle <= (minAngle + maxAngle) / 2 + 5) dCenter = distance; // near center
  if (currentAngle >= maxAngle - 5) dLeft = distance;   // near leftmost

  // --- Change direction and classify after each full sweep ---
  if (sweepingForward) {
    currentAngle += step;
    if (currentAngle >= maxAngle) {
      sweepingForward = false; // reached left end

      // ✅ classify environment after leftward sweep
      String environment = classifyEnvironment(dLeft, dCenter, dRight);
      Serial.println("----------------------------------");
      Serial.print("Left: ");   Serial.print(dLeft);   Serial.print(" cm | ");
      Serial.print("Center: "); Serial.print(dCenter); Serial.print(" cm | ");
      Serial.print("Right: ");  Serial.print(dRight);  Serial.println(" cm");
      Serial.print("Environment: ");
      Serial.println(environment);
      Serial.println("----------------------------------");
    }
  } else {
    currentAngle -= step;
    if (currentAngle <= minAngle) {
      sweepingForward = true; // reached right end

      // ✅ classify environment after rightward sweep
      String environment = classifyEnvironment(dLeft, dCenter, dRight);
      Serial.println("----------------------------------");
      Serial.print("Left: ");   Serial.print(dLeft);   Serial.print(" cm | ");
      Serial.print("Center: "); Serial.print(dCenter); Serial.print(" cm | ");
      Serial.print("Right: ");  Serial.print(dRight);  Serial.println(" cm");
      Serial.print("Environment: ");
      Serial.println(environment);
      Serial.println("----------------------------------");
    }
  }
}