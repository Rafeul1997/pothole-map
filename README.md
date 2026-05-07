# 🚧 ESP32 Smart Pothole Detection System

An IoT-based pothole detection system using ESP32, GPS, accelerometer sensor, and Firebase Realtime Database.

The system detects sudden road surface changes using a vibration/tilt sensor and uploads pothole locations with GPS coordinates to Firebase in real time.

---

## 📌 Features

- 📡 Real-time pothole detection
- 🌍 GPS location tracking
- ☁️ Firebase Realtime Database integration
- 📶 Automatic WiFi reconnect
- 🚨 LED status indicators
- ⚡ ESP32 based low-cost system

---

## 🛠 Hardware Used

- ESP32 Development Board
- GY-61 Accelerometer Module
- NEO-6M GPS Module
- LEDs
- Jumper wires
- Power supply / battery

---

## 🔌 Pin Connections

| Component | ESP32 Pin |
|---|---|
| GY-61 Output | GPIO 34 |
| GPS RX | GPIO 16 |
| GPS TX | GPIO 17 |
| GPS LED | GPIO 2 |
| Pothole LED | GPIO 4 |
| WiFi LED | GPIO 5 |

---

## 📚 Libraries Required

Install these libraries from Arduino IDE Library Manager:

- WiFi
- Firebase ESP Client
- TinyGPS++

---

## ⚙️ Working Principle

1. ESP32 continuously reads accelerometer values.
2. Sudden Z-axis changes indicate potholes or bumps.
3. GPS module provides live coordinates.
4. Detected pothole data is uploaded to Firebase.
5. LEDs indicate GPS, WiFi, and pothole status.

---

## 📂 Data Uploaded to Firebase

```json
{
  "zChange": 0.45,
  "timestamp": 123456,
  "lat": 22.5726,
  "lng": 88.3639
}
