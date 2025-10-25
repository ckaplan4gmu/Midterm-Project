//ChatGPT was used to generate this code
//Prompt used: #include <Servo.h>

// -----------------------------
// Pin definitions
// -----------------------------
/*const int trigPin   = 8;
const int echoPin   = 7;
const int servoPin  = 6;
const int buttonPin = 2;
const int buzzerPin = 11;
const int redPin    = 9;
const int greenPin  = 12;
const int bluePin   = 10;

// -----------------------------
// Servo settings
// -----------------------------
Servo myServo;
const int restAngle = 100;   // forward direction
const int minScan   = -140;    // scan start angle
const int maxScan   = 220;  // scan end angle

// -----------------------------
// State variables
// -----------------------------
bool scanMode = false;
bool lastButtonState = LOW;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

// -----------------------------
// Distance and buzzer timing
// -----------------------------
unsigned long lastDistancePrint = 0;
const unsigned long distanceInterval = 250; // ms between prints
unsigned long lastBeepTime = 0;
bool beeping = false;
int beepInterval = 0;
int toneFreq = 250;

// -----------------------------
// Helpers
// -----------------------------
float getDistance() {
  long duration;
  float distance;
  float sum = 0;
  for (int i = 0; i < 5; i++) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH, 30000);
    distance = duration * 0.034 / 2;
    sum += distance;
  }
  return sum / 5.0;
}

void setColor(bool red, bool green, bool blue) {
  digitalWrite(redPin, red);
  digitalWrite(greenPin, green);
  digitalWrite(bluePin, blue);
}

void proximityFeedback(float distance) {
  if (distance < 5.0) {                // CLOSE
    setColor(HIGH, LOW, LOW);          // Red flashes if you want
    beepInterval = 120;
  } else if (distance <= 20.0) {       // MID-RANGE
    setColor(HIGH, HIGH, LOW);         // Yellow
    beepInterval = 350;
  } else {                             // FAR
    setColor(LOW, HIGH, LOW);          // Green
    beepInterval = 0;
    noTone(buzzerPin);
  }

  if (beepInterval > 0) {
    unsigned long now = millis();
    if (now - lastBeepTime >= beepInterval) {
      lastBeepTime = now;
      if (!beeping) {
        tone(buzzerPin, toneFreq);
        beeping = true;
      } else {
        noTone(buzzerPin);
        beeping = false;
      }
    }
  }
}

// ---------------------------------------------------
// Environment classification
// ---------------------------------------------------
String classifyEnvironment(float dLeft, float dCenter, float dRight) {
  const float wallThreshold = 20.0;
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
  pinMode(buzzerPin, OUTPUT);

  myServo.attach(servoPin);
  myServo.write(restAngle);
  setColor(LOW, HIGH, LOW); // Idle = green
  Serial.println("Mode: IDLE");
}

// ---------------------------------------------------
// Main loop
// ---------------------------------------------------
// -----------------------------
// Idle mode timing
// -----------------------------
String lastEnvironment = "Unknown"; // stores environment after last scan

void loop() {
  // --- Button handling ---
  bool reading = !digitalRead(buttonPin);
  if (reading != lastButtonState) lastDebounceTime = millis();

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading && !scanMode) {
      scanMode = true;
      Serial.println("Mode: SCAN");
      setColor(HIGH, LOW, LOW); // Red
      noTone(buzzerPin);
    } else if (reading && scanMode) {
      scanMode = false;
      Serial.println("Mode: IDLE");
      setColor(LOW, HIGH, LOW); // Green
      noTone(buzzerPin);
    }
  }
  lastButtonState = reading;

  // --- IDLE MODE ---
  if (!scanMode) {
    float distance = getDistance();
    proximityFeedback(distance);

    // Serial output every 250ms
    if (millis() - lastDistancePrint >= distanceInterval) {
      lastDistancePrint = millis();

      String proximity;
      if (distance < 5.0) proximity = "Close";
      else if (distance <= 20.0) proximity = "Mid-range";
      else proximity = "Far";

      Serial.print("Distance: ");
      Serial.print(distance);
      Serial.print(" cm  |  Proximity: ");
      Serial.print(proximity);
      Serial.print("  | Environment: ");
      Serial.println(lastEnvironment); // print last scanned environment
    }
  }

  // --- SCAN MODE ---
  if (scanMode) {
    float minDistance = 9999;
    int bestAngle = restAngle;
    float dLeft, dCenter, dRight;

    // Measure Right, Center, Left
    myServo.write(minScan); delay(250); dRight = getDistance();
    myServo.write(restAngle); delay(250); dCenter = getDistance();
    myServo.write(maxScan); delay(250); dLeft = getDistance();

    // Sweep full 180° to find closest object
    for (int angle = minScan; angle <= maxScan; angle += 5) {
      myServo.write(angle);
      delay(100);
      float dist = getDistance();
      Serial.print("Angle: "); Serial.print(angle);
      Serial.print("  Distance: "); Serial.println(dist);
      if (dist < minDistance) {
        minDistance = dist;
        bestAngle = angle;
      }
    }

    // Point servo to closest object
    myServo.write(bestAngle);
    Serial.print("Closest object at ");
    Serial.print(bestAngle);
    Serial.print("°, distance: ");
    Serial.print(minDistance);
    Serial.println(" cm");

    // Classify environment and store it
    lastEnvironment = classifyEnvironment(dLeft, dCenter, dRight);
    Serial.print("Environment: ");
    Serial.println(lastEnvironment);

    // Return to idle
    scanMode = false;
    setColor(LOW, HIGH, LOW); // Green
    Serial.println("Mode: IDLE");
  }
}

//Snippet of code for an ultrasonic sensor mounted onto a servo, does a lot of things. 

//First part of project:
//The Ultrasonic sensor does not need to move for this part.
//Functional requirements:
//A. The distance to the object should be continuously sensed and displayed in the serial
//monitor.
//B. There should be at least three categories in the level of proximity/risk:
//a. “Close”: less than 5 cm
//b. “Mid-range”: between 5 cm and 20 cm
//c. “Far”: beyond 20 cm
//C. The color of the RGB LED should be
//a. “Close” → Red
//b. “Mid-range” → Yellow
//c. “Far” → Green
//D. There should be a beeping sound with the following characteristics:
//a. “Close” → High frequency of beeping to alert the user
//b. “Mid-range” → Low frequency of beeping
//c. “Far” → No beeping
//*Note: high frequency does not mean high pitch.
//E. Since the ultrasonic sensor is noisy, there should be some type of “filtering” to avoid
//erroneous measurements (e.g., take the average of five samples). Write a separate
//method for this so that you can use it in the subsequent parts

//Now this is already built-in to the code I gave you, I just need you to eliminate erroneous code, or code that moves the servo around and such. Just strip it down to the beeping, RGB LED light. Also, write out explanations for all code blocks */
//**** NOTE: I previously generated code for all 3 parts combined, was just asking it to strip this code for each part.****
#include <Arduino.h>

// -----------------------------
// Pin definitions
// -----------------------------
const int trigPin   = 8;
const int echoPin   = 7;
const int buzzerPin = 11;
const int redPin    = 9;
const int greenPin  = 12;
const int bluePin   = 10;

// -----------------------------
// Distance and buzzer timing
// -----------------------------
unsigned long lastDistancePrint = 0;
const unsigned long distanceInterval = 250; // ms between prints
unsigned long lastBeepTime = 0;
bool beeping = false;
int beepInterval = 0;
int toneFreq = 250;

// ---------------------------------------------------
// Helper function: Measure distance (with filtering)
// ---------------------------------------------------
float getDistance() {
  long duration;
  float distance;
  float sum = 0;

  // Take 5 readings and average them to reduce noise
  for (int i = 0; i < 5; i++) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH, 30000); // timeout: 30ms (~5m)
    distance = duration * 0.034 / 2;          // convert to cm
    sum += distance;
  }
  return sum / 5.0; // averaged distance
}

// ---------------------------------------------------
// Helper function: Set RGB color
// ---------------------------------------------------
void setColor(bool red, bool green, bool blue) {
  digitalWrite(redPin, red);
  digitalWrite(greenPin, green);
  digitalWrite(bluePin, blue);
}

// ---------------------------------------------------
// Proximity feedback: LED + Buzzer
// ---------------------------------------------------
void proximityFeedback(float distance) {
  if (distance < 5.0) {                // CLOSE
    setColor(HIGH, LOW, LOW);          // Red
    beepInterval = 120;                // fast beeping
  } else if (distance <= 20.0) {       // MID-RANGE
    setColor(HIGH, HIGH, LOW);         // Yellow
    beepInterval = 350;                // slower beeping
  } else {                             // FAR
    setColor(LOW, HIGH, LOW);          // Green
    beepInterval = 0;                  // no beeping
    noTone(buzzerPin);
  }

  // Handle beeping timing
  if (beepInterval > 0) {
    unsigned long now = millis();
    if (now - lastBeepTime >= beepInterval) {
      lastBeepTime = now;
      if (!beeping) {
        tone(buzzerPin, toneFreq);
        beeping = true;
      } else {
        noTone(buzzerPin);
        beeping = false;
      }
    }
  }
}

// ---------------------------------------------------
// Setup
// ---------------------------------------------------
void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  setColor(LOW, HIGH, LOW); // Start with Green (idle)
  Serial.println("Ultrasonic Distance Sensor - Active");
}

// ---------------------------------------------------
// Main loop
// ---------------------------------------------------
void loop() {
  float distance = getDistance();
  proximityFeedback(distance);

  // Print readings every 250ms
  if (millis() - lastDistancePrint >= distanceInterval) {
    lastDistancePrint = millis();

    String proximity;
    if (distance < 5.0) proximity = "Close";
    else if (distance <= 20.0) proximity = "Mid-range";
    else proximity = "Far";

    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.print(" cm  |  Proximity: ");
    Serial.println(proximity);
  }
}
