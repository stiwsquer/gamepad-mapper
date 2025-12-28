#include "KeyboardMouse.h"

KeyboardMouse::KeyboardMouse()
{
}

KeyboardMouse::~KeyboardMouse()
{
}

bool KeyboardMouse::SendKeyDown(WORD virtualKey)
{
    // Method 1: Try scan code method first (most reliable - bypasses some protections)
    if (SendKeyDownScanCode(virtualKey))
    {
        return true;
    }
    
    // Method 2: Fallback to virtual key method with SendInput
    INPUT input = { 0 };
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = virtualKey;
    input.ki.dwFlags = 0; // Key down

    UINT result = SendInput(1, &input, sizeof(INPUT));
    if (result == 1)
    {
        return true;
    }
    
    // Method 3: Final fallback to keybd_event (older API, sometimes works when SendInput doesn't)
    SendKeyEvent(virtualKey, true);
    return true;
}

bool KeyboardMouse::SendKeyUp(WORD virtualKey)
{
    // Method 1: Try scan code method first (most reliable - bypasses some protections)
    if (SendKeyUpScanCode(virtualKey))
    {
        return true;
    }
    
    // Method 2: Fallback to virtual key method with SendInput
    INPUT input = { 0 };
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = virtualKey;
    input.ki.dwFlags = KEYEVENTF_KEYUP; // Key up

    UINT result = SendInput(1, &input, sizeof(INPUT));
    if (result == 1)
    {
        return true;
    }
    
    // Method 3: Final fallback to keybd_event (older API, sometimes works when SendInput doesn't)
    SendKeyEvent(virtualKey, false);
    return true;
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

bool KeyboardMouse::SendKeyDownScanCode(WORD virtualKey)
{
    // Get scan code from virtual key
    WORD scanCode = MapVirtualKey(virtualKey, MAPVK_VK_TO_VSC);
    if (scanCode == 0)
    {
        return false;
    }
    
    INPUT input = { 0 };
    input.type = INPUT_KEYBOARD;
    input.ki.wScan = scanCode;
    input.ki.dwFlags = KEYEVENTF_SCANCODE; // Use scan code instead of virtual key
    
    UINT result = SendInput(1, &input, sizeof(INPUT));
    return (result == 1);
}

bool KeyboardMouse::SendKeyUpScanCode(WORD virtualKey)
{
    // Get scan code from virtual key
    WORD scanCode = MapVirtualKey(virtualKey, MAPVK_VK_TO_VSC);
    if (scanCode == 0)
    {
        return false;
    }
    
    INPUT input = { 0 };
    input.type = INPUT_KEYBOARD;
    input.ki.wScan = scanCode;
    input.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP; // Use scan code + key up
    
    UINT result = SendInput(1, &input, sizeof(INPUT));
    return (result == 1);
}

bool KeyboardMouse::SendKeyEvent(WORD virtualKey, bool keyDown)
{
    // Use keybd_event as a fallback (older API, sometimes works when SendInput doesn't)
    if (keyDown)
    {
        keybd_event((BYTE)virtualKey, 0, 0, 0);
    }
    else
    {
        keybd_event((BYTE)virtualKey, 0, KEYEVENTF_KEYUP, 0);
    }
    return true;
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

