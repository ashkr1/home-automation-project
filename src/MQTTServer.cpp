#ifdef USING_MQTT_SERVER

#include <MQTTServer.hpp>

FireStoreResultCallback* MQTTServer::callback = nullptr;

WiFiClientSecure espClient;
PubSubClient mqtt_client(espClient);

#ifndef MQTT_PORT
    #define MQTT_PORT 100
#endif

const char* emqx_cert = R"EOF(
{{MQTT_CONNECTION_CERT}}
)EOF";

MQTTServer::~MQTTServer(){
    if(callback){
        delete callback;
        callback = nullptr;
    }
}

void MQTTServer::setup(){
    mqtt_client.setServer(MQTT_BROKER, 8883);
    mqtt_client.setCallback(MQTTServer::mqttCallback);
    connectToMQTTBroker();
}

String getDeviceId(){
    String chipIdHex = String(ESP.getChipId(), HEX);
    while (chipIdHex.length() < 8) {
        chipIdHex = "0" + chipIdHex;
    }
    return "device_" + chipIdHex;
}

void MQTTServer::connectToMQTTBroker() {
    BearSSL::X509List serverTrustedCA(emqx_cert);
    espClient.setTrustAnchors(&serverTrustedCA);
    while (!mqtt_client.connected()) {
        String client_id = getDeviceId();
        LOGI("Connecting to MQTT Broker as %s.....\n", client_id.c_str());
        if (mqtt_client.connect(client_id.c_str(), MQTT_USERNAME, MQTT_PASSWORD)) {
            LOGFI("Connected to MQTT broker");
            mqtt_client.subscribe(MQTT_TOPIC);
            // Publish message upon successful connection
            // mqtt_client.publish(MQTT_TOPIC, "{Hi EMQX I'm ESP8266 ^^");
        } else {
            char err_buf[128];
            espClient.getLastSSLError(err_buf, sizeof(err_buf));
            Serial.print("Failed to connect to MQTT broker, rc=");
            Serial.println(mqtt_client.state());
            Serial.print("SSL error: ");
            Serial.println(err_buf);
            delay(5000);
        }
    }

}

void MQTTServer::mqttCallback(char *topic, byte *payload, unsigned int length) {
    LOGFV("Message received on topic: ");
    LOGV(topic);
    String result="";
    for (unsigned int i = 0; i < length; i++) {
        result+=(char) payload[i];
        // Serial.print((char) payload[i]);
    }
    LOGFV("-----------------------");
    JsonDocument doc;
    DeserializationError error =  deserializeJson(doc,result);
    if (error) {
        LOGFE("error occured when deserializing");
        LOGE("deserializeJson() returned %s",error.c_str());
    }else{
        bool state = false, reset = false;
        if(doc["state"].is<bool>()){
            state = doc["state"].as<bool>();
        }
        if(doc["reset"].is<bool>()){
            reset = doc["reset"].as<bool>();
        }

        if(callback){
            callback->onDeviceStateChange(state,reset);
        }
    }
}

void MQTTServer::manageConnection(){
    if (!mqtt_client.connected()) {
        connectToMQTTBroker();
    }

    mqtt_client.loop();
}

#endif