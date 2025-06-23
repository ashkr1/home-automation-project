#include <FirestoreManager.hpp>

FirestoreManager::~FirestoreManager(){

}

void FirestoreManager::setFirebaseConfig(FirebaseApp* app,WiFiClientSecure& client ){
    this->app = app;
    this->client = client;
}

void FirestoreManager::setStateChangeListener(FireStoreResultCallback* callback){
    this->callback = callback;
}

String getISOTimestamp() {
  time_t now = time(nullptr);
  struct tm* t = gmtime(&now);
  char buf[30];
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", t);
  return String(buf);
}

bool FirestoreManager::initilizeFireStore(String email){
    LOGFV("initilizing firestore");
    bool status = true;
    if(!isFirestoreSetupDone()){
        LOGFV("firestore not registered in database from eeprom");
        JsonDocument* doc = createOrUpdateDocument("GET", "");
        
        if(doc==nullptr){
            LOGFV("firestore not registered, verified from cloud");
            if(registerNewDevice(email)){
                LOGFV("device registered on firebase complete");
                setFirestoreSetupState(true);
            }else{
                status = false;
                LOGFE("device registeretion failed");
            }
        }

        if(doc!=nullptr){
            delete doc;
        }
            doc = nullptr;
    }

    return status;
}

String FirestoreManager::deviceJsonBody(String name, String email, bool updateTimeStamp, bool hasState, bool state, bool hasRebot, bool reboot) {
  String jsonBody = "{\n  \"fields\": {\n";
  bool firstField = true;

  if (email.length() > 0) {
    jsonBody += "    \"email\": { \"stringValue\": \"" + email + "\" }";
    firstField = false;
  }

  if (name.length() > 0) {
    if (!firstField) jsonBody += ",\n";
    jsonBody += "    \"name\": { \"stringValue\": \"" + name + "\" }";
    firstField = false;
  }

  if (hasState) {
    if (!firstField) jsonBody += ",\n";
    jsonBody += "    \"state\": { \"booleanValue\": " + String(state ? "true" : "false") + " }";
  }

  if(updateTimeStamp){
    if (!firstField) jsonBody += ",\n";
    jsonBody+= " \"heartbeat\": { \"timestampValue\": \"" + getISOTimestamp() + "\" } ";
  }

  if(hasRebot){
    if (!firstField) jsonBody += ",\n";
    jsonBody += "    \"reboot\": { \"booleanValue\": " + String(reboot ? "true" : "false") + " }";
  }

  jsonBody += "\n  }\n}";
  return jsonBody;
}

bool FirestoreManager::verifyDocumentExist(JsonDocument doc){
    if(doc["code"].is<JsonVariant>()){
        return false;
    }
    return true;
}

void FirestoreManager::getDeviceInfo(){
    JsonDocument* doc= createOrUpdateDocument("GET","","state");
    if(doc){
        bool state=false, reboot=false;
        LOGFV("found state value");
        if((*doc)["fields"].is<JsonObject>()){
            JsonObject jo = (*doc)["fields"].as<JsonObject>();
            if(jo["state"].is<JsonObject>()){
                JsonObject sObj = jo["state"].as<JsonObject>();
                if(sObj["booleanValue"].is<bool>()){
                    state = sObj["booleanValue"].as<bool>();
                    LOGV("found state as %d",state);
                }
            }
            if(jo["reboot"].is<JsonObject>()){
                JsonObject sObj = jo["reboot"].as<JsonObject>();
                if(sObj["booleanValue"].is<bool>()){
                    reboot = sObj["booleanValue"].as<bool>();
                    LOGV("reboot state as %d",reboot);
                }
            }

            if(callback){
                callback->onDeviceStateChange(state,reboot);
            }
        }

        delete doc;
    }
    doc = nullptr;
}

void FirestoreManager::updateDeviceState(bool state){
    JsonDocument* doc = createOrUpdateDocument("PATCH",deviceJsonBody("","",true,true,state),"state");
    if(doc){
        delete doc;
    }
    doc = nullptr;
}

void FirestoreManager::updateHeartbeat(){
    LOGV("updating heartbeat, %s",getISOTimestamp());
    JsonDocument* doc = createOrUpdateDocument("PATCH",deviceJsonBody("","",true),"heartbeat");
    if(doc){
        LOGFV("details updated successfully");
        delete doc;
    }
    doc = nullptr;
}

void FirestoreManager::updateReboot(bool reboot){
    JsonDocument* doc = createOrUpdateDocument("PATCH",deviceJsonBody("","",true, false,false, true, reboot),"reboot");
    if(doc){
        LOGFV("details updated successfully");
        delete doc;
    }
    doc = nullptr;
}

bool FirestoreManager::registerNewDevice(String email){
    JsonDocument* doc = createOrUpdateDocument("PATCH", deviceJsonBody("Prime HomeLink", email,true, true, false));
    bool state = false;
    if(doc){

        state = true;

        delete doc;
    }

    doc = nullptr;

    return state;
}

String FirestoreManager::cleanJson(String input) {
  int start = input.indexOf('{');
  int end = input.lastIndexOf('}');

  if (start == -1 || end == -1 || start >= end) {
    // Invalid JSON format
    return "";
  }

  return input.substring(start, end + 1);
}


JsonDocument* FirestoreManager::createOrUpdateDocument(String requestType, String jsonBody, String mask) {

  const char* host = "firestore.googleapis.com";
  const int httpsPort = 443;

  LOGFV("Starting Firestore document creation");

  String masking="";
  if(requestType=="GET"){
    masking=(mask.length()>0? "&mask.fieldPaths=" + mask : "");
  }else{
    masking = (mask.length()>0? "&updateMask.fieldPaths=" + mask : "");
  }

  String path = "/v1/projects/" + String(FIREBASE_PROJECT_ID) + "/databases/(default)/documents/Devices/" + app->getUid()+"?key=" + FB_API_KEY +masking;

  LOGV("Connecting to host: %s", host);

  if (!client.connect(host, httpsPort)) {
    LOGFE(" 🔥 Connection to Firestore failed");
    return nullptr;
  }
  client.setTimeout(5000);
  LOGFV(" ✅ Connected to Firestore");

  LOGV(path.c_str());

    client.print(requestType);
    client.print(" ");
    client.print(path);
    client.print(F(" HTTP/1.1\r\n"));
    client.print(F("Host: firestore.googleapis.com\r\n"));
    client.print(F("Content-Type: application/json\r\n"));
    client.print(F("Authorization: Bearer "));
    client.print(app->getToken());
    client.print(F("\r\nContent-Length: "));
    client.print(String(jsonBody.length()));
    client.print(F("\r\n\r\n"));
    client.print(jsonBody);


  LOGFV("========== 🔽 BEGIN HTTP REQUEST 🔽 ==========");

  LOGFV("========== 🔼 END HTTP REQUEST 🔼 ==========");

//   client.print(request);

  LOGFV(" ✅ Request sent, awaiting response...");

  LOGFV("========== 🔽 RESPONSE HEADERS 🔽 ==========");
  
  String header = "";
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if(line.startsWith("HTTP/1.1")){
        header = line;
    }
    LOGV(line.c_str());
    if (line == "\r") break;
  }

  LOGFV("========== 🔼 END HEADERS 🔼 ==========");

  LOGFV("========== 🔽 RESPONSE BODY 🔽 ==========");
  JsonDocument* doc = new JsonDocument();
  String response=client.readString();
  String filterJson = cleanJson(response);
  LOGFV("========== 🔼 END BODY 🔼 ==========");

  client.stop();
  LOGFV(" 🔒 Connection closed");

  if(!header.startsWith("HTTP/1.1 200")){
    LOGFE("query failed!");
    if(doc)
        delete doc;
    doc = nullptr;
    LOGV(response.c_str());
    return nullptr;
  }

  DeserializationError error =  deserializeJson(*doc,filterJson);
  if (error) {
    LOGFE("error occured when deserializing");
    LOGE("deserializeJson() returned %s",error.c_str());
    if(doc)
        delete doc;
    doc = nullptr;
    return nullptr;
  }

  return doc;
}