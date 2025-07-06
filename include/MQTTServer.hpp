#pragma once

#ifdef USING_MQTT_SERVER

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Logger.hpp>
#include <PubSubClient.h>
#include <Configs.hpp>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <callbacks/Callbacks.hpp>
#include <Hash.h>

class MQTTServer
{
private:
    void connectToMQTTBroker();
    char * setLWT(bool deviceStatus);
    String Mqtt_topic;
public:
    static void mqttCallback(char *topic, byte *payload, unsigned int length);
    void setup();
    void manageConnection();
    static FireStoreResultCallback* callback;
    ~MQTTServer();
};
#endif