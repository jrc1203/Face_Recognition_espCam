# 📸 Classroom Face Recognition Attendance System

A simple, educational face recognition system for Class 9 students. It runs entirely in the browser and can use either a laptop webcam or an ESP32-CAM.

## 🚀 Quick Start

### 1. Prepare the Hardware (ESP32-CAM)
*Note: You can skip this if you only want to use the Laptop Webcam.*

1.  Open `esp32cam_capture.ino` in Arduino IDE.
2.  **Select Board**: `AI Thinker ESP32-CAM`.
3.  **Important - Internet Access**:
    - The app needs to download `face-api.js` from the internet the first time it loads.
    - **Recommended**: Edit the top of the code to enter your School/Home WiFi credentials (`ssid` and `password`). This allows your laptop to stay on the internet while talking to the camera.
    - **Offline Mode**: If you leave credentials empty, the ESP32 creates a WiFi hotspot named `ESP32-CAM-Attend`. If you connect to this, your laptop will lose internet, and the app might not load unless you have cached it previously.
4.  **Upload** the code. Open **Serial Monitor** (115200 baud) to see the IP address (e.g., `192.168.1.105`).

### 2. Run the App
1.  You need to "serve" the `index.html` file (opening it directly by double-clicking usually blocks the camera due to security rules).
2.  **Easy Method**:
    - If you have Python installed: Open terminal in this folder and run `python -m http.server`.
    - Then open `http://localhost:8000` in your browser.
    - OR use the "Live Server" extension in VS Code.

### 3. Using the App
1.  **Select Source**: When the page loads, you have 5+ seconds (infinite, actually) to choose:
    - **Laptop Webcam**: Easiest for testing.
    - **ESP32-CAM**: Enter the IP address you saw in the Serial Monitor.
2.  **Enrollment**:
    - Type a student's name (e.g., "Rahul").
    - Click **Enroll**. The camera will take 3 quick snapshots.
    - *Tip*: Move your head slightly between shots for better accuracy.
3.  **Attendance**:
    - Click **Start Recognition**.
    - When the enrolled face appears, the box turns green, and the name is added to the "Present" list.

## 👩‍🏫 Teacher's Demo Script (Class 9)
*"Class, today we are building an AI Attendance system. It works in 3 steps:*
1.  ***Detection**: The computer scans the image to find a face (like a box).*
2.  ***Embedding**: It converts the face into a list of numbers (a unique code for your face).*
3.  ***Matching**: It compares the new code with the codes we saved. If the numbers are close enough, it knows it's you!"*

## 🛠 Troubleshooting
- **"Model Load Error"**: You probably don't have internet. If using ESP32 AP mode, switch to Station mode (connect ESP32 to your router) so your laptop has internet.
- **ESP32 Image Frozen**: The ESP32-CAM can be slow. Try reloading the page or pressing the Reset button on the ESP32.
- **False Matches**: Increase the "Confidence Threshold" slider to 0.7 or 0.8 to make it stricter.
