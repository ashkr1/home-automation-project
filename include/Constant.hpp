#pragma once
#include <Arduino.h>
#include <Configs.hpp>

#ifdef ENABLE_CAPTIVE_MODE
    class HtmlTemplates {
        public:
            static const char HTML_CONTENT[];
            static const char JS[];
            static const char CSS[];
    };
#endif