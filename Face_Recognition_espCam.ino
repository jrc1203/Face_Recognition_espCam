#include "esp_camera.h"
#include <WiFi.h>
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "driver/rtc_io.h"

// =============================================================================
//                                 USER SETTINGS
// =============================================================================
// 1. WIFI CREDENTIALS
//    - If you want the ESP32 to connect to your Home/School WiFi (Recommended so laptop keeps Internet):
//      Enter your SSID and Password below.
//    - If you want the ESP32 to create its own WiFi (AP Mode):
//      Leave these empty (""). Laptop must connect to "ESP32-CAM-Attend".
const char* ssid = "";      // e.g., "School_WiFi"
const char* password = "";  // e.g., "secret123"

// 2. AP MODE SETTINGS (Only used if ssid above is empty or connection fails)
const char* ap_ssid = "ESP32-CAM-Attend";
const char* ap_password = "esp32attend";

// =============================================================================
//                                PIN DEFINITIONS
// =============================================================================
// AI-THINKER ESP32-CAM PIN MAP
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Built-in Flash LED (GPIO 4) - We'll keep it off to avoid blinding students
#define FLASH_LED_GPIO     4

WiFiServer server(80);

void setup() {
  // 1. Disable brownout detector to prevent crashes on power dips
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 

  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  // 2. Camera Configuration
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; 
  
  // Resolution: VGA (640x480) is good for face rec. CIF (400x296) is faster.
  // Let's use VGA.
  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 12; // 0-63, lower is higher quality. 10-12 is good balance.
  config.fb_count = 1;

  // Camera Init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // Turn off Flash LED just in case
  pinMode(FLASH_LED_GPIO, OUTPUT);
  digitalWrite(FLASH_LED_GPIO, LOW);

  // 3. WiFi Setup
  if (String(ssid).length() > 0) {
    // Try Station Mode
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 20) {
      delay(500);
      Serial.print(".");
      retries++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.print("Camera Ready! Use this IP: http://");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("\nWiFi connection failed. Falling back to AP Mode.");
      setupAP();
    }
  } else {
    // Default to AP Mode
    setupAP();
  }

  // 4. Start Server
  server.begin();
}

void setupAP() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);
  Serial.println("");
  Serial.println("AP Mode Started");
  Serial.print("Connect your laptop to WiFi: ");
  Serial.println(ap_ssid);
  Serial.print("Camera IP: http://");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Wait for data
  unsigned long timeout = millis() + 3000;
  while (!client.available() && millis() < timeout) {
    delay(1);
  }
  if (millis() > timeout) {
    client.stop();
    return;
  }

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  client.readStringUntil('\n'); // Read the rest of headers (discard)

  // Handle /capture request
  if (req.indexOf("GET /capture") != -1) {
    camera_fb_t * fb = NULL;
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      client.println("HTTP/1.1 500 Internal Server Error");
      client.println();
      client.stop();
      return;
    }

    // Send Headers (IMPORTANT: CORS allows browser JS to read this)
    client.println("HTTP/1.1 200 OK");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Content-Type: image/jpeg");
    client.println("Content-Disposition: inline; filename=capture.jpg");
    client.print("Content-Length: ");
    client.println(fb->len);
    client.println();

    // Send Image Data
    client.write(fb->buf, fb->len);
    
    // Return the frame buffer
    esp_camera_fb_return(fb);
  } 
  else {
    // Default response for other paths
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println();
    client.println("<h1>ESP32-CAM Attendance</h1>");
    client.println("<p>Go to <a href='/capture'>/capture</a> to see a snapshot.</p>");
  }

  delay(20);
  client.stop();
}
