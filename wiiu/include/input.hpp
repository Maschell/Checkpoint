#pragma once

#include <wut.h>
#include <vpad/input.h>
#include <padscore/kpad.h>
#include <padscore/wpad.h>

#include <vector>

typedef VPADTouchData touchPosition;

class Input
{
public:
    enum Buttons {
        BUTTON_NONE             = 0x0000,
        VPAD_TOUCH              = 0x80000000,
        BUTTON_Z                = 0x20000,
        BUTTON_C                = 0x10000,
        BUTTON_A                = 0x8000,
        BUTTON_B                = 0x4000,
        BUTTON_X                = 0x2000,
        BUTTON_Y                = 0x1000,
        BUTTON_1                = BUTTON_Y,
        BUTTON_2                = BUTTON_X,
        BUTTON_LEFT             = 0x0800,
        BUTTON_RIGHT            = 0x0400,
        BUTTON_UP               = 0x0200,
        BUTTON_DOWN             = 0x0100,
        BUTTON_ZL               = 0x0080,
        BUTTON_ZR               = 0x0040,
        BUTTON_L                = 0x0020,
        BUTTON_R                = 0x0010,
        BUTTON_PLUS             = 0x0008,
        BUTTON_MINUS            = 0x0004,
        BUTTON_HOME             = 0x0002,
        BUTTON_SYNC             = 0x0001,
        BUTTON_STICK_R          = 0x00020000,
        BUTTON_STICK_L          = 0x00040000,
        STICK_R_LEFT            = 0x04000000,
        STICK_R_RIGHT           = 0x02000000,
        STICK_R_UP              = 0x01000000,
        STICK_R_DOWN            = 0x00800000,
        STICK_L_LEFT            = 0x40000000,
        STICK_L_RIGHT           = 0x20000000,
        STICK_L_UP              = 0x10000000,
        STICK_L_DOWN            = 0x08000000,
    };

    static void initialize();
    static void finalize();
    static void update();
    static touchPosition getTouch();
    static uint32_t getHeld();
    static uint32_t getDown();

    static VPADStatus getVpad() { return getInstance().vpad; }
    static KPADStatus getKpad(int i) { return getInstance().kpads[i]; }

private:
    Input() { }
    ~Input() { }
    static Input& getInstance()
    {
        static Input instance;
        return instance;
    }
    VPADStatus vpad;
    std::vector<KPADStatus> kpads;
};