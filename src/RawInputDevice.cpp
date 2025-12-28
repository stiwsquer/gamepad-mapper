#include "RawInputDevice.h"
#include <cstring>

// HID usage pages and usages for Xbox controller buttons
#define HID_USAGE_PAGE_GENERIC 0x01
#define HID_USAGE_GENERIC_GAMEPAD 0x05
#define HID_USAGE_GENERIC_BUTTON_1 0x01
#define HID_USAGE_GENERIC_BUTTON_2 0x02
#define HID_USAGE_GENERIC_BUTTON_3 0x03
#define HID_USAGE_GENERIC_BUTTON_4 0x04

RawInputDevice::RawInputDevice()
    : m_hwnd(nullptr)
    , m_isConnected(false)
{
    std::memset(m_currentButtons, 0, sizeof(m_currentButtons));
    std::memset(m_previousButtons, 0, sizeof(m_previousButtons));
}

RawInputDevice::~RawInputDevice()
{
}

bool RawInputDevice::Initialize(HWND hwnd)
{
    if (!hwnd)
    {
        return false;
    }

    m_hwnd = hwnd;

    // Register for Raw Input from game controllers
    RAWINPUTDEVICE rid[1];
    
    rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
    rid[0].usUsage = HID_USAGE_GENERIC_GAMEPAD;
    rid[0].dwFlags = RIDEV_INPUTSINK; // Receive input even when not in foreground
    rid[0].hwndTarget = hwnd;

    BOOL result = RegisterRawInputDevices(rid, 1, sizeof(RAWINPUTDEVICE));
    
    if (!result)
    {
        return false;
    }

    m_isConnected = true;
    return true;
}

bool RawInputDevice::ProcessRawInput(LPARAM lParam)
{
    UINT dwSize = 0;
    
    // Get the size of the Raw Input data
    if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER)) == (UINT)-1)
    {
        return false;
    }
    
    if (dwSize == 0)
    {
        return false;
    }

    // Allocate buffer for Raw Input data
    LPBYTE lpb = new BYTE[dwSize];
    if (!lpb)
    {
        return false;
    }

    // Get the Raw Input data
    if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) == (UINT)-1)
    {
        delete[] lpb;
        return false;
    }

    RAWINPUT* raw = (RAWINPUT*)lpb;

    // Check if this is a HID device (game controller)
    if (raw->header.dwType == RIM_TYPEHID)
    {
        RAWHID* pHid = &raw->data.hid;
        
        // Xbox controllers send variable length HID reports
        // We need to parse the button data from the HID report
        if (pHid->dwSizeHid > 0 && pHid->bRawData)
        {
            BYTE* pData = (BYTE*)pHid->bRawData;
            DWORD dataSize = pHid->dwSizeHid;
            
            // Xbox 360/One controller HID format varies, but buttons are usually in the first few bytes
            // Try to find button data - common formats:
            // Format 1: Buttons at offset 4-5 (Xbox 360)
            // Format 2: Buttons at offset 0-1 (some controllers)
            
            if (dataSize >= 6)
            {
                // Try common Xbox controller format: buttons at byte 4-5
                BYTE buttons1 = pData[4];
                BYTE buttons2 = (dataSize >= 6) ? pData[5] : 0;
                
                // Update button states
                m_currentButtons[BUTTON_A] = (buttons1 & 0x01) != 0;
                m_currentButtons[BUTTON_B] = (buttons1 & 0x02) != 0;
                m_currentButtons[BUTTON_X] = (buttons1 & 0x04) != 0;
                m_currentButtons[BUTTON_Y] = (buttons1 & 0x08) != 0;
                m_currentButtons[BUTTON_LB] = (buttons1 & 0x10) != 0;
                m_currentButtons[BUTTON_RB] = (buttons1 & 0x20) != 0;
                m_currentButtons[BUTTON_BACK] = (buttons1 & 0x40) != 0;
                m_currentButtons[BUTTON_START] = (buttons1 & 0x80) != 0;
                
                if (dataSize >= 6)
                {
                    m_currentButtons[BUTTON_LS] = (buttons2 & 0x01) != 0;
                    m_currentButtons[BUTTON_RS] = (buttons2 & 0x02) != 0;
                }
                
                m_isConnected = true;
            }
            else if (dataSize >= 2)
            {
                // Alternative format: buttons might be at offset 0-1
                BYTE buttons1 = pData[0];
                
                m_currentButtons[BUTTON_A] = (buttons1 & 0x01) != 0;
                m_currentButtons[BUTTON_B] = (buttons1 & 0x02) != 0;
                m_currentButtons[BUTTON_X] = (buttons1 & 0x04) != 0;
                m_currentButtons[BUTTON_Y] = (buttons1 & 0x08) != 0;
                
                m_isConnected = true;
            }
        }
    }

    delete[] lpb;
    return true;
}

bool RawInputDevice::IsButtonPressed(int button) const
{
    if (button < 0 || button >= 16)
    {
        return false;
    }
    
    return m_currentButtons[button];
}

bool RawInputDevice::IsButtonJustPressed(int button) const
{
    if (button < 0 || button >= 16)
    {
        return false;
    }
    
    return !m_previousButtons[button] && m_currentButtons[button];
}

bool RawInputDevice::IsButtonJustReleased(int button) const
{
    if (button < 0 || button >= 16)
    {
        return false;
    }
    
    return m_previousButtons[button] && !m_currentButtons[button];
}

void RawInputDevice::Update()
{
    // Store previous button states for transition detection
    std::memcpy(m_previousButtons, m_currentButtons, sizeof(m_currentButtons));
}

int RawInputDevice::GetButtonIndex(USHORT usagePage, USHORT usage) const
{
    // Map HID usage to button index
    if (usagePage == HID_USAGE_PAGE_GENERIC)
    {
        switch (usage)
        {
        case HID_USAGE_GENERIC_BUTTON_1: return BUTTON_A;
        case HID_USAGE_GENERIC_BUTTON_2: return BUTTON_B;
        case HID_USAGE_GENERIC_BUTTON_3: return BUTTON_X;
        case HID_USAGE_GENERIC_BUTTON_4: return BUTTON_Y;
        }
    }
    return -1;
}

bool RawInputDevice::IsXboxController(const RAWINPUTHEADER& header) const
{
    return header.dwType == RIM_TYPEHID;
}

