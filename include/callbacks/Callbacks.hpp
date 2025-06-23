#pragma once
#include <enums/Enums.hpp>
#include <FirebaseClient.h>

struct FirebaseCallbackResult
{
    //compulsary
    FirebaseEnum::FirebaseTask event;
    char * message;
    int code;
    bool isError;

    //optional [can be null at runtime]
    FirebaseApp app;

    FirebaseCallbackResult(FirebaseEnum::FirebaseTask t, char * msg, int c, bool error)
        : event(t), message(msg), code(c), isError(error){}
};

class ConnectionCallback
{
public:
virtual ~ConnectionCallback()=default;
virtual void onConnectionStateChange(Connection::StaConnection sta)=0;
};

class FirebaseStatusCallback{
    public:
    virtual ~FirebaseStatusCallback() = default;
    virtual void onFailure(FirebaseCallbackResult& result)=0;
    virtual void onSuccess(FirebaseCallbackResult& result)=0;

};

class FirebaseCallback{
    public:
    virtual ~FirebaseCallback()=default;
    virtual void onFailure(FirebaseCallbackResult& result){};
    virtual void onAuthSuccess(FirebaseCallbackResult& result){};
    
};

class FireStoreResultCallback{
    public:
        virtual ~FireStoreResultCallback()=default;

        virtual void onDeviceStateChange(bool state, bool reboot){};

};