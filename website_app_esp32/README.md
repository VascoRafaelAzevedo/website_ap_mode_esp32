# ESP32 Website Access Point Mode

This project implements a web server on ESP32 that works in Access Point (AP) mode, allowing direct access to the device without needing a WiFi router.

## Features

- ✅ ESP32 configured as Access Point
- ✅ HTTP web server on port 80
- ✅ Responsive website showing variable value
- ✅ Auto-refresh every 15 seconds
- ✅ Modern responsive design interface
- ✅ Real-time system information

## How to Use

### 1. Connect to Access Point
1. After upload, the ESP32 will create an Access Point
2. On your device (PC/smartphone), look for the WiFi network:
   - **SSID:** `ESP32-Access-Point`
   - **Password:** `12345678`
3. Connect to the network

### 2. Access the Website
- Open your browser and go to: `http://192.168.4.1`
- The website will show the current value of the `sensor_value` variable

## Code Structure

- `setupAccessPoint()` - Configures ESP32 as AP
- `setupWebServer()` - Starts the web server
- `handleRoot()` - Responds to main page requests
- `generateHTML()` - Generates the HTML page
- `loop()` - Updates variable and processes requests

## Dynamic Values Example

The code includes an example that simulates variable changes every 5 seconds:
```cpp
// In loop()
if (millis() - lastUpdate > 5000) {
    sensor_value += random(-10, 11) / 10.0; // Varies ±1.0
    lastUpdate = millis();
}
```

To use real sensor values, replace with:
```cpp
sensor_value = analogRead(A0) * (100.0 / 4095.0); // Example for analog sensor
```
