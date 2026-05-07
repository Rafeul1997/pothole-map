#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <TinyGPS++.h>

// ---------------- WIFI ----------------
#define WIFI_SSID "MOTO_ARM"
#define WIFI_PASSWORD "phoneno."

// ---------------- FIREBASE ----------------
#define API_KEY "AIzaSyCOFZ8zjLw3S1UbG3R7w8OGEEaMxlyP8lg"
#define DATABASE_URL "https://potholedetection-a2a3d-default-rtdb.asia-southeast1.firebasedatabase.app/"

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;

// ---------------- GPS ----------------
TinyGPSPlus gps;
HardwareSerial gpsSerial(1);

// ---------------- SENSOR ----------------
#define GY61_PIN 34
float threshold = 0.1;

// ---------------- LED ----------------
#define GPS_LED 2
#define POTHOLE_LED 4
#define WIFI_LED 5

// ---------------- TIMING ----------------
unsigned long lastSendTime = 0;
int sendInterval = 2000;

unsigned long potholeLedTimer = 0;
bool potholeLedState = false;

// ---------------- JSON ----------------
FirebaseJson json;

// ---------------- WIFI RECONNECT ----------------
void reconnectWiFi() {
  if (WiFi.status() != WL_CONNECTED) {

    digitalWrite(WIFI_LED, LOW);

    Serial.println("🔄 Reconnecting WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    digitalWrite(WIFI_LED, HIGH);
    Serial.println("\n✅ WiFi Reconnected");
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(GY61_PIN, INPUT);

  pinMode(GPS_LED, OUTPUT);
  pinMode(POTHOLE_LED, OUTPUT);
  pinMode(WIFI_LED, OUTPUT);

  digitalWrite(GPS_LED, LOW);
  digitalWrite(POTHOLE_LED, LOW);
  digitalWrite(WIFI_LED, LOW);

  // GPS (RX=16, TX=17)
  gpsSerial.begin(9600, SERIAL_8N1, 16, 17);

  // WiFi connect
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi Connected");
  digitalWrite(WIFI_LED, HIGH);

  // Firebase setup
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("🔥 Firebase Ready");
    signupOK = true;
  } else {
    Serial.printf("❌ Signup error: %s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // SSL fix
  fbdo.setBSSLBufferSize(4096, 1024);
}

void loop() {

  // -------- WIFI LED STATUS --------
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(WIFI_LED, HIGH);
  } else {
    digitalWrite(WIFI_LED, LOW);
  }

  // -------- GPS READ --------
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  bool gpsAvailable = gps.location.isValid() && gps.location.age() < 2000;

  // -------- GPS LED --------
  digitalWrite(GPS_LED, gpsAvailable ? HIGH : LOW);

  // -------- SENSOR READ --------
  int rawValue = analogRead(GY61_PIN);
  float voltage = rawValue * (3.3 / 4095.0);

  static float prevVoltage = 0;
  float zChange = voltage - prevVoltage;
  prevVoltage = voltage;

  Serial.print("Z Change: ");
  Serial.println(zChange);

  // -------- POTHOLE DETECTION --------
  if (abs(zChange) > threshold) {

    Serial.println("🚧 POTHOLE DETECTED!");

    // LED ON
    digitalWrite(POTHOLE_LED, HIGH);
    potholeLedTimer = millis();
    potholeLedState = true;

    if (gpsAvailable) {
      Serial.println("📍 GPS AVAILABLE");
    } else {
      Serial.println("⚠️ GPS NOT AVAILABLE");
    }

    // -------- SEND WITH COOLDOWN --------
    if (millis() - lastSendTime > sendInterval) {

      reconnectWiFi();

      if (Firebase.ready() && signupOK) {

        if (gpsAvailable) {

          Serial.println("🚧 Sending Data...");

          json.clear();
          json.set("zChange", zChange);
          json.set("timestamp", millis());
          json.set("lat", gps.location.lat());
          json.set("lng", gps.location.lng());

          if (Firebase.RTDB.pushJSON(&fbdo, "/potholes", &json)) {
            Serial.println("✅ Uploaded Successfully");
          } else {
            Serial.print("❌ Firebase Error: ");
            Serial.println(fbdo.errorReason());
          }

          lastSendTime = millis();

        } else {
          Serial.println("⛔ Upload skipped (GPS not available)");
        }
      } else {
        Serial.println("⚠️ Firebase not ready");
      }
    }
  }

  // -------- POTHOLE LED AUTO OFF --------
  if (potholeLedState && millis() - potholeLedTimer > 1000) {
    digitalWrite(POTHOLE_LED, LOW);
    potholeLedState = false;
  }

  delay(200);
}