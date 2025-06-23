#pragma once

#include <Arduino.h>
#include <time.h>
#include <vector>
#include <WiFiUdp.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <Constant.hpp>
#include <Logger.hpp>
#include <callbacks/Callbacks.hpp>
#include <PersistantStorageManager.hpp>

class CaptivePortal:private PersistantStorageManager, private DNSServer{

    static CaptivePortal* INSTANCE;
    static std::vector<ConnectionCallback*> callbacks;

    bool isFirstTime = true;
    unsigned long lastReconnectAttempt=0;

    const int port = 80;
    ESP8266WebServer* webServer = new ESP8266WebServer(80);
    bool dnsServerRunning = false;

    void softApEndpoints();
    void staEndPoints(); 
    void handleConnectionCallback(Connection::StaConnection state);
    bool connectToStation(char* ssid, char* pass);
    CaptivePortal();

    protected:
    
    bool connectToSavedStation();
    void restartServer();
    bool startSoftAp();

    void connectFromAPI(char* ssid, char * pass);

    public:

    static CaptivePortal* getInstance();
    static CaptivePortal* getInstance(ConnectionCallback* callback);

    static void addStaConnectionCallback(ConnectionCallback* callback);

    ~CaptivePortal();

    static void onStationDisconnected(const WiFiEventSoftAPModeStationDisconnected& evt);
    static void onStationConnected(const WiFiEventSoftAPModeStationConnected& evt);

    void startServer();

    bool reconnect();

    void nextRequestHandler();

};

