#include "Arduino.h"
#include "driver/rtc_io.h"
#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "soc/rtc_cntl_reg.h" 
#include "soc/soc.h"          
#include <WiFi.h>


const char *ssid = "MySpyCar";
const char *password = "123456789";


#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

#define FLASH_LED_GPIO 4

WiFiServer server(80);

void setup() {
  // 1. Disable brownout detector
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

  // Resolution: CIF (400x296) 
  config.frame_size = FRAMESIZE_CIF;
  config.jpeg_quality =
      20; // Higher number = lower quality (smaller file, faster speed)
  config.fb_count = 1;

  // Camera Init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  pinMode(FLASH_LED_GPIO, OUTPUT);
  digitalWrite(FLASH_LED_GPIO, LOW);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("Camera Ready! Use this IP: http://");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  unsigned long timeout = millis() + 3000;
  while (!client.available() && millis() < timeout) {
    delay(1);
  }
  if (millis() > timeout) {
    client.stop();
    return;
  }

  // Read request
  String req = client.readStringUntil('\r');
  client.readStringUntil('\n'); // Discard rest

  // Handle /capture
  if (req.indexOf("GET /capture") != -1) {
    camera_fb_t *fb = NULL;
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      client.println("HTTP/1.1 500 Internal Server Error");
      client.stop();
      return;
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Content-Type: image/jpeg");
    client.println("Content-Disposition: inline; filename=capture.jpg");
    client.print("Content-Length: ");
    client.println(fb->len);
    client.println();

    // Send Image
    client.write(fb->buf, fb->len);
    esp_camera_fb_return(fb);
  }
  else if (req.indexOf("GET /flash") != -1) {
    if (req.indexOf("state=1") != -1) {
      digitalWrite(FLASH_LED_GPIO, HIGH);
    } else if (req.indexOf("state=0") != -1) {
      digitalWrite(FLASH_LED_GPIO, LOW);
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Content-Type: text/plain");
    client.println();
    client.println("OK");
  } else {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println();
    client.println("<h1>ESP32-CAM Attendance (BETA)</h1>");
    client.print("<p>Connected to: ");
    client.print(ssid);
    client.println("</p>");
    client.println(
        "<p>Go to <a href='/capture'>/capture</a> to see a snapshot.</p>");
  }

  delay(20);
  client.stop();
}
