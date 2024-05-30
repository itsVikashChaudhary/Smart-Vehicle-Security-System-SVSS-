

// Define the pins for the LDR and the LED
const int ldrPin = A0;
// const int ledPin = 13;
const int numLeds = 2;

// Define the pins for the ultrasonic sensor and the buzzer
const int trigPin = 12;
const int echoPin = 13;
const int buzzerPin = 9;

// Define the distance threshold and the beep duration
const int distanceThreshold = 30; // in cm
const int beepDuration = 100; // in ms

// Define variables for the LDR and the LED
int ldrValue;
int ledIntensity;

// Define variables for the ultrasonic sensor and the buzzer
long travelTime;
int distance;

void setup() {
  // Initialize the serial communication
  Serial.begin(9600);

  // Initialize the pins for the LDR and the LED
  pinMode(ldrPin, INPUT);
  // pinMode(ledPin, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);

  // Initialize the pins for the ultrasonic sensor and the buzzer
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  // Turn off the buzzer initially
  digitalWrite(buzzerPin, LOW);
}

void loop() {
  // Read the analog value from the LDR sensor
  ldrValue = analogRead(ldrPin);

  // Map the inverse of the LDR value to the LED intensity (0 - 255)
  ledIntensity = map(ldrValue, 0, 1023, 0, 255);

  //   // Set LED intensity based on LDR value ranges
  if (ldrValue > 700) {
    // High light intensity
    ledIntensity = 1000;  // Max intensity
  } else if (ldrValue > 400 && ldrValue < 700) {
    // Medium light intensity
    ledIntensity = 150;  // Medium intensity
   }
   else if (ldrValue > 320 && ldrValue <= 500) {
    // Low light intensity
    ledIntensity = 20;   // Low intensity
  } 
  else {
    ledIntensity = 0;    // No intensity for very low LDR values
  }
  // Print the LDR value and adjusted LED intensity
  Serial.print("LDR Value: ");
  Serial.println(ldrValue);

  // Set the built-in LED intensity
  // analogWrite(ledPin, ledIntensity);

  // Set the external LEDs intensity
  analogWrite(10, ledIntensity);
  analogWrite(11, ledIntensity);

  // Generate a 10-microsecond pulse on the TRIG pin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Measure the pulse duration on the ECHO pin
  travelTime = pulseIn(echoPin, HIGH);

  // Calculate the distance from the travel time
  distance = travelTime * 0.034 / 2;

  // Print the distance to the serial monitor
  Serial.print("Distance: ");
  Serial.println(distance);

  // Check if the distance is less than the distance threshold
  if (distance < distanceThreshold) {
    // Turn on the buzzer for the beep duration
    digitalWrite(buzzerPin, HIGH);
    delay(beepDuration);

    // Turn off the buzzer for the beep duration
    digitalWrite(buzzerPin, LOW);
    delay(beepDuration);

    // Check if the distance is less than half of the distance threshold
    if (distance < distanceThreshold / 2) {
      // Turn on the buzzer for the beep duration
      digitalWrite(buzzerPin, HIGH);
      delay(beepDuration);

      // Turn off the buzzer for the beep duration
      digitalWrite(buzzerPin, LOW);
      delay(beepDuration);
    }
  }

  // Add a small delay for stability and to reduce flicker
  delay(1000);
}










