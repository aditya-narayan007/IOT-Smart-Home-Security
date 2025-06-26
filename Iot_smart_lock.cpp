// Template ID, Device Name and Auth Token are provided by the Blynk.Cloud
#define BLYNK_TEMPLATE_ID "TMPL3lCgofpsa"
#define BLYNK_TEMPLATE_NAME "IOT smart door lock"
#define BLYNK_AUTH_TOKEN "uGN9amLkdwYBlOMV4hRyY2wHvxL0BQfu"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED config
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// WiFi credentials
char ssid[] = "Wifi_Name";
char pass[] = "Wifi_Paddword";
char auth[] = "Auth Id";

// RFID Pins
#define RST_PIN 43
#define SS_PIN 44
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

// Motor driver pins (update if needed based on your board!)
int enA = 1;
int in1 = 2;
int in2 = 3;

// Student card UIDs
byte ADITYA[] = {0x04, 0xB5, 0x6C, 0x8E};
byte PRINCE[] = {0x18, 0x77, 0x96, 0x8A};
byte RAKESH[] = {0x28, 0x58, 0x28, 0x8A};
byte SHARK[]  = {0x18, 0xAC, 0x77, 0x8A};

// Button states from Blynk
bool ADITYA_enabled = false;
bool PRINCE_enabled = false;
bool RAKESH_enabled = false;
bool SHARK_enabled = false;

// Virtual pins
#define STUDENT_A_BUTTON V3
#define STUDENT_B_BUTTON V4
#define STUDENT_C_BUTTON V5
#define STUDENT_D_BUTTON V6

// Blynk button handlers
BLYNK_WRITE(STUDENT_A_BUTTON) {
  ADITYA_enabled = param.asInt();
  Blynk.virtualWrite(V2, ADITYA_enabled ? "Student A: Enabled" : "Student A: Disabled");
}

BLYNK_WRITE(STUDENT_B_BUTTON) {
  PRINCE_enabled = param.asInt();
  Blynk.virtualWrite(V2, PRINCE_enabled ? "Student B: Enabled" : "Student B: Disabled");
}

BLYNK_WRITE(STUDENT_C_BUTTON) {
  RAKESH_enabled = param.asInt();
  Blynk.virtualWrite(V2, RAKESH_enabled ? "Student C: Enabled" : "Student C: Disabled");
}

BLYNK_WRITE(STUDENT_D_BUTTON) {
  SHARK_enabled = param.asInt();
  Blynk.virtualWrite(V2, SHARK_enabled ? "Student D: Enabled" : "Student D: Disabled");
}

void setup(void) {
  Serial.begin(115200);
  Serial.println("Initializing...");

  // OLED setup
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Initializing...");
  display.display();
  delay(1000);

  // RFID + Motor setup
  SPI.begin();
  mfrc522.PCD_Init();
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  analogWrite(enA, 255);
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  mfrc522.PCD_DumpVersionToSerial();

  Blynk.begin(auth, ssid, pass);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Waiting for RFID...");
  display.display();
}

bool compareUID(byte *uid, byte *storedUID, byte length = 4) {
  for (byte i = 0; i < length; i++) {
    if (uid[i] != storedUID[i]) {
      return false;
    }
  }
  return true;
}

void unlockDoor(String studentName) {
  Serial.println("Access granted for " + studentName);
  Blynk.virtualWrite(V2, "Last Access: " + studentName);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Access Granted");
  display.println(studentName);
  display.display();

  // Unlock door (run motor)
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  delay(5000); // Hold 5 seconds
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Waiting for RFID...");
  display.display();
}

void denyAccess() {
  Serial.println("Access denied!");
  Blynk.virtualWrite(V2, "Last Attempt: Denied");

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Access Denied!");
  display.display();
  delay(2000);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Waiting for RFID...");
  display.display();
}

void loop(void) {
  Blynk.run();

  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  Serial.println("Card detected");
  Serial.print("UID Length: ");
  Serial.print(mfrc522.uid.size, DEC);
  Serial.println(" bytes");
  Serial.print("UID Value: ");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(" 0x");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();

  if (compareUID(mfrc522.uid.uidByte, ADITYA) && ADITYA_enabled) {
    unlockDoor("Student A");
  } else if (compareUID(mfrc522.uid.uidByte, PRINCE) && PRINCE_enabled) {
    unlockDoor("Student B");
  } else if (compareUID(mfrc522.uid.uidByte, RAKESH) && RAKESH_enabled) {
    unlockDoor("Student C");
  } else if (compareUID(mfrc522.uid.uidByte, SHARK) && SHARK_enabled) {
    unlockDoor("Student D");
  } else {
    denyAccess();
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  delay(500);
}