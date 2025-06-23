#pragma once

#include <cstring>
namespace Connection{
    enum StaConnection{
        STA_CONNECTED,
        STA_DISCONNECTED,
        STA_CONNECTION_FAILURE
    };
}

namespace FirebaseEnum
{
    enum FirebaseTask{
        SIGNUP,
        LOGIN,
        UNKNOWN
    };

    const char* toString(FirebaseTask task);

    FirebaseTask toFirebaseTask(const char*  task);
}