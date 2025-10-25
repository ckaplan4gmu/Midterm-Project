/*Generated using ChatGPT
Prompt: Functional requirements:
A. Scan: The sensor must scan a 180 degrees field of view in front of the “robot”
and then point to the closest object. (The object stays static.)
B. Idle: Once the “scan” is completed, the servo stops moving until the push button
is pressed.
C. A push button switches from “Idle” to “Scan” mode.
D. Indicate the mode using the color of RGB LED:
○ Scan = red
○ Idle = green

This is the final part, this part was built-in to the first snippet of code I gave you. just remove erroneous code*/
#include <Servo.h>  // Include Servo library

// -----------------------------
// Pin definitions
// -----------------------------
const int trigPin   = 8;    // Ultrasonic trigger pin
const int echoPin   = 7;    // Ultrasonic echo pin
const int servoPin  = 6;    // Servo control pin
const int buttonPin = 2;    // Push button pin
const int redPin    = 9;    // RGB LED red
const int greenPin  = 12;   // RGB LED green
const int bluePin   = 10;   // RGB LED blue

// -----------------------------
// Servo settings
// -----------------------------
Servo myServo;
const int minAngle = -40;   // Rightmost limit (≈ 0° physical)
const int maxAngle = 200;   // Leftmost limit (≈ 180° physical)
const int restAngle = 80;  // Neutral "center" position

// -----------------------------
// State variables
// -----------------------------
bool scanMode = false;        // false = Idle, true = Scan
bool lastButtonState = LOW;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// ---------------------------------------------------
// Helper: Get filtered distance (ignore 0 cm readings)
// ---------------------------------------------------
float getDistance() {
  long duration;
  float distance;
  float sum = 0;
  int validSamples = 0;   // count how many valid (non-zero) readings we have

  for (int i = 0; i < 5; i++) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    duration = pulseIn(echoPin, HIGH, 30000); // 30 ms timeout
    distance = duration * 0.034 / 2;          // convert to cm

    // ✅ Only include valid non-zero readings
    if (distance > 0) {
      sum += distance;
      validSamples++;
    }

    delay(10); // small delay between samples
  }

  // If no valid readings, return large distance to avoid false "close" detection
  if (validSamples == 0) {
    return 9999.0;
  }

  return sum / validSamples;
}

// ---------------------------------------------------
// Helper: Set RGB LED color
// ---------------------------------------------------
void setColor(bool red, bool green, bool blue) {
  digitalWrite(redPin, red);
  digitalWrite(greenPin, green);
  digitalWrite(bluePin, blue);
}

// ---------------------------------------------------
// Setup
// ---------------------------------------------------
void setup() {
  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  myServo.attach(servoPin);
  myServo.write(restAngle);

  setColor(LOW, HIGH, LOW);  // Green = Idle
  Serial.println("Mode: IDLE");
}

// ---------------------------------------------------
// Main loop
// ---------------------------------------------------
void loop() {
  // -----------------------------
  // Handle button input (toggle modes)
  // -----------------------------
  bool reading = !digitalRead(buttonPin); // active LOW
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  // debounce
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading && !scanMode) {
      scanMode = true;
      setColor(HIGH, LOW, LOW); // Red = Scan
      Serial.println("Mode: SCAN");
    } else if (reading && scanMode) {
      scanMode = false;
      setColor(LOW, HIGH, LOW); // Green = Idle
      myServo.write(restAngle);
      Serial.println("Mode: IDLE");
    }
  }
  lastButtonState = reading;

  // -----------------------------
  // IDLE MODE
  // -----------------------------
  if (!scanMode) return;

  // -----------------------------
  // SCAN MODE
  // -----------------------------
  float minDistance = 9999;
  int bestAngle = restAngle;

  // Sweep servo across full field of view
  for (int angle = minAngle; angle <= maxAngle; angle += 5) {
    myServo.write(angle);
    delay(80); // adjust for slower scanning

    float dist = getDistance();

    // Ignore invalid distances that slipped through
    if (dist <= 0) continue;

    Serial.print("Angle: ");
    Serial.print(angle);
    Serial.print("° | Distance: ");
    Serial.print(dist);
    Serial.println(" cm");

    if (dist < minDistance) {
      minDistance = dist;
      bestAngle = angle;
    }
  }

  // Point servo to closest object
  myServo.write(bestAngle);
  Serial.println("----------------------------------");
  Serial.print("Closest object at ");
  Serial.print(bestAngle);
  Serial.print("°, distance: ");
  Serial.print(minDistance);
  Serial.println(" cm");
  Serial.println("----------------------------------");

  // Return to Idle
  scanMode = false;
  setColor(LOW, HIGH, LOW); // Green = Idle
  Serial.println("Mode: IDLE");
}