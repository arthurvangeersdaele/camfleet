#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <base64.h>  // ESP32 Base64 library for encoding images
#include <Base64.h>

#define CAMERA_MODEL_AI_THINKER // Has PSRAM
#include "camera_pins.h"

// Flash pin fonctionality 
#define FLASH_LED_PIN 4

// identifiers
const char *ssid = "Metrologie";
const char *password = "8491metrologie";
const char* host = "script.google.com";
String myDeploymentID = "AKfycbzD7ZJGEIU0eHfJSmCqQaQ7Gd6U-G9jF-N4SG1hHAaG1lmUWYuZ2XDvVc2d9M9whXAxUA"; // 21 (upload and delete)
String myCamName = "ESP32-CAM-Nomad";
WiFiClientSecure client;

// void startCameraServer();

void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);
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

    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");

    // startCameraServer();
    // Serial.printf("Camera Ready! Use 'http://%s' to connect\n", WiFi.localIP().toString().c_str());
}
void sendSnapshotToGoogleDrive() {
    client.setInsecure();  // Disable SSL certificate verification
    if (client.connect(host, 443)) {
      Serial.println("Connection to script.google.com successful.");
    }
    else{
      return;
    }
    camera_fb_t *fb = esp_camera_fb_get();
    if(!fb) {
      Serial.println("Camera capture failed.");
      Serial.println("Restarting the ESP32 CAM.");
      delay(1000);
      ESP.restart();
      return;
    } 
    else{
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
    }
    else{
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
    sendSnapshotToGoogleDrive();
    delay(1000);  // Send image every 1 seconds (adjust as needed)
}
