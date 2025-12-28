#include "KeyboardMouse.h"

KeyboardMouse::KeyboardMouse()
{
}

KeyboardMouse::~KeyboardMouse()
{
}

bool KeyboardMouse::SendKeyDown(WORD virtualKey)
{
    INPUT input = { 0 };
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = virtualKey;
    input.ki.dwFlags = 0; // Key down

    UINT result = SendInput(1, &input, sizeof(INPUT));
    return (result == 1);
}

bool KeyboardMouse::SendKeyUp(WORD virtualKey)
{
    INPUT input = { 0 };
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = virtualKey;
    input.ki.dwFlags = KEYEVENTF_KEYUP; // Key up

    UINT result = SendInput(1, &input, sizeof(INPUT));
    return (result == 1);
}

bool KeyboardMouse::SendKeyPress(WORD virtualKey)
{
    // Send key down, then key up
    if (!SendKeyDown(virtualKey))
    {
        return false;
    }
    
    return SendKeyUp(virtualKey);
}

bool KeyboardMouse::SendMouseButtonDown(int button)
{
    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = GetMouseButtonFlag(button);

    UINT result = SendInput(1, &input, sizeof(INPUT));
    return (result == 1);
}

bool KeyboardMouse::SendMouseButtonUp(int button)
{
    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    
    // Get the corresponding up flag for the button
    DWORD flag = 0;
    switch (button)
    {
    case 0: flag = MOUSEEVENTF_LEFTUP; break;
    case 1: flag = MOUSEEVENTF_RIGHTUP; break;
    case 2: flag = MOUSEEVENTF_MIDDLEUP; break;
    default: return false;
    }
    
    input.mi.dwFlags = flag;

    UINT result = SendInput(1, &input, sizeof(INPUT));
    return (result == 1);
}

bool KeyboardMouse::SendMouseMove(int deltaX, int deltaY)
{
    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dx = deltaX;
    input.mi.dy = deltaY;
    input.mi.dwFlags = MOUSEEVENTF_MOVE;

    UINT result = SendInput(1, &input, sizeof(INPUT));
    return (result == 1);
}

DWORD KeyboardMouse::GetMouseButtonFlag(int button) const
{
    switch (button)
    {
    case 0: return MOUSEEVENTF_LEFTDOWN;
    case 1: return MOUSEEVENTF_RIGHTDOWN;
    case 2: return MOUSEEVENTF_MIDDLEDOWN;
    default: return 0;
    }
}

