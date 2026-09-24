# include <U8g2lib.h>
#include <Wire.h>
#include <WiFi.h>
#include <esp_now.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// Replace these hex positions with exact targeted device MAC address
uint8_t receiverMacAddress[] = {0x24, 0xDC, 0xC3, 0xAA, 0xBB, 0xCC};

int liveRSSI = 0;

typedef struct struct_message {
  uint16_t roll;
  uint16_t pitch;
  uint16_t throttle;
  uint16_t yaw;
} struct_message;

typedef struct struct_telemetry {
  float batteryVoltage;
  int motorSpeed;
  float sensorValue;
} struct_telemetry;

struct_message TelemetryData;
struct_telemetry incomingReadings;

esp_now_peer_info_t peerInfo;

String statusReport = "Initializing...";

void readFlysky() {
  while (Serial1.available() > 0) {

    if (Serial1.peek() != 0x20) {
      Serial1.read();
      continue;
    }
    
    if (Serial1.available() < 32) {
      return; 
    }
    
    uint8_t packet[32];
    Serial1.readBytes(packet, 32);
    
    if (packet[1] == 0x40) {
      uint16_t checksum = 0xFFFF;
      for (int i = 0; i < 30; i++) {
        checksum -= packet[i];
      }
      
      uint16_t rxChecksum = packet[30] + (packet[31] << 8);
      
      if (checksum == rxChecksum) {
        TelemetryData.roll     = packet[2] + (packet[3] << 8);
        TelemetryData.pitch    = packet[4] + (packet[5] << 8);
        TelemetryData.throttle = packet[6] + (packet[7] << 8);
        TelemetryData.yaw      = packet[8] + (packet[9] << 8);
      }
    }
  }
}

void OnDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    statusReport = "Link: CONNECTED";
  } else {
    statusReport = "Link: DISCONNECTED";
    liveRSSI=-100;
  }
}

void OnDataRecvd(const esp_now_recv_info_t * recv_info, const uint8_t *incomingData, int len) {
  if (len == sizeof(struct_telemetry)) {
    memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
    liveRSSI= recv_info->rx_ctrl->rssi; 
  }
}

void setup() {
  Serial.begin(115200); // Serial

  Serial1.begin(115200, SERIAL_8N1, D7, D6); // Hardware Serial Setup

  u8g2.begin(); // Oled setup
  u8g2.setFont(u8g2_font_ncenB08_tr);

  // ESPNOW setup
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESPNOW");
    statusReport = "Error";
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecvd));
  memcpy(peerInfo.peer_addr, receiverMacAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    statusReport = "Peer Bind Failed";
    return;
  }

  Serial.println("Flysky2espNOW Initialized");

}

void loop() {
  readFlysky();

  esp_now_send(receiverMacAddress, (uint8_t *) &TelemetryData, sizeof(TelemetryData));  // Sending data over ESPNOW

  u8g2.clearBuffer();

  u8g2.drawStr(0, 10, "--- FLYSKY2espNOW ---");

  char displayLine[64];

  snprintf(displayLine, sizeof(displayLine), "Battery : %.2f V", incomingReadings.batteryVoltage);
  u8g2.drawStr(0, 24, displayLine);
  
  snprintf(displayLine, sizeof(displayLine), "RPM: %d | Val: %.1f", incomingReadings.motorSpeed, incomingReadings.sensorValue);
  u8g2.drawStr(0, 38, displayLine);
  
  snprintf(displayLine, sizeof(displayLine), "Thrust: %d | Yaw: %d", TelemetryData.throttle, TelemetryData.yaw);
  u8g2.drawStr(0, 50, displayLine);
  
  snprintf(displayLine, sizeof(displayLine), "%s (%d dBm)", statusReport.c_str(), liveRSSI);
  u8g2.drawStr(0, 62, displayLine);



  u8g2.sendBuffer();

  delay(50);
}
