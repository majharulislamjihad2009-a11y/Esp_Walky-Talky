/*
  ESP‑NOW Walkie‑Talkie for ESP32‑S3 (Broadcast Mode)
  Made by xihadnova
  GitHub: https://github.com/majharulislamjihad2009-a11y/
*/

#include <WiFi.h>
#include <esp_now.h>
#include <driver/i2s.h>

// I2S pins
#define I2S_MIC_WS    4
#define I2S_MIC_SCK   5
#define I2S_MIC_DIN   6
#define I2S_SPK_DOUT  7
#define I2S_SPK_BCLK 15
#define I2S_SPK_LRC  16
#define PTT_PIN       39

// Audio settings
#define SAMPLE_RATE       16000
#define I2S_BUFFER_LEN    256
#define FRAME_SAMPLES     100
#define FRAME_BYTES       (FRAME_SAMPLES * sizeof(int16_t))

// ESP‑NOW broadcast address
uint8_t broadcastAddr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void setupI2SMic() {
  i2s_config_t i2s_config_mic = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_MSB,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = I2S_BUFFER_LEN,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };
  i2s_pin_config_t pin_config_mic = {
    .bck_io_num = I2S_MIC_SCK,
    .ws_io_num = I2S_MIC_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_MIC_DIN
  };
  i2s_driver_install(I2S_NUM_1, &i2s_config_mic, 0, NULL);
  i2s_set_pin(I2S_NUM_1, &pin_config_mic);
  i2s_zero_dma_buffer(I2S_NUM_1);
}

void setupI2SSpeaker() {
  i2s_config_t i2s_config_speaker = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_MSB,
    .intr_alloc_flags = 0,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0
  };
  i2s_pin_config_t pin_config_speaker = {
    .bck_io_num = I2S_SPK_BCLK,
    .ws_io_num = I2S_SPK_LRC,
    .data_out_num = I2S_SPK_DOUT,
    .data_in_num = I2S_PIN_NO_CHANGE
  };
  i2s_driver_install(I2S_NUM_0, &i2s_config_speaker, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config_speaker);
  i2s_zero_dma_buffer(I2S_NUM_0);
}

void OnDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
  Serial.print("Delivery ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (len == FRAME_BYTES) {
    size_t bytes_written;
    i2s_write(I2S_NUM_0, data, len, &bytes_written, portMAX_DELAY);
  }
}

void setPeer() {
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddr, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\nESP‑NOW Walkie‑Talkie (Broadcast Mode) – Made by xnova");

  setupI2SSpeaker();
  pinMode(PTT_PIN, INPUT_PULLUP);

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP‑NOW init failed");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  setPeer();

  Serial.println("Ready. Press PTT to talk.");
}

void loop() {
  static bool sending = false;
  static bool lastPTT = HIGH;

  bool currentPTT = digitalRead(PTT_PIN);

  if (currentPTT == LOW && lastPTT == HIGH) {
    sending = true;
    setupI2SMic();
    esp_now_register_recv_cb(NULL);
    Serial.println("PTT ON – transmitting");
  }
  else if (currentPTT == HIGH && lastPTT == LOW) {
    sending = false;
    i2s_driver_uninstall(I2S_NUM_1);
    esp_now_register_recv_cb(OnDataRecv);
    Serial.println("PTT OFF – listening");
  }

  if (sending) {
    int16_t buffer[FRAME_SAMPLES];
    size_t bytesRead;
    i2s_read(I2S_NUM_1, buffer, FRAME_BYTES, &bytesRead, portMAX_DELAY);
    if (bytesRead == FRAME_BYTES) {
      for (int i = 0; i < FRAME_SAMPLES; i++) {
        int32_t amplified = buffer[i] * 2;
        if (amplified > 32767) amplified = 32767;
        if (amplified < -32768) amplified = -32768;
        buffer[i] = (int16_t)amplified;
      }
      esp_now_send(broadcastAddr, (uint8_t*)buffer, FRAME_BYTES);
    }
  }

  lastPTT = currentPTT;
  delay(1);
}
