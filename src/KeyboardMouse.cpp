#include "KeyboardMouse.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <cwchar>

KeyboardMouse::KeyboardMouse()
    : m_cachedGameWindow(nullptr)
    , m_lastWindowCheckTime(0)
{
}

KeyboardMouse::~KeyboardMouse()
{
}

bool KeyboardMouse::SendKeyDown(WORD virtualKey)
{
    // Method 1: Try SendInput with scan codes first (most reliable for games)
    UINT scanCode = MapVirtualKey(virtualKey, MAPVK_VK_TO_VSC);
    if (scanCode != 0)
    {
        INPUT input = { 0 };
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = 0; // Use scan code
        input.ki.wScan = scanCode;
        input.ki.dwFlags = KEYEVENTF_SCANCODE;
        input.ki.time = 0;
        input.ki.dwExtraInfo = 0;

        UINT result = SendInput(1, &input, sizeof(INPUT));
        if (result == 1)
        {
            return true;
        }
    }

    // Method 2: Try standard SendInput with virtual key
    INPUT input2 = { 0 };
    input2.type = INPUT_KEYBOARD;
    input2.ki.wVk = virtualKey;
    input2.ki.dwFlags = 0;
    input2.ki.time = 0;
    input2.ki.dwExtraInfo = 0;

    UINT result2 = SendInput(1, &input2, sizeof(INPUT));
    if (result2 == 1)
    {
        return true;
    }

    // Method 3: Try window messages
    HWND gameWindow = GetGameWindow();
    if (gameWindow != nullptr)
    {
        if (SendKeyToWindow(gameWindow, virtualKey, true))
        {
            return true;
        }
    }
    
    // Fallback to foreground window
    HWND fgWindow = GetForegroundWindow();
    if (fgWindow != nullptr)
    {
        return SendKeyToWindow(fgWindow, virtualKey, true);
    }
    
    // Method 4: Fallback to keybd_event
    return SendKeyEvent(virtualKey, true);
}

bool KeyboardMouse::SendKeyUp(WORD virtualKey)
{
    // Method 1: Try SendInput with scan codes first
    UINT scanCode = MapVirtualKey(virtualKey, MAPVK_VK_TO_VSC);
    if (scanCode != 0)
    {
        INPUT input = { 0 };
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = 0;
        input.ki.wScan = scanCode;
        input.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
        input.ki.time = 0;
        input.ki.dwExtraInfo = 0;

        UINT result = SendInput(1, &input, sizeof(INPUT));
        if (result == 1)
        {
            return true;
        }
    }

    // Method 2: Try standard SendInput
    INPUT input2 = { 0 };
    input2.type = INPUT_KEYBOARD;
    input2.ki.wVk = virtualKey;
    input2.ki.dwFlags = KEYEVENTF_KEYUP;
    input2.ki.time = 0;
    input2.ki.dwExtraInfo = 0;

    UINT result2 = SendInput(1, &input2, sizeof(INPUT));
    if (result2 == 1)
    {
        return true;
    }

    // Method 3: Try window messages
    HWND gameWindow = GetGameWindow();
    if (gameWindow != nullptr)
    {
        if (SendKeyToWindow(gameWindow, virtualKey, false))
        {
            return true;
        }
    }
    
    // Fallback to foreground window
    HWND fgWindow = GetForegroundWindow();
    if (fgWindow != nullptr)
    {
        return SendKeyToWindow(fgWindow, virtualKey, false);
    }
    
    // Method 4: Fallback to keybd_event
    return SendKeyEvent(virtualKey, false);
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

bool KeyboardMouse::SendKeyEvent(WORD virtualKey, bool keyDown)
{
    // keybd_event is deprecated but sometimes works when SendInput doesn't
    // It's less reliable but can bypass some input blocking
    DWORD flags = keyDown ? 0 : KEYEVENTF_KEYUP;
    keybd_event(static_cast<BYTE>(virtualKey), 0, flags, 0);
    return true; // keybd_event doesn't return error codes
}

bool KeyboardMouse::SendKeyToWindow(HWND hWnd, WORD virtualKey, bool keyDown)
{
    if (hWnd == nullptr)
    {
        hWnd = GetForegroundWindow();
    }

    if (hWnd == nullptr)
    {
        return false;
    }

    // Try multiple methods since the game blocks standard input
    
    // Method 1: Try SendInput with scan codes (sometimes works when virtual keys don't)
    UINT scanCode = MapVirtualKey(virtualKey, MAPVK_VK_TO_VSC);
    if (scanCode != 0)
    {
        INPUT input = { 0 };
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = 0; // Use scan code instead
        input.ki.wScan = scanCode;
        input.ki.dwFlags = KEYEVENTF_SCANCODE;
        if (!keyDown)
        {
            input.ki.dwFlags |= KEYEVENTF_KEYUP;
        }
        input.ki.time = 0;
        input.ki.dwExtraInfo = 0;

        UINT result = SendInput(1, &input, sizeof(INPUT));
        if (result == 1)
        {
            return true;
        }
    }

    // Method 2: Try window messages with extended key flag
    DWORD targetThreadId = GetWindowThreadProcessId(hWnd, nullptr);
    DWORD currentThreadId = GetCurrentThreadId();
    
    bool attached = false;
    if (targetThreadId != currentThreadId)
    {
        attached = AttachThreadInput(currentThreadId, targetThreadId, TRUE) != FALSE;
    }

    // Map virtual key to scan code
    UINT scanCode2 = MapVirtualKey(virtualKey, MAPVK_VK_TO_VSC);
    
    // Create lParam for WM_KEYDOWN/WM_KEYUP with extended key flag
    LPARAM lParam = (scanCode2 << 16);
    
    // Check if it's an extended key (right alt, ctrl, etc.)
    bool isExtended = (virtualKey == VK_RMENU || virtualKey == VK_RCONTROL || 
                       virtualKey == VK_RSHIFT || virtualKey == VK_INSERT || 
                       virtualKey == VK_DELETE || virtualKey == VK_HOME || 
                       virtualKey == VK_END || virtualKey == VK_PRIOR || 
                       virtualKey == VK_NEXT || virtualKey == VK_LEFT || 
                       virtualKey == VK_RIGHT || virtualKey == VK_UP || 
                       virtualKey == VK_DOWN || virtualKey == VK_NUMPAD0 || 
                       virtualKey == VK_NUMPAD1 || virtualKey == VK_NUMPAD2 || 
                       virtualKey == VK_NUMPAD3 || virtualKey == VK_NUMPAD4 || 
                       virtualKey == VK_NUMPAD5 || virtualKey == VK_NUMPAD6 || 
                       virtualKey == VK_NUMPAD7 || virtualKey == VK_NUMPAD8 || 
                       virtualKey == VK_NUMPAD9);
    
    if (isExtended)
    {
        lParam |= (1 << 24); // Extended key flag
    }
    
    if (!keyDown)
    {
        lParam |= (1 << 30); // Previous key state
        lParam |= (1 << 31); // Transition state
    }

    // Try multiple message types
    UINT message = keyDown ? WM_KEYDOWN : WM_KEYUP;
    
    // Try SendMessage
    SendMessage(hWnd, message, virtualKey, lParam);
    
    // Try PostMessage (asynchronous, sometimes works better)
    PostMessage(hWnd, message, virtualKey, lParam);
    
    // Also try WM_SYSKEYDOWN/WM_SYSKEYUP for system keys
    if (virtualKey == VK_CONTROL || virtualKey == VK_SHIFT || virtualKey == VK_MENU)
    {
        UINT sysMessage = keyDown ? WM_SYSKEYDOWN : WM_SYSKEYUP;
        PostMessage(hWnd, sysMessage, virtualKey, lParam);
    }
    
    // Send WM_CHAR for printable characters
    if (keyDown)
    {
        if (virtualKey >= 'A' && virtualKey <= 'Z')
        {
            WPARAM charCode = virtualKey + 32; // 'A' -> 'a'
            PostMessage(hWnd, WM_CHAR, charCode, lParam);
        }
        else if (virtualKey >= '0' && virtualKey <= '9')
        {
            PostMessage(hWnd, WM_CHAR, virtualKey, lParam);
        }
        else if (virtualKey == VK_RETURN)
        {
            PostMessage(hWnd, WM_CHAR, 13, lParam); // Carriage return
        }
    }

    // Detach if we attached
    if (attached)
    {
        AttachThreadInput(currentThreadId, targetThreadId, FALSE);
    }

    return true; // Return true - we tried multiple methods
}

HWND KeyboardMouse::GetGameWindow()
{
    DWORD currentTime = GetTickCount();
    
    // Re-check window every few seconds in case it was closed/reopened
    if (m_cachedGameWindow == nullptr || 
        (currentTime - m_lastWindowCheckTime) > WINDOW_CACHE_TIMEOUT_MS)
    {
        // Try Polish title first
        m_cachedGameWindow = FindWindowByTitle("Wiedźmin");
        if (m_cachedGameWindow == nullptr)
        {
            // Try English title
            m_cachedGameWindow = FindWindowByTitle("Witcher");
        }
        
        m_lastWindowCheckTime = currentTime;
        
        #ifdef _DEBUG
        if (m_cachedGameWindow != nullptr)
        {
            wchar_t title[256];
            GetWindowTextW(m_cachedGameWindow, title, sizeof(title) / sizeof(wchar_t));
            std::wcout << L"Found game window: " << title << std::endl;
        }
        #endif
    }
    
    // Verify the window still exists
    if (m_cachedGameWindow != nullptr && !IsWindow(m_cachedGameWindow))
    {
        m_cachedGameWindow = nullptr;
    }
    
    return m_cachedGameWindow;
}

HWND KeyboardMouse::FindWindowByTitle(const char* titlePart)
{
    struct WindowSearchData {
        HWND foundWindow;
        const char* searchText;
        bool debug;
    };

    WindowSearchData searchData = { nullptr, titlePart, false };

    // Convert search string to wide string for proper Unicode handling
    int searchLen = MultiByteToWideChar(CP_UTF8, 0, titlePart, -1, nullptr, 0);
    std::vector<wchar_t> searchWide(searchLen);
    MultiByteToWideChar(CP_UTF8, 0, titlePart, -1, searchWide.data(), searchLen);
    std::wstring searchLowerW = searchWide.data();
    std::transform(searchLowerW.begin(), searchLowerW.end(), searchLowerW.begin(), ::towlower);

    // Store search string in a way accessible to the callback
    struct SearchContext {
        WindowSearchData* data;
        std::wstring* searchStr;
    };
    SearchContext context = { &searchData, &searchLowerW };

    // Enumerate all top-level windows using wide character functions
    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        SearchContext* ctx = reinterpret_cast<SearchContext*>(lParam);
        wchar_t title[256];
        if (GetWindowTextW(hwnd, title, sizeof(title) / sizeof(wchar_t)) > 0 && wcslen(title) > 0)
        {
            // Convert to lowercase for case-insensitive comparison
            std::wstring titleW = title;
            std::transform(titleW.begin(), titleW.end(), titleW.begin(), ::towlower);

            if (titleW.find(*ctx->searchStr) != std::wstring::npos)
            {
                // Found matching window
                ctx->data->foundWindow = hwnd;
                return FALSE; // Stop enumeration
            }
        }
        return TRUE; // Continue enumeration
    }, reinterpret_cast<LPARAM>(&context));

    return searchData.foundWindow;
}

