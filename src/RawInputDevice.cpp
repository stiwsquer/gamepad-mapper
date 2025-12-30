#include "RawInputDevice.h"
#include <cstring>
#include <string>
#include <iostream>
#include <cstdio>
#include <algorithm>
#include <cctype>
#include <setupapi.h>
#include <initguid.h>
#include <devguid.h>

// HID usage pages and usages for Xbox controller buttons
#define HID_USAGE_PAGE_GENERIC 0x01
#define HID_USAGE_GENERIC_GAMEPAD 0x05
#define HID_USAGE_GENERIC_BUTTON_1 0x01
#define HID_USAGE_GENERIC_BUTTON_2 0x02
#define HID_USAGE_GENERIC_BUTTON_3 0x03
#define HID_USAGE_GENERIC_BUTTON_4 0x04

// Xbox controller vendor IDs
#define VENDOR_ID_MICROSOFT 0x045E
#define VENDOR_ID_XBOX_360_WIRED 0x028E
#define VENDOR_ID_XBOX_360_WIRELESS 0x0291

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

    // Register for Raw Input from ALL HID devices (Xbox controllers may not report as gamepad)
    // We'll filter for game controllers in ProcessRawInput
    RAWINPUTDEVICE rid[2];
    
    // Register for generic gamepad
    rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
    rid[0].usUsage = HID_USAGE_GENERIC_GAMEPAD;
    rid[0].dwFlags = RIDEV_INPUTSINK; // Receive input even when not in foreground
    rid[0].hwndTarget = hwnd;
    
    // Also register for joystick (Xbox controllers sometimes report as joystick)
    rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
    rid[1].usUsage = 0x04; // Joystick
    rid[1].dwFlags = RIDEV_INPUTSINK;
    rid[1].hwndTarget = hwnd;

    BOOL result = RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE));
    
    if (!result)
    {
        DWORD error = GetLastError();
        // Continue anyway - registration might partially work
    }

    // Don't set connected yet - wait for first input
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
        // Get device info to check if it's an Xbox controller
        UINT deviceInfoSize = sizeof(RAWINPUTDEVICELIST);
        UINT deviceCount = 1;
        RAWINPUTDEVICELIST deviceList;
        
        // Get device handle from the raw input
        HANDLE hDevice = raw->header.hDevice;
        
        // Try to get device info
        UINT nameSize = 0;
        if (GetRawInputDeviceInfo(hDevice, RIDI_DEVICENAME, nullptr, &nameSize) == (UINT)-1)
        {
            // Can't get device info, but continue anyway
        }
        
        // Check if device name contains Xbox identifiers
        bool isXboxController = false;
        static bool hasShownDeviceInfo = false;
        if (nameSize > 0)
        {
            WCHAR* deviceName = new WCHAR[nameSize];
            if (GetRawInputDeviceInfo(hDevice, RIDI_DEVICENAME, deviceName, &nameSize) != (UINT)-1)
            {
                // Check for Xbox controller identifiers in device name
                std::wstring name(deviceName);
                
                // Convert to uppercase for case-insensitive comparison
                std::wstring nameUpper = name;
                std::transform(nameUpper.begin(), nameUpper.end(), nameUpper.begin(), ::towupper);
                
                // Debug: Show device name once to help diagnose issues
                if (!hasShownDeviceInfo)
                {
                    std::wcout << L"Raw Input: Detected HID device: " << deviceName << std::endl;
                    hasShownDeviceInfo = true;
                }
                
                // Check for Xbox controller by Vendor ID and Product ID
                // VID_045E = Microsoft, common PIDs: 02A1, 02DD, 02E0, 02EA, 02FD, etc.
                // VID_028E = Xbox 360 wired
                // VID_0291 = Xbox 360 wireless
                bool hasMicrosoftVID = nameUpper.find(L"VID_045E") != std::wstring::npos;
                bool hasXbox360VID = nameUpper.find(L"VID_028E") != std::wstring::npos || 
                                     nameUpper.find(L"VID_0291") != std::wstring::npos;
                bool hasXboxKeyword = nameUpper.find(L"XBOX") != std::wstring::npos;
                
                // If it's a Microsoft device, check if it's likely a game controller
                // Common Xbox controller PIDs start with 02
                bool hasGameControllerPID = nameUpper.find(L"PID_02") != std::wstring::npos;
                
                if (hasXbox360VID || hasXboxKeyword || (hasMicrosoftVID && hasGameControllerPID))
                {
                    isXboxController = true;
                    if (!m_isConnected)
                    {
                        std::cout << "Raw Input: Xbox controller detected!" << std::endl;
                    }
                }
                else
                {
                    // Not an Xbox controller - this is expected for other HID devices
                    // Only show once to avoid spam
                    static bool hasShownNonXboxWarning = false;
                    if (!hasShownNonXboxWarning && !m_isConnected)
                    {
                        std::cout << "Raw Input: Non-Xbox HID device detected (will be ignored)" << std::endl;
                        hasShownNonXboxWarning = true;
                    }
                }
            }
            delete[] deviceName;
        }
        
        RAWHID* pHid = &raw->data.hid;
        
        if (pHid->dwSizeHid > 0 && pHid->bRawData)
        {
            BYTE* pData = (BYTE*)pHid->bRawData;
            DWORD dataSize = pHid->dwSizeHid;
            
            // CRITICAL: Only process if we're certain it's an Xbox controller
            // This prevents false positives from other HID devices
            if (isXboxController)
            {
                // Debug: Show data format and button parsing attempts
                static bool hasShownDataFormat = false;
                static DWORD lastDebugTime = 0;
                DWORD currentTick = GetTickCount();
                
                if (!hasShownDataFormat)
                {
                    std::cout << "Raw Input: Xbox controller data detected! Report size: " << dataSize << " bytes" << std::endl;
                    std::cout << "Raw Input: First 16 bytes (hex): ";
                    for (DWORD i = 0; i < dataSize && i < 16; i++)
                    {
                        printf("%02X ", pData[i]);
                    }
                    std::cout << std::endl;
                    hasShownDataFormat = true;
                }
                
                // Debug: Show what we're parsing when buttons change
                static BYTE lastButtonByte = 0;
                BYTE currentButtonByte = (dataSize > 4) ? pData[4] : 0;
                if (currentButtonByte != lastButtonByte && (currentTick - lastDebugTime > 100))
                {
                    std::cout << "Raw Input: Button byte at offset 4 = 0x" << std::hex << (int)currentButtonByte << std::dec;
                    std::cout << " (binary: ";
                    for (int bit = 7; bit >= 0; bit--)
                    {
                        std::cout << ((currentButtonByte >> bit) & 1);
                    }
                    std::cout << ")" << std::endl;
                    lastButtonByte = currentButtonByte;
                    lastDebugTime = currentTick;
                }
                
                bool buttonsFound = false;
                
                // Xbox controllers via Raw Input send 16-byte reports
                // Based on the hex dump analysis:
                // When A is pressed: byte 10 = 0x80, byte 11 = 0x01
                // When released: byte 10 = 0x00, byte 11 = 0x00 (or different values)
                // This suggests buttons might be encoded in bytes 10-11
                
                // Format 1: Try bytes 10-11 for button data (most likely based on hex dump)
                if (dataSize >= 12)
                {
                    BYTE byte10 = pData[10];
                    BYTE byte11 = pData[11];
                    
                    // Debug: Show these bytes when they change significantly
                    static BYTE lastByte10 = 0, lastByte11 = 0;
                    static DWORD lastDebugTime2 = 0;
                    DWORD currentTick2 = GetTickCount();
                    if ((byte10 != lastByte10 || byte11 != lastByte11) && (currentTick2 - lastDebugTime2 > 200))
                    {
                        std::cout << "Raw Input: Bytes 10-11: 0x" << std::hex << (int)byte10 << " " << (int)byte11 << std::dec << std::endl;
                        lastByte10 = byte10;
                        lastByte11 = byte11;
                        lastDebugTime2 = currentTick2;
                    }
                    
                    // Pattern analysis from testing:
                    // When A is pressed: byte 10 = 0x80, byte 11 = 0x01 (bit 0)
                    // When B is pressed: byte 10 = 0x80, byte 11 = 0x02 (bit 1)
                    // 
                    // Conclusion:
                    // - Byte 10 bit 7 (0x80) = general "button pressed" flag
                    // - Byte 11 indicates which specific button is pressed:
                    //   - Bit 0 (0x01) = Button A
                    //   - Bit 1 (0x02) = Button B
                    //   - Other bits likely map to other buttons
                    
                    // Map buttons based on byte 11 when byte 10 bit 7 is set
                    bool buttonPressed = (byte10 & 0x80) != 0;
                    
                    if (buttonPressed)
                    {
                        m_currentButtons[BUTTON_A] = (byte11 & 0x01) != 0;  // Bit 0
                        m_currentButtons[BUTTON_B] = (byte11 & 0x02) != 0;  // Bit 1
                        m_currentButtons[BUTTON_X] = (byte11 & 0x04) != 0;  // Bit 2 (needs testing)
                        m_currentButtons[BUTTON_Y] = (byte11 & 0x08) != 0;  // Bit 3 (needs testing)
                        m_currentButtons[BUTTON_LB] = (byte11 & 0x10) != 0; // Bit 4 (needs testing)
                        m_currentButtons[BUTTON_RB] = (byte11 & 0x20) != 0; // Bit 5 (needs testing)
                        m_currentButtons[BUTTON_BACK] = (byte11 & 0x40) != 0; // Bit 6 (needs testing)
                        m_currentButtons[BUTTON_START] = (byte11 & 0x80) != 0; // Bit 7 (needs testing)
                    }
                    else
                    {
                        // No buttons pressed
                        m_currentButtons[BUTTON_A] = false;
                        m_currentButtons[BUTTON_B] = false;
                        m_currentButtons[BUTTON_X] = false;
                        m_currentButtons[BUTTON_Y] = false;
                        m_currentButtons[BUTTON_LB] = false;
                        m_currentButtons[BUTTON_RB] = false;
                        m_currentButtons[BUTTON_BACK] = false;
                        m_currentButtons[BUTTON_START] = false;
                    }
                    
                    // Stick buttons might be in a different byte - need to test
                    m_currentButtons[BUTTON_LS] = false;
                    m_currentButtons[BUTTON_RS] = false;
                    
                    // Debug: Show full hex dump when data changes significantly to help find button mappings
                    static BYTE lastFullReport[16] = {0};
                    bool reportChanged = false;
                    for (DWORD i = 0; i < dataSize && i < 16; i++)
                    {
                        if (pData[i] != lastFullReport[i])
                        {
                            reportChanged = true;
                            break;
                        }
                    }
                    
                    if (reportChanged && (currentTick2 - lastDebugTime2 > 300))
                    {
                        std::cout << "Raw Input: Full report when button state changes: ";
                        for (DWORD i = 0; i < dataSize && i < 16; i++)
                        {
                            printf("%02X ", pData[i]);
                            lastFullReport[i] = pData[i];
                        }
                        std::cout << std::endl;
                        std::cout << "Raw Input: Byte 4 = 0x" << std::hex << (int)pData[4] << std::dec;
                        std::cout << ", Byte 10 = 0x" << std::hex << (int)byte10 << std::dec;
                        std::cout << ", Byte 11 = 0x" << std::hex << (int)byte11 << std::dec << std::endl;
                        lastDebugTime2 = currentTick2;
                    }
                    
                    buttonsFound = true;
                    m_isConnected = true;
                }
                
                // Format 2: Fallback to original offset 4-5 if bytes 10-11 don't work
                if (!buttonsFound && dataSize >= 6)
                {
                    BYTE buttons1 = pData[4];
                    BYTE buttons2 = (dataSize >= 6) ? pData[5] : 0;
                    
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
                    
                    buttonsFound = true;
                    m_isConnected = true;
                }
            }
            else
            {
                // Not an Xbox controller - ignore this input to prevent false positives
                // Only process Xbox controllers via Raw Input
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

