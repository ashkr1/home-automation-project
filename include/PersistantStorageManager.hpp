#pragma once

#include <EEPROM.h>
#include <Configs.hpp>
#include <Logger.hpp>

class PersistantStorageManager
{
protected:
    void saveWifiCredentials(char* ssid, char* pass);
    bool readWifiCredentials(char* ssid, char* pass);
    bool isWifiCredentialsExists();
    bool isFirebaseLoginSet();
    void setFirebaseLoginConfig(bool isSet);
    void setFirestoreSetupState(bool isSet);
    bool isFirestoreSetupDone();
    void wipeAll();
public:
    PersistantStorageManager();
};
