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

String sms_status, sender_number, received_date, msg;
String latitude, longitude;
#define BUTTON 11

boolean impact_detected = false;
unsigned long impact_time;
unsigned long alert_delay = 30000;

void setup() {
  Serial.begin(9600);
  sim800.begin(9600);
  neogps.begin(9600);

  pinMode(BUTTON, INPUT_PULLUP);

  sms_status = "";
  sender_number = "";
  received_date = "";
  msg = "";

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
    impact_detected = false;
    impact_time = 0;
  }

  while (sim800.available()) {
    parseData(sim800.readString());
  }

  while (Serial.available()) {
    sim800.println(Serial.readString());
  }
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
  sim800.print("AT+CMGF=1\r");
  delay(200);
  sim800.print("AT+CMGS=\"" + EMERGENCY_PHONE + "\"\r");
  delay(200);
  sim800.print(text);
  delay(50);
  sim800.write(0x1A);
  delay(500);
  Serial.println("SMS Sent Successfully.");

}













