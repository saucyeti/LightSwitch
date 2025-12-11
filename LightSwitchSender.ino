/*LightSwitchSender
  Darren Lin
  12/5/2025
  Utilizes a Seeed Studio Xiao Esp32-S3. Stays in deep sleep mode until external event is triggered --> by a cherry mx switch with space bar. 
  When no longer in deep sleep, uses ESP-NOW protocol to talk to receiver board, sending a signal for 1000ms to let board know to change light 
  switch state. 
*/ 
#include "driver/rtc_io.h"
#include <esp_now.h>
#include "WiFi.h"


#define BUTTON_PIN_BITMASK(GPIO) (1ULL << GPIO)
#define USE_EXT0_WAKEUP 1
#define WAKEUP_GPIO  GPIO_NUM_4

uint8_t broadcastAddress[] = {0x10, 0x20, 0xBA, 0x03, 0xB6, 0x1C}; //receiver MAC address 10:20:ba:03:b6:1c


// Structure to send data
// Must match the receiver structure
typedef struct struct_message {
  bool a;
} struct_message;

// Create a struct_message called myData
struct_message myData;

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(10);
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Transmitted packet
  esp_now_register_send_cb(esp_now_send_cb_t(OnDataSent));
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  while (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    delay(10);//could change this to something like 50 because Receiver wakes up every 150ms
  }
  // Send signal
  myData.a = true;
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));


  #if USE_EXT0_WAKEUP
    esp_sleep_enable_ext0_wakeup(WAKEUP_GPIO,1);
    
    rtc_gpio_pullup_dis(WAKEUP_GPIO);
    rtc_gpio_pulldown_dis(WAKEUP_GPIO);
  
  #else
    esp_sleep_enable_ext1_wakeup_io(BUTTON_PIN_BITMASK(WAKEUP_GPIO), ESP_EXT1_WAKEUP_ANY_HIGH);

    rtc_gpio_pulldown_en(WAKEUP_GPIO); 
    rtc_gpio_pullup_dis(WAKEUP_GPIO);
  #endif

    esp_deep_sleep_start();

}

void loop() {
  // put your main code here, to run repeatedly:

}
