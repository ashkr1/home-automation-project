#pragma once

#ifdef USING_FIREBASE_SERVER

#include <Arduino.h>
#include <time.h>
#include <FirebaseClient.h>
#include <Configs.hpp>
#include <WiFiClientSecure.h>
#include <Logger.hpp>
#include <callbacks/Callbacks.hpp>
#include <ArduinoJson.h>
#include <PersistantStorageManager.hpp>

class FirestoreManager: private virtual PersistantStorageManager{

    WiFiClientSecure client;

    FirebaseApp* app;

    FireStoreResultCallback* callback;

    String deviceJsonBody(String name="", String email="", bool timestamp=false, bool hasState=false, bool state=false, bool hasRebot=false, bool reboot=false);
    JsonDocument* createOrUpdateDocument(String requestType, String jsonBody, String mask="");
    bool registerNewDevice(String email);
    bool verifyDocumentExist(JsonDocument doc);
    String cleanJson(String str);
protected:

    void setFirebaseConfig(FirebaseApp* app, WiFiClientSecure& client);
    void updateDeviceState(bool state);
public:
    ~FirestoreManager();
    void setStateChangeListener(FireStoreResultCallback* callback);
    bool initilizeFireStore(String email);
    void updateHeartbeat();
    void getDeviceInfo();
    void updateReboot(bool reboot);
};
#endif