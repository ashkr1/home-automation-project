#define DEBUG_ESP_PORT Serial
#define DEBUG_ESP_WIFI

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <callbacks/Callbacks.hpp>
#include <FirebaseManager.hpp>
#include "CaptivePortal.hpp"
#include <Logger.hpp>

#ifdef USING_MQTT_SERVER
#include <MQTTServer.hpp>
MQTTServer server;
#endif

bool abnormalReset=false;
int blinking=10;
#define ABNORMAL_RESET_WARNING_PIN 0

class Handler:public ConnectionCallback, public FirebaseCallback, public FireStoreResultCallback{

  bool isConnected=false;
  #ifdef USING_FIREBASE_SERVER
  FirebaseManager* manager = nullptr;
  #endif
  long long int lastReconnectAttempt = 0;
  long long int heartBeatSyncing=0;
  bool firestore_initilization_complete = false;
public:

  ~Handler(){
    #ifdef USING_FIREBASE_SERVER
    if(manager!=nullptr){
      delete manager;
      manager = nullptr;
    }
    #elif USING_MQTT_SERVER
      if(MQTTServer::callback){
        delete MQTTServer::callback;
        MQTTServer::callback = nullptr;
      }
    #endif
  }

  void init(){
    #ifdef USING_FIREBASE_SERVER
      manager = FirebaseManager::getInstance();
      manager->addStatusCallback(this);
      manager->setStateChangeListener(this);
    #elif USING_MQTT_SERVER
      MQTTServer::callback = this;
    #endif
  }

  void onConnectionStateChange(Connection::StaConnection sta) override {

    if(sta == Connection::StaConnection::STA_CONNECTED){
      LOGFV("callback connection success");
      isConnected=true;
      delay(1000);
      #ifdef USING_MQTT_SERVER
        server.setup();
      #elif USING_FIREBASE_SERVER
        manager->initilizeApp();
      #endif
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
      #ifdef USING_FIREBASE_SERVER
      LOGI("login/signup success");
      if(result.code == 10){
            LOGI("User UID: %s\n", result.app.getUid().c_str());
            LOGI("Auth Token: %s\n", result.app.getToken().c_str());
            LOGI("Refresh Token: %s\n", result.app.getRefreshToken().c_str());

            LOGFV("proceeding for initilizing firestore...");
            #ifdef USING_FIREBASE_SERVER
              manager->initilizeFirestore();
              firestore_initilization_complete = true;
            #endif
        }
        #endif
  };

  void onDeviceStateChange(bool state, bool reboot) override {
    if(!reboot){
      if(state){
        LOGFI("device state changed, Pin set to LOW");
        digitalWrite(2,LOW);
      }else{
        LOGFI("device state changed, Pin set to HIGH");
        digitalWrite(2, HIGH);
      }
    }else{
      firestore_initilization_complete=false;
      #ifdef USING_FIREBASE_SERVER
        manager->updateReboot(false);
      #endif
      digitalWrite(2, LOW);
      LOGFW("rebooting device on request....");
      delay(2000);
      ESP.restart();
    }
  };

  #ifdef USING_FIREBASE_SERVER
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
  #endif

  
  void manageConnectionCalls(){
    
    CaptivePortal::getInstance()->nextRequestHandler();
    
    CaptivePortal::getInstance()->reconnect();

    if(isConnected){
      #ifdef USING_MQTT_SERVER
        server.manageConnection();
      #endif
    }
    #ifdef USING_FIREBASE_SERVER
      readStateChange();
      updateServerHeartBeat();
    #endif
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
    #ifdef USING_FIREBASE_SERVER
      FirebaseManager::getInstance()->asyncLoop();
    #endif
    handler.manageConnectionCalls();
}
