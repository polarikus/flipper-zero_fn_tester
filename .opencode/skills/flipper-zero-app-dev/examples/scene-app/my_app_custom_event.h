#pragma once

// События приложения: не пересекаются с индексами пунктов меню,
// потому что обрабатываются в своих сценах.
typedef enum {
    MyAppCustomEventAbout,
    MyAppCustomEventExit,
} MyAppCustomEvent;
