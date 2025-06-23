#include <enums/Enums.hpp>

namespace FirebaseEnum {

    const char* toString(FirebaseTask task) {
        switch (task) {
            case FirebaseTask::SIGNUP:
                return "SIGNUPTASK";
            case FirebaseTask::LOGIN:
                return "LOGINTASK";
            default:
                return "UNKNOWNTASK";
        }
    }

    FirebaseTask toFirebaseTask(const char* task) {
        if (strcmp(task, "SIGNUPTASK") == 0) {
            return FirebaseTask::SIGNUP;
        } else if (strcmp(task, "LOGINTASK") == 0) {
            return FirebaseTask::LOGIN;
        } else {
            return FirebaseTask::UNKNOWN;
        }
    }
}
