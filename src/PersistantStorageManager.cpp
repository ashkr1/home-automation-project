#include <PersistantStorageManager.hpp>

PersistantStorageManager::PersistantStorageManager(){
    EEPROM.begin(EEPROM_SIZE);
}

void PersistantStorageManager::saveWifiCredentials(char* ssid, char* pass){
    int i=0;
    for(; i<strlen(ssid);i++){
        EEPROM.write(SSID_ADDR + i, ssid[i]);
    }
    EEPROM.write(SSID_ADDR + i, '\0');
    
    i=0;
    for(; i<strlen(pass);i++){
        EEPROM.write(PASS_ADDR + i,  pass[i]);
    }
    EEPROM.write(PASS_ADDR + i,  '\0');


    EEPROM.write(FLAG_ADDR, 0xAA);
    EEPROM.commit();
}

bool PersistantStorageManager::readWifiCredentials(char* ssid, char* pass){

    if(!isWifiCredentialsExists()){
        LOGW("ssid and pass does not exist!");
        return false;
    }

    for (int i = 0; i < 32; i++) {
        ssid[i] = EEPROM.read(SSID_ADDR + i);
        if(ssid[i]=='\0') break;
    }

    for(int i=0; i<64;i++){
        pass[i] = EEPROM.read(PASS_ADDR + i);
        if(pass[i]=='\0') break;
    }
  return true;
}

bool PersistantStorageManager::isWifiCredentialsExists(){
    if (EEPROM.read(FLAG_ADDR) != 0xAA) {
        LOGFI("Wifi ssid and password does not exist");
        return false; 
    }
    return true;
}

bool PersistantStorageManager::isFirebaseLoginSet(){
    if (EEPROM.read(FLAG_FB_SETUP) != 0x1) {
        uint8_t val = EEPROM.read(FLAG_FB_SETUP);
        return false; 
    }
    return true;
}

void PersistantStorageManager::setFirebaseLoginConfig(bool isSet){
    if(isSet){
        EEPROM.write(FLAG_FB_SETUP, 0x1);
    }else{
        EEPROM.write(FLAG_FB_SETUP, 0x0);
    }
    EEPROM.commit();
}

void PersistantStorageManager::setFirestoreSetupState(bool isSet){
    if(isSet){
        EEPROM.write(FLAG_FIRESTORE_SETUP,0x1);
    }else{
        EEPROM.write(FLAG_FIRESTORE_SETUP, 0x0);
    }
    EEPROM.commit();
}

bool PersistantStorageManager::isFirestoreSetupDone(){
    if (EEPROM.read(FLAG_FIRESTORE_SETUP) != 0x1) {
        uint8_t val = EEPROM.read(FLAG_FIRESTORE_SETUP);
        return false; 
    }
    return true;
}

void PersistantStorageManager::wipeAll() {
    for (int i = 0; i < EEPROM_SIZE; i++) {
        EEPROM.write(i, 0xFF);
    }
    EEPROM.commit();
    Serial.println("⚠️ All EEPROM data wiped.");
}
