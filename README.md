# Classroom Face Recognition Attendance

Browser-based attendance system using face recognition. Works with ESP32-CAM or Laptop Webcam.

## Setup

1. **ESP32-CAM**
   - Open `Face_Recognition_espCam.ino` in Arduino IDE.
   - Edit `ssid` and `password` with your WiFi credentials.
   - Upload to AI Thinker ESP32-CAM.
   - Note the IP address from Serial Monitor.

2. **Web App**
   - Serve `index.html` using a local server (e.g., Python `python -m http.server` or VS Code Live Server).
   - Open the hosted URL (e.g., `http://localhost:8000`).

## Usage

1. **Select Source**: Choose "Laptop Webcam" or enter ESP32 IP and click "Use ESP32-CAM".
2. **Enroll**: Enter name, click "Enroll". System takes 3 samples.
3. **Attendance**: Click "Start Recognition". Detected faces are marked present.
4. **Export**: Click "Export CSV" to save attendance log.

## Notes
- Ensure laptop and ESP32 are on the same WiFi.
- Browser must have internet access to load AI models.
- If using ESP32, performance depends on network speed.
