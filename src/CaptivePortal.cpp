#include "CaptivePortal.hpp"

CaptivePortal* CaptivePortal::INSTANCE = nullptr;
std::vector<ConnectionCallback*> CaptivePortal::callbacks;

CaptivePortal::CaptivePortal(){
    if(isWifiCredentialsExists()){
        LOGFV("initiating class from callback");
        if(connectToSavedStation()){
            LOGFV("class connection set true");
            handleConnectionCallback(Connection::StaConnection::STA_CONNECTED);
        }else{
            LOGFV("class connection set false");
            handleConnectionCallback(Connection::StaConnection::STA_DISCONNECTED);
        }
    }else{
        LOGD(startSoftAp()?"softAP server started" : "failed to start soft ap server");
    }
}

void syncTime() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    while (time(nullptr) < 100000) {
        delay(100);
        Serial.print(".");
    }
    LOGFI("⏰ Time synced");
}

bool CaptivePortal::clearPersistedData(){
    if(this->isWifiCredentialsExists()){
        this->wipeAll();
        return true;
    }

    return false;
}


CaptivePortal::~CaptivePortal(){
    if(webServer!=nullptr){
        webServer->stop();
        webServer = nullptr;
        delete webServer;
    }
}

CaptivePortal* CaptivePortal::getInstance(){
    if(INSTANCE == nullptr){
        INSTANCE = new CaptivePortal();
    }
    return INSTANCE;
}

CaptivePortal* CaptivePortal::getInstance(ConnectionCallback* callback){
    if(INSTANCE == nullptr){
        INSTANCE = new CaptivePortal();
    }

    LOGFV("calling instance callback");
    INSTANCE->addStaConnectionCallback(callback);

    return INSTANCE;
}

void CaptivePortal::addStaConnectionCallback(ConnectionCallback* callback){
    LOGFV("callback added to vector");
    callbacks.push_back(callback);
}

void CaptivePortal::handleConnectionCallback(Connection::StaConnection state){
    for(ConnectionCallback* callback : this->callbacks){
        LOGV("calling callbacks connection state: %d",state);
        if(callback){
            callback->onConnectionStateChange(state);
        }
    }
}

bool CaptivePortal::startSoftAp(){

    WiFi.onSoftAPModeStationConnected(CaptivePortal::onStationConnected);
    WiFi.onSoftAPModeStationDisconnected(CaptivePortal::onStationDisconnected);

    #if SOFTAP_NETWORK_OPEN
        return WiFi.softAP(SOFTAP_HOMELINK_SSID);
    #else
        return WiFi.softAP(SOFTAP_HOMELINK_SSID,SOFTAP_HOMELINK_PASSWORD)
    #endif

}

void CaptivePortal::onStationConnected(const WiFiEventSoftAPModeStationConnected& evt) {
  LOGV("Device connected: MAC=%02X:%02X:%02X:%02X:%02X:%02X, AID=%d\n",
                evt.mac[0], evt.mac[1], evt.mac[2], evt.mac[3], evt.mac[4], evt.mac[5],
                evt.aid);
}

void CaptivePortal::onStationDisconnected(const WiFiEventSoftAPModeStationDisconnected& evt) {
  LOGV("Device disconnected: MAC=%02X:%02X:%02X:%02X:%02X:%02X, AID=%d\n",
                evt.mac[0], evt.mac[1], evt.mac[2], evt.mac[3], evt.mac[4], evt.mac[5],
                evt.aid);
}

void CaptivePortal::softApEndpoints(){
    LOGFV("setting endpoints for softAP mode");
    #ifdef ENABLE_CAPTIVE_MODE
        webServer->on("/",HTTP_GET,[this](){
        webServer->send(200,"text/html",HtmlTemplates::HTML_CONTENT);
        });


        //android captive view
        webServer->on("/generate_204", HTTP_GET, [this]() {
            webServer->sendHeader("Location", "/");
            webServer->send(302, "text/html", "");
        });

        //ios captive view
        webServer->on("/hotspot-detect.html", HTTP_GET, [this]() {
            webServer->send(200,"text/html",HtmlTemplates::HTML_CONTENT);
        });

        webServer->on("/styles.css", HTTP_GET, [this]() {
            webServer->sendHeader("Content-Type", "text/css");
            webServer->send(200, "text/css", HtmlTemplates::CSS);
        });

        webServer->on("/script.js", HTTP_GET, [this]() {
            webServer->sendHeader("Content-Type", "application/javascript");
            webServer->send(200, "application/javascript", HtmlTemplates::JS);
        });
    
    #endif
    webServer->on("/scan", HTTP_GET, [this]() {
    // Log the scan request
    LOGFI("[INFO] Received WiFi scan request");
    
    // Start scanning
    LOGFI("[INFO] Starting WiFi network scan...");
    int n = WiFi.scanNetworks();
    
    // Log scan results
    LOGI("[INFO] Found %d networks\n", n);
    
    // Create JSON response
    String json = "[";
    for (int i = 0; i < n; ++i) {
        if (i != 0) json += ",";
    
        // Escape special characters in SSID
        String escapedSSID = WiFi.SSID(i);
        escapedSSID.replace("\"", "\\\"");
        escapedSSID.replace("\\", "\\\\");
    
        bool isSecure = WiFi.encryptionType(i) != ENC_TYPE_NONE;
    
        json += "{";
        json += "\"ssid\":\"" + escapedSSID + "\",";
        json += "\"secure\":" + String(isSecure ? "true" : "false") + ",";  // Removed extra quote
        json += "\"rssi\":" + String(WiFi.RSSI(i));
        json += "}";
    
        // Log each network found
        LOGI("[SCAN] Network %d: %s (%s) (%d)\n", 
                 i+1, 
                 WiFi.SSID(i).c_str(), 
                 isSecure ? "secured" : "open",
                 WiFi.RSSI(i));
    }
    json += "]";
    
    // Send response
    webServer->send(200, "application/json", json);
    LOGFI("[INFO] Sent JSON response to client");
    
    // Clean up scan result
    WiFi.scanDelete();
    LOGFI("[INFO] Cleared scan cache");
    });


    webServer->on("/connect", HTTP_POST, [this]() {
        if (webServer->hasArg("ssid")) {
        String ssid = webServer->arg("ssid");
        String password = webServer->arg("password");
        
        LOGI("Connecting to: %s", ssid.c_str());

        connectFromAPI((char*)ssid.c_str(),(char*)password.c_str());

  } else {
    webServer->send(400, "text/plain", "Missing SSID parameter");
  }
});


    webServer->onNotFound([this](){
        // Redirect all unknown requests to the portal
        webServer->sendHeader("Location", "http://" + webServer->client().localIP().toString());
        webServer->send(302, "text/plain", "Redirect to portal");
    });
    LOGFV("endpoint setting completed");
}

void CaptivePortal::connectFromAPI(char* ssid, char* pass){
    if (this->connectToStation(ssid,pass)) {

            LOGFV("syncing time");
            syncTime();
            LOGD("Connection to the station success!");
            handleConnectionCallback(Connection::StaConnection::STA_CONNECTED);
            LOGFV("Saving the wifi credentials to eeprom");
            saveWifiCredentials(ssid,pass);

            LOGI("Connected to %s, ip %s" , ssid, WiFi.localIP());
            restartServer();
            LOGFI("Restarting server...");
            webServer->send(200, "text/plain", "success");
        } else {
            handleConnectionCallback(Connection::StaConnection::STA_CONNECTION_FAILURE);
            LOGI("Failed to connect to %s", ssid);
            delay(2000);
            connectFromAPI(ssid,pass);
            // webServer->send(200, "text/plain", "Failed to connect to " + ssid);
        }
}
void CaptivePortal::staEndPoints(){
    LOGFV("setting endpoint for sta model");
    webServer->onNotFound([this](){
        webServer->send(404, "text/plain", "Page not found");
    });

    webServer->on("/",HTTP_GET,[this](){
    webServer->send(200,"text/html","sta mode enabled!");
    });
}

bool CaptivePortal::connectToStation(char* ssid, char* pass){
        LOGFV("connection to sta called");

        WiFi.mode(WIFI_STA);
        WiFi.persistent(false);
        WiFi.setAutoReconnect(false);
        WiFi.setAutoConnect(false);

        LOGD("attempting connection with network, %s and pass %s",ssid, pass);

        WiFi.begin(ssid, pass);
        WiFi.printDiag(Serial);

        int retries = 0;
        const int maxRetries = 25; 

        while (WiFi.status() != WL_CONNECTED && retries < maxRetries) {
            delay(500);
            yield();
            // Serial.print(".");
            LOGD("Status: %d", WiFi.status());
            retries++;
        }

        // Serial.println();

        if (WiFi.status() == WL_CONNECTED) {
            LOGFI("device connected");
            return true;
        } else {
            LOGFI("failed to connect to device");
            
            WiFi.disconnect(true);
            delay(100);            
            LOGV("wifi mode is: %d", WiFi.getMode());
            WiFi.mode(WIFI_STA);
            return false;
        }
}
bool CaptivePortal::reconnect(){
    if (WiFi.getMode()==WIFI_STA && WiFi.status() != WL_CONNECTED) {
        if (millis() - lastReconnectAttempt >= RECONNECT_TIME_INTERVEL) {
            LOGFI("Attempting reconnect to known station...");
            if(connectToSavedStation()){
                LOGFV("connection success");
                restartServer();
                handleConnectionCallback(Connection::StaConnection::STA_CONNECTED);
            }else{
                LOGFV("connection failed");
                handleConnectionCallback(Connection::StaConnection::STA_DISCONNECTED);
            }
            LOGD("available free heap is: %d", ESP.getFreeHeap());
            lastReconnectAttempt = millis();
        }
    }

    return true;
}

bool CaptivePortal::connectToSavedStation(){

    if(!isWifiCredentialsExists()) return false;

    if(WiFi.status() != WL_CONNECTED){
        char* ssid = (char*)malloc(32);
        char* pass = (char*)malloc(64);
        LOGFV("allocated memory to ssid and pass");
        if (!ssid || !pass) {
            LOGFE("Memory allocation failed");
            free(ssid);
            free(pass);
            return false;
        }

        LOGFV("reading wifi credentials");
        readWifiCredentials(ssid,pass);
        LOGV("connection credential found, %s, %s", ssid, pass);

        bool result = connectToStation(ssid,pass);

        LOGV("connection state: %s ",result ? "connected" : "failed");

        if(result){
            syncTime();
        }

        free(ssid);
        free(pass);

        LOGFV("freed ssid and pass memory");
        return result;
    }

    return true;
}


void CaptivePortal::restartServer(){
    dnsServerRunning=false;
    LOGFV("stopping dnsserver");
    stop();
    if(webServer!=nullptr){
        LOGFV("stopping webserver");
        webServer->stop();
        webServer = nullptr;
        delete webServer;
    }

    if(!webServer){
        LOGFV("starting webserver");
        webServer = new ESP8266WebServer(80);
    }
    
    startServer();
}

void CaptivePortal::startServer(){

    if(isWifiCredentialsExists()){
        LOGFV("starting dns server as localIP");
        start(DNS_PORT,"*",WiFi.localIP());
        delay(1000);
        dnsServerRunning=true;
        LOGFI("starting server with sta mode");
        staEndPoints();
    }else{
        LOGFV("starting dns server as softApIP");
        start(DNS_PORT, "*", WiFi.softAPIP());
        delay(1000);
        dnsServerRunning=true;
        LOGFI("starting server with softAp mode");
        softApEndpoints();
    }

    LOGFV("webserver is setup and running");
    webServer->begin();
}

void CaptivePortal::nextRequestHandler()
{
    if(webServer!=nullptr && dnsServerRunning){
        
        processNextRequest();
        webServer->handleClient();

    }
}

