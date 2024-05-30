#include <AltSoftSerial.h>
#include <TinyGPS++.h>
#include <Wire.h>
#include <SoftwareSerial.h>

const String EMERGENCY_PHONE = "+91123456789";
#define rxPin 2
#define txPin 3

SoftwareSerial sim800(rxPin, txPin);
AltSoftSerial neogps;
TinyGPSPlus gps;

String latitude, longitude;
#define BUTTON 7

boolean impact_detected = false;
unsigned long impact_time;
unsigned long alert_delay = 30000;

// Define the pins for the LDR and the LED
const int ldrPin = A0;
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
  Serial.begin(9600);
  sim800.begin(9600);
  neogps.begin(9600);

  pinMode(BUTTON, INPUT_PULLUP);

  sim800.println("AT");
  delay(1000);
  sim800.println("ATE1");
  delay(1000);
  sim800.println("AT+CPIN?");
  delay(1000);
  sim800.println("AT+CMGF=1");
  delay(1000);
  sim800.println("AT+CNMI=1,1,0,0,0");
  delay(1000);

  // Initialize the pins for the LDR and the LED
  pinMode(ldrPin, INPUT);
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
  if (digitalRead(BUTTON) == LOW) {
    delay(200);
    impact_detected = true;
    impact_time = millis();
    getGps();
    makeCall();
    delay(500);
    sendAlert();
    
  }

  while (sim800.available()) {
    parseData(sim800.readString());
  }

  while (Serial.available()) {
    sim800.println(Serial.readString());
  }

  // Read the analog value from the LDR sensor
  ldrValue = analogRead(ldrPin);

  // Map the inverse of the LDR value to the LED intensity (0 - 255)
  ledIntensity = map(ldrValue, 0, 1023, 0, 255);

  // Set LED intensity based on LDR value ranges
  if (ldrValue > 700) {
    // High light intensity
    ledIntensity = 1000;  // Max intensity
  } else if (ldrValue > 400 && ldrValue < 700) {
    // Medium light intensity
    ledIntensity = 150;  // Medium intensity
  } else if (ldrValue > 320 && ldrValue <= 500) {
    // Low light intensity
    ledIntensity = 20;   // Low intensity
  } else {
    ledIntensity = 0;    // No intensity for very low LDR values
  }

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

void parseData(String buff) {
  Serial.println(buff);
  unsigned int len, index;
  index = buff.indexOf("\r");
  buff.remove(0, index + 2);
  buff.trim();

  if (buff != "OK") {
    index = buff.indexOf(":");
    String cmd = buff.substring(0, index);
    cmd.trim();
    buff.remove(0, index + 2);

    if (cmd == "+CMTI") {
      index = buff.indexOf(",");
      String temp = buff.substring(index + 1, buff.length());
      temp = "AT+CMGR=" + temp + "\r";
      sim800.println(temp);
    }
    else if (cmd == "+CMGR") {
      if (buff.indexOf(EMERGENCY_PHONE) > 1) {
        buff.toLowerCase();
        if (buff.indexOf("get gps") > 1) {
          getGps();
          String sms_data;
          sms_data = "GPS Location Data\r";
          sms_data += "http://maps.google.com/maps?q=loc:";
          sms_data += latitude + "," + longitude;
          sendSms(sms_data);
        }
      }
    }
  }
}

void getGps() {
  boolean newData = false;
  for (unsigned long start = millis(); millis() - start < 2000;) {
    while (neogps.available()) {
      if (gps.encode(neogps.read())) {
        newData = true;
        break;
      }
    }
  }

  if (newData) {
    latitude = String(gps.location.lat(), 6);
    longitude = String(gps.location.lng(), 6);
    newData = false;
  }
  else {
    Serial.println("No GPS data is available");
    latitude = "30.2724";
    longitude = "78.0010";
  }

  Serial.print("Latitude= "); Serial.println(latitude);
  Serial.print("Longitude= "); Serial.println(longitude);
}

void sendAlert() {
  String sms_data;
  sms_data = "Accident Alert!!\r";
  sms_data += "http://maps.google.com/maps?q=loc:";
  sms_data += latitude + "," + longitude;
  sendSms(sms_data);
}

void makeCall() {
  Serial.println("calling....");
  sim800.println("ATD" + EMERGENCY_PHONE + ";");
  delay(20000); //20 sec delay
  sim800.println("ATH");
  delay(1000); //1 sec delay
}

void sendSms(String text) {
  sim800.println("AT+CMGF=1"); // Set SMS mode to text mode
  delay(100);
  sim800.println("AT+CMGS=\"" + EMERGENCY_PHONE + "\""); // Specify recipient number
  delay(100);
  sim800.println(text); // Send the SMS text
  delay(100);
  sim800.write(0x1A); // Send CTRL+Z to indicate the end of the message
  delay(1000); // Give some time for the module to send the message
  Serial.println("SMS Sent Successfully.");
}












