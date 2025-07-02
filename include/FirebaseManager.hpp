#pragma once

#ifdef USING_FIREBASE_SERVER

#include <Configs.hpp>

#include <Arduino.h>
#include <Hash.h>
#include <WiFiClientSecure.h>
#include <Logger.hpp>
#include <enums/Enums.hpp>
#include <callbacks/Callbacks.hpp>
#include <PersistantStorageManager.hpp>
#include <FirebaseClient.h>
#include <FirestoreManager.hpp>

class FirebaseManager: private virtual PersistantStorageManager, public FirebaseStatusCallback, public FirestoreManager{

    static FirebaseManager* INSTANCE;
    String mac = "";

    std::vector<FirebaseCallback*> callbacks;

    static FirebaseStatusCallback* firebaseCallback;
    FirebaseApp app;

    FirebaseManager();

    void createUser();
    bool verifyUserExist();
    void loginUser();

    String getDeviceFBEmail();
    String getDeviceFBPass();

    void handleStatusCallback(FirebaseCallbackResult& result);

    public:
    ~FirebaseManager();
    void addStatusCallback(FirebaseCallback* callback);
    static void asyncFirebaseCallback(AsyncResult &aResult);
    static FirebaseManager* getInstance();
    
    void onFailure(FirebaseCallbackResult& result) override;
    void onSuccess(FirebaseCallbackResult& result) override;

    void asyncLoop();
    void initilizeApp();
    void initilizeFirestore();

    void setMacAddress(String mac);

};
#endif