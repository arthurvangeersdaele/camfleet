#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <esp_sleep.h>
#include <NTPClient.h>

#include <base64.h>  // ESP32 Base64 library for encoding images
#include <Base64.h>

#define CAMERA_MODEL_AI_THINKER  // Has PSRAM
#include "camera_pins.h"

// global variables
#define FLASH_LED_PIN 4

#define SLEEP_TIME 10 // Sleep time in seconds

// time management
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000); // UTC time, update every 60s

// identifiers
const char *ssids[] = {"SSID1", "SSID2", "SSID3"};
const char *passwords[] = {"PASS1", "PASS2", "PASS3"};
const int numNetworks = sizeof(ssids) / sizeof(ssids[0]);

const char *host = "script.google.com";
String myDeploymentID = "";  // replace by your deployment ID
String myCamName = "ESP32-CAM_SimonDeJeager";
WiFiClientSecure client;

// void startCameraServer();

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  pinMode(FLASH_LED_PIN, OUTPUT);
  connectToWiFi();
  Serial.println();

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
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_RGB565;
  config.frame_size = FRAMESIZE_VGA;  // Can adjust to FRAMESIZE_VGA or higher
  config.jpeg_quality = 8;
  config.fb_count = 1;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init failed!");
    return;
  }
}
void sendSnapshotToGoogleScript() {
  client.setInsecure();  // Disable SSL certificate verification
  if (client.connect(host, 443)) {
    Serial.println("Connection to script.google.com successful.");
  } else {
    return;
  }
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed.");
    Serial.println("Restarting the ESP32 CAM.");
    delay(1000);
    ESP.restart();
    return;
  } else {
    Serial.println("Taking a photo was successful.\nStarting conversion to JPG.");
  }

  // JPEG compression, if necessary (the frame is already in JPEG format, but let's use frame2jpg to confirm)
  uint8_t *out_buffer = nullptr;
  size_t out_len = 0;

  // Compress the frame to JPEG if it's not already
  frame2jpg(fb, 90, &out_buffer, &out_len);  // Compress with 90% quality

  if (!out_buffer) {
    Serial.println("JPEG compression failed. \nHexadecimal compression anyway...");
    esp_camera_fb_return(fb);
    return;
  } else {
    Serial.println("JPEG compression was successful.\nStarting hexadecimal compression...");
  }
  String url = "/macros/s/" + myDeploymentID + "/exec?sourceCam=" + myCamName + "&requestType=" + "upload";
  client.println("POST " + url + " HTTP/1.1");
  client.println("Host: " + String(host));
  client.println("Transfer-Encoding: chunked");
  client.println();
  int chunkSize = 3000;
  int chunkBase64Size = base64_enc_len(chunkSize);
  char output[chunkBase64Size + 1];
  char *input = (char *)out_buffer;
  int fbLen = fb->len;

  for (int i = 0; i < fbLen; i += chunkSize) {
    int l = base64_encode(output, input, min(fbLen - i, chunkSize));
    client.print(l, HEX);  // Sending the base64-encoded chunk directly
    client.print("\r\n");
    client.print(output);
    client.print("\r\n");
    delay(100);
    input += chunkSize;
  }
  client.print("0\r\n\r\n");

  free(out_buffer);  // Don't forget to free the memory for JPEG buffer

  esp_camera_fb_return(fb);
  Serial.println("Image sent.");
  client.stop();
}


void loop() {

  if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi disconnected, restarting connection...");
        connectToWiFi();
  }
  timeClient.update();
  int currentHour = timeClient.getHours();
  Serial.println(currentHour);
  // Turn the LED on between 5 PM (17:00) and 8 AM (08:00)
  if (currentHour >= 16 || currentHour < 9) {
      digitalWrite(FLASH_LED_PIN, HIGH);
  } else {
      digitalWrite(FLASH_LED_PIN, LOW);
  }
  sendSnapshotToGoogleScript();
  delay(5000);  // Send image every 30 seconds (adjust as needed)

}

void connectToWiFi() {
    for (int i = 0; i < numNetworks; i++) {
        Serial.print("Trying to connect to: ");
        Serial.println(ssids[i]);

        WiFi.disconnect(true); // Clear WiFi state
        WiFi.mode(WIFI_OFF);   // Turn off WiFi
        delay(1000);           // Short delay to reset hardware
        WiFi.mode(WIFI_STA);   // Set WiFi to station mode

        WiFi.begin(ssids[i], passwords[i]);

        int attempt = 0;
        while (WiFi.status() != WL_CONNECTED && attempt < 80) { // Try for 20 seconds
            //digitalWrite(FLASH_LED_PIN, HIGH);
            analogWriteManual(FLASH_LED_PIN, 1); 
            delay(10);
            digitalWrite(FLASH_LED_PIN, LOW);
            delay(240);
            Serial.print(".");
            attempt++;
        }

        if (WiFi.status() == WL_CONNECTED) {
          Serial.println("\nWiFi connected");

          // Victory sequence (LED blinking pattern)
          for (int i = 0; i < 3; i++) {  // Blink 3 times rapidly
              digitalWrite(FLASH_LED_PIN, HIGH);  // LED on
              delay(100);                         // 100ms on
              digitalWrite(FLASH_LED_PIN, LOW);   // LED off
              delay(100);                         // 100ms off
          }
    
            digitalWrite(FLASH_LED_PIN, LOW); // Ensure LED is off after sequence
            return;
        }
        Serial.println(WiFi.status());
        Serial.println("\nFailed to connect");
        
    }
    Serial.println("No networks connected, entering deep sleep...");
    enterDeepSleep();
    ESP.restart();
}


void enterDeepSleep() {
    Serial.println("Going into deep sleep...");
    digitalWrite(FLASH_LED_PIN, LOW); // Ensure LED is off
    esp_sleep_enable_timer_wakeup(SLEEP_TIME * 1000000); // Sleep for 10 seconds
    esp_deep_sleep_start();
}

// Manual "analogWrite" replacement using PWM with delay for dimming effect
void analogWriteManual(int pin, int value) {
  int delayTime = map(value, 0, 255, 1000, 0);  // Map intensity to delay time
  digitalWrite(pin, HIGH);  // LED on
  delayMicroseconds(delayTime);  // On time
  digitalWrite(pin, LOW);  // LED off
  delayMicroseconds(1000 - delayTime);  // Off time
}
