const int ldrPin = A0;     // Analog pin for LDR sensor
const int ledPin = 13;      // Pin for the built-in LED on Arduino
const int numLeds = 2;     // Number of LEDs

void setup() {
  Serial.begin(9600);       // Initialize serial communication
  pinMode(ldrPin, INPUT);
  pinMode(ledPin, OUTPUT);

  // Assuming LEDs are connected to pins 10 and 11
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
}

void loop() {
  // Read the analog value from the LDR sensor
  int ldrValue = analogRead(ldrPin);

  // Map the inverse of the LDR value to the LED intensity (0 - 255)
  int ledIntensity = map(ldrValue, 0, 1023, 0, 255);

  // Set LED intensity based on LDR value ranges
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
  analogWrite(ledPin, ledIntensity);

  // Set the external LEDs intensity
  analogWrite(10, ledIntensity);
  analogWrite(11, ledIntensity);

  // Delay for stability and to reduce flicker
  delay(500);
}



