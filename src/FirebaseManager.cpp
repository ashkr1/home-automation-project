#ifdef USING_FIREBASE_SERVER
#include <FirebaseManager.hpp>

WiFiClientSecure sslClient;

using AsyncClient = AsyncClientClass;
AsyncClient  aClient(sslClient);

FirebaseManager* FirebaseManager::INSTANCE = nullptr;

FirebaseStatusCallback* firebaseStatusCallback = nullptr;

FirebaseManager::FirebaseManager(){
    LOGFV("initilizing firbasemanage constructor");
    sslClient.setInsecure();
    sslClient.setBufferSizes(4096, 1024);

    LOGFV("attaching firebase status callback to firebaseManager");
    firebaseStatusCallback = this;
    setFirebaseConfig(&app,sslClient);

    LOGFV("firebasemanager initilization complete");
}

FirebaseManager::~FirebaseManager(){
    if(firebaseStatusCallback != nullptr){
        delete firebaseStatusCallback;
        firebaseStatusCallback = nullptr;
    }
}

FirebaseManager* FirebaseManager::getInstance(){
    if(INSTANCE==nullptr) INSTANCE = new FirebaseManager();
    return INSTANCE;
}

void FirebaseManager::onFailure(FirebaseCallbackResult& result){
    LOGE("FirebaseStatusFailed: %s, code: %d, msg: %s", FirebaseEnum::toString(result.event), result.code, result.message);
    handleStatusCallback(result);
}
void FirebaseManager::onSuccess(FirebaseCallbackResult& result){
    LOGFV("recieved call on success!");
        if(result.code == 10){
            if(result.event == FirebaseEnum::FirebaseTask::SIGNUP){
                setFirebaseLoginConfig(true);        
            }
            handleStatusCallback(result);
        }else{
            LOGV("app is not ready in onSuccess-> task:%s msg:%s, code: %d", FirebaseEnum::toString(result.event), result.message,result.code);        
        }
}

void FirebaseManager::asyncLoop(){
    app.loop();
}

void FirebaseManager::initilizeApp(){
    if(!isFirebaseLoginSet()){
        LOGFV("eeprom check user not exist");
        if(!verifyUserExist()){
            LOGFV("firebaseverification check user not exist");
            createUser();
            return;
        }
        LOGFV("firebaseverification user exist in firebase but not in eeprom");
        setFirebaseLoginConfig(true);
        LOGFV("marked user login in eeprom memory");
    }
    LOGFV("user already exist in eeprom, proceeding to login...");

    loginUser();
}

void FirebaseManager::initilizeFirestore(){
    if(initilizeFireStore(getDeviceFBEmail())){
        LOGFV("Firestore Initilization Complete");
    }
}

void FirebaseManager::createUser(){
    UserAccount user(FB_API_KEY);
    LOGV("signup username: %s, password: %s",getDeviceFBEmail().c_str(),getDeviceFBPass().c_str());
    signup(aClient,app,getAuth(user.email(getDeviceFBEmail()).password(getDeviceFBPass())),asyncFirebaseCallback,FirebaseEnum::toString(FirebaseEnum::FirebaseTask::SIGNUP));
}

void FirebaseManager::loginUser(){
    LOGV("Login with username: %s, password: %s",getDeviceFBEmail().c_str(),getDeviceFBPass().c_str());
    UserAuth userAuth(FB_API_KEY,getDeviceFBEmail(),getDeviceFBPass());
    initializeApp(aClient,app,getAuth(userAuth),asyncFirebaseCallback,FirebaseEnum::toString(FirebaseEnum::FirebaseTask::LOGIN));
}

bool FirebaseManager::verifyUserExist(){
    if (sslClient.connected())
        sslClient.stop();

    String host = "www.googleapis.com";
    bool ret = false;

    if (sslClient.connect(host.c_str(), 443) > 0)
    {
        String payload = F("{\"email\":\"");
        payload += getDeviceFBEmail();
        payload += F("\",\"password\":\"");
        payload += getDeviceFBPass();
        payload += F("\",\"returnSecureToken\":true}");

        String header = F("POST /identitytoolkit/v3/relyingparty/verifyPassword?key=");
        header += FB_API_KEY;
        header += F(" HTTP/1.1\r\n");
        header += F("Host: ");
        header += host;
        header += F("\r\n");
        header += F("Content-Type: application/json\r\n");
        header += F("Content-Length: ");
        header += payload.length();
        header += F("\r\n\r\n");

        if (sslClient.print(header) == header.length())
        {
            if (sslClient.print(payload) == payload.length())
            {
                unsigned long ms = millis();
                while (sslClient.connected() && sslClient.available() == 0 && millis() - ms < 5000)
                {
                    delay(1);
                }

                ms = millis();
                while (sslClient.connected() && sslClient.available() && millis() - ms < 5000)
                {
                    String line = sslClient.readStringUntil('\n');
                    if (line.length())
                    {
                        ret = line.indexOf("HTTP/1.1 200 OK") > -1;
                        LOGV("verification complete, user %s", ret?"exist":"not exist");
                        break;
                    }
                }
                sslClient.stop();
            }
        }
    }

    return ret;
}

void FirebaseManager::addStatusCallback(FirebaseCallback* callback){
    callbacks.push_back(callback);
}

void FirebaseManager::handleStatusCallback(FirebaseCallbackResult& result){
    LOGFV("entering in the handler callback");
    for(FirebaseCallback* callback : callbacks){
        if(callback){

            if(result.isError){
                LOGV("error found, task: %s",FirebaseEnum::toString(result.event));
                callback->onFailure(result);
                continue;
            }

            switch (result.event)
            {
            case FirebaseEnum::FirebaseTask::LOGIN:
            case FirebaseEnum::FirebaseTask::SIGNUP:
                result.app = app;
                LOGFV("found task login/signup, added app to the result obj");
                callback->onAuthSuccess(result);
                LOGFV("passed result to the callback");
                break;
            case FirebaseEnum::FirebaseTask::UNKNOWN:
                break;
            }
        }
    }
}

void FirebaseManager::asyncFirebaseCallback(AsyncResult &aResult)
{
    if (!aResult.isResult())
        return;

    if (aResult.isEvent())
    {   
        LOGV("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.eventLog().message().c_str(), aResult.eventLog().code());
        FirebaseCallbackResult result(FirebaseEnum::toFirebaseTask(aResult.uid().c_str()), (char *) aResult.eventLog().message().c_str(),aResult.eventLog().code(), false);
        LOGFV("created callback result object ");
        if(firebaseStatusCallback!=nullptr){
            firebaseStatusCallback->onSuccess(result);
        }
        else{
            LOGFE("callbackstatus found is null");
        }
        LOGFV("passed the result from isevent to callback");
    }

    if (aResult.isDebug())
    {
        LOGV("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());
    }

    if (aResult.isError())
    {
        LOGV("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());
        FirebaseCallbackResult result(FirebaseEnum::toFirebaseTask(aResult.uid().c_str()), (char *)aResult.eventLog().message().c_str(),aResult.error().code(), true);
        firebaseStatusCallback->onFailure(result);
    }

    if (aResult.available())
    {
        LOGV("task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
    }
}

String FirebaseManager::getDeviceFBEmail(){
    String chipIdHex = String(ESP.getChipId(), HEX);
    while (chipIdHex.length() < 8) {
        chipIdHex = "0" + chipIdHex;
    }
    return "device_" + chipIdHex + "@phl.com";
}

String FirebaseManager::getDeviceFBPass(){
    String deviceId = String(ESP.getChipId(), HEX);
    mac.replace(":", "");  
    mac.toLowerCase();
    String combined = mac + deviceId;
    return sha1(combined);
}

void FirebaseManager::setMacAddress(String mac){
    this->mac = mac;
}

#endif