/* LightSwitchReceiver
  Darren Lin 
  12/6/2025
  Receives message from other ESP32-S3(normally in deep sleep, wakes on external event),
  Turns a servo motor when message is received to turn off the lights, goes from N to SW back to N
  Board is normally in deep sleep, wakes every 150 ms to check for a message.   

  Notes: Vss is always on even in deep sleep,
  need another analogWrite pin along with a transitor to control whether or not current is allowed through to light sensor and servo
*/

#include "driver/rtc_io.h"
#include <esp_now.h>
#include "WiFi.h"
#include <ESP32Servo.h>


#define uS_To_S_Factor 1000000ULL
#define Time_To_Sleep 0.150

#define LIGHTPIN A2
#define SERVOPIN D4
#define POWERPIN GPIO_NUM_6
Servo servo;

typedef struct struct_message{
  bool a;
}struct_message;

struct_message myData;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(10);
  pinMode(LIGHTPIN, INPUT);
  //pinMode(SERVOPIN, OUTPUT);
  pinMode(POWERPIN,INPUT);
  servo.attach(SERVOPIN);


  WiFi.mode(WIFI_STA);
  esp_sleep_enable_timer_wakeup(Time_To_Sleep * uS_To_S_Factor);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }


  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Transmitted packet
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
  
  esp_deep_sleep_start();
}

void loop() {
  // put your main code here, to run repeatedly:

}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  analogWrite(POWERPIN, 1);
  if(analogRead(LIGHTPIN) < 1000 ) //When Dark , turn servo to bright CW --> CCW
  {
    servo.writeMicroseconds(700);//might need to swap with 2200 unsure, how servo behaves
    delay(1000);//calibrate
    servo.writeMicroseconds(2200);
    delay(1000);
    servo.writeMicroseconds(1500);//calibrate
  }
  else //When Light, turn servo to darken CCW --> CW
  {
    servo.writeMicroseconds(2200);
    delay(1000);//calibrate
    servo.writeMicroseconds(700);
    delay(1000);
    servo.writeMicroseconds(1500);//calibrate
  }
  
  analogWrite(POWERPIN,0);
  esp_deep_sleep_start();//sleep after one cycle is ran, might be unnecessary
}
