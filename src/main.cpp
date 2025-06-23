#define DEBUG_ESP_PORT Serial
#define DEBUG_ESP_WIFI

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <callbacks/Callbacks.hpp>
#include <FirebaseManager.hpp>
#include "CaptivePortal.hpp"
#include <Logger.hpp>

bool abnormalReset=false;
int blinking=10;
#define ABNORMAL_RESET_WARNING_PIN 0

class Handler:public ConnectionCallback, public FirebaseCallback, public FireStoreResultCallback{

  bool isConnected=false;
  FirebaseManager* manager = nullptr;
  long long int lastReconnectAttempt = 0;
  long long int heartBeatSyncing=0;
  bool firestore_initilization_complete = false;
public:

  ~Handler(){
    if(manager!=nullptr){
      delete manager;
      manager = nullptr;
    }
  }

  void init(){
    manager = FirebaseManager::getInstance();
    manager->addStatusCallback(this);
    manager->setStateChangeListener(this);
  }

  void onConnectionStateChange(Connection::StaConnection sta) override {

    if(sta == Connection::StaConnection::STA_CONNECTED){
      LOGFV("callback connection success");
      isConnected=true;
      delay(1000);
      manager->initilizeApp();
      // FirebaseManager::getInstance()->initilizeApp();
    }else{
      LOGV("callback connection failed");
      isConnected=false;
    }
  }

  void onFailure(FirebaseCallbackResult& result) override {
      LOGE("failure: %s", result.message);
  };

  void onAuthSuccess(FirebaseCallbackResult& result) override {
      LOGI("login/signup success");
      if(result.code == 10){
            LOGI("User UID: %s\n", result.app.getUid().c_str());
            LOGI("Auth Token: %s\n", result.app.getToken().c_str());
            LOGI("Refresh Token: %s\n", result.app.getRefreshToken().c_str());

            LOGFV("proceeding for initilizing firestore...");
            manager->initilizeFirestore();
            firestore_initilization_complete = true;
        }
  };

  void onDeviceStateChange(bool state, bool reboot) override {
    if(!reboot){
      if(state){
        digitalWrite(2,LOW);
      }else{
        digitalWrite(2, HIGH);
      }
    }else{
      firestore_initilization_complete=false;
      manager->updateReboot(false);
      digitalWrite(2, LOW);
      LOGFW("rebooting device on request....");
      delay(1000);
      ESP.restart();
    }
  };

  void readStateChange(){
    if (firestore_initilization_complete && millis() - lastReconnectAttempt >= RECONNECT_TIME_INTERVEL) {
            LOGFV("calling reading state value");
            manager->getDeviceInfo();
            LOGFI("reading state value completed");
            lastReconnectAttempt = millis();
        }
  }

  void updateServerHeartBeat(){
    if (firestore_initilization_complete && millis() - heartBeatSyncing >= HEART_BEAT_INTERVAL) {
            LOGFV("updating heartbeat");
            manager->updateHeartbeat();
            LOGFI("updating heartbeat completed");
            heartBeatSyncing = millis();
        }
  }

  
  void manageConnectionCalls(){
    
    CaptivePortal::getInstance()->nextRequestHandler();
    
    CaptivePortal::getInstance()->reconnect();

    readStateChange();
    updateServerHeartBeat();
  }

};

Handler handler;

void setup() {
  

  LOGI("Prime HomeLink, started: %s",FIRMWARE_VERSION);

  Serial.begin(74880);
  Serial.setDebugOutput(true);

  pinMode(ABNORMAL_RESET_WARNING_PIN,OUTPUT);
  
  auto reason = ESP.getResetInfoPtr()->reason;
  if (reason == REASON_EXCEPTION_RST) {
      digitalWrite(ABNORMAL_RESET_WARNING_PIN, HIGH);
      LOGFW("⚠️ Previous boot ended abnormally.");
  }else{
    digitalWrite(ABNORMAL_RESET_WARNING_PIN,LOW);
  }

  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);

  handler.init();

  delay(5000);

  CaptivePortal::addStaConnectionCallback(&handler);

  CaptivePortal::getInstance()->startServer();

}

void loop() {

    if (ESP.getFreeHeap() < 5000) {
      LOGFE("Heap too low! Resetting...");
      ESP.restart();
    }

    auto reason = ESP.getResetInfoPtr()->reason;

    if (reason == REASON_EXCEPTION_RST) {
      if(--blinking){
          digitalWrite(ABNORMAL_RESET_WARNING_PIN, abnormalReset?HIGH:LOW);
          abnormalReset=!abnormalReset;
          delay(1000);
          LOGI("blink");
      }else{
        ESP.restart();
      }
      
    LOGFW("⚠️ Previous boot ended abnormally.");
  }

    FirebaseManager::getInstance()->asyncLoop();
    handler.manageConnectionCalls();
}
