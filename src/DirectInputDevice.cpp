#include "DirectInputDevice.h"
#include <cstring>

// Callback to enumerate joysticks
BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext)
{
    DirectInputDevice* pDevice = (DirectInputDevice*)pContext;
    
    if (!pDevice)
    {
        return DIENUM_CONTINUE;
    }
    
    // Try to create the device
    LPDIRECTINPUT8 pDI = pDevice->GetDirectInput();
    LPDIRECTINPUTDEVICE8* ppJoystick = pDevice->GetJoystickPtr();
    
    if (!pDI || !ppJoystick)
    {
        return DIENUM_CONTINUE;
    }
    
    HRESULT hr = pDI->CreateDevice(pdidInstance->guidInstance, ppJoystick, nullptr);
    
    if (FAILED(hr))
    {
        return DIENUM_CONTINUE; // Try next device
    }
    
    return DIENUM_STOP; // Found a device, stop enumeration
}

DirectInputDevice::DirectInputDevice()
    : m_pDI(nullptr)
    , m_pJoystick(nullptr)
    , m_isConnected(false)
{
    std::memset(m_currentButtons, 0, sizeof(m_currentButtons));
    std::memset(m_previousButtons, 0, sizeof(m_previousButtons));
}

DirectInputDevice::~DirectInputDevice()
{
    Cleanup();
}

bool DirectInputDevice::Initialize(HINSTANCE hInstance, HWND hwnd)
{
    // Initialize DirectInput
    HRESULT hr = DirectInput8Create(
        hInstance,
        DIRECTINPUT_VERSION,
        IID_IDirectInput8,
        (VOID**)&m_pDI,
        nullptr
    );
    
    if (FAILED(hr))
    {
        return false;
    }
    
    // Enumerate for joystick devices
    hr = m_pDI->EnumDevices(
        DI8DEVCLASS_GAMECTRL,
        EnumJoysticksCallback,
        this,
        DIEDFL_ATTACHEDONLY
    );
    
    if (FAILED(hr) || !m_pJoystick)
    {
        if (m_pDI)
        {
            m_pDI->Release();
            m_pDI = nullptr;
        }
        return false;
    }
    
    // Set data format
    hr = m_pJoystick->SetDataFormat(&c_dfDIJoystick);
    if (FAILED(hr))
    {
        m_pJoystick->Release();
        m_pJoystick = nullptr;
        m_pDI->Release();
        m_pDI = nullptr;
        return false;
    }
    
    // Set cooperative level
    DWORD flags = DISCL_NONEXCLUSIVE | DISCL_BACKGROUND;
    if (hwnd)
    {
        flags |= DISCL_FOREGROUND;
    }
    
    hr = m_pJoystick->SetCooperativeLevel(hwnd, flags);
    if (FAILED(hr))
    {
        m_pJoystick->Release();
        m_pJoystick = nullptr;
        m_pDI->Release();
        m_pDI = nullptr;
        return false;
    }
    
    // Set axis ranges (optional, but helps with analog sticks)
    DIPROPRANGE diprg;
    diprg.diph.dwSize = sizeof(DIPROPRANGE);
    diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    diprg.diph.dwObj = DIJOFS_X;
    diprg.diph.dwHow = DIPH_BYOFFSET;
    diprg.lMin = -1000;
    diprg.lMax = 1000;
    m_pJoystick->SetProperty(DIPROP_RANGE, &diprg.diph);
    
    diprg.diph.dwObj = DIJOFS_Y;
    m_pJoystick->SetProperty(DIPROP_RANGE, &diprg.diph);
    
    // Acquire the device
    hr = m_pJoystick->Acquire();
    if (FAILED(hr))
    {
        // Device might be in use, but we can still try to use it
    }
    
    m_isConnected = true;
    return true;
}

bool DirectInputDevice::Update()
{
    if (!m_pJoystick || !m_isConnected)
    {
        return false;
    }
    
    // Store previous button states
    std::memcpy(m_previousButtons, m_currentButtons, sizeof(m_currentButtons));
    
    // Read joystick state
    DIJOYSTATE js;
    HRESULT hr = m_pJoystick->GetDeviceState(sizeof(DIJOYSTATE), &js);
    
    if (FAILED(hr))
    {
        // Try to reacquire the device
        hr = m_pJoystick->Acquire();
        if (FAILED(hr))
        {
            return false;
        }
        
        // Try reading again
        hr = m_pJoystick->GetDeviceState(sizeof(DIJOYSTATE), &js);
        if (FAILED(hr))
        {
            return false;
        }
    }
    
    // Update button states
    // DirectInput buttons are in js.rgbButtons array
    // Xbox controller mapping (may vary by controller):
    // Button 0 = A, Button 1 = B, Button 2 = X, Button 3 = Y
    // Button 4 = LB, Button 5 = RB, Button 6 = Back, Button 7 = Start
    // Button 8 = LS, Button 9 = RS
    
    for (int i = 0; i < 32 && i < 10; i++)
    {
        m_currentButtons[i] = (js.rgbButtons[i] & 0x80) != 0;
    }
    
    return true;
}

bool DirectInputDevice::IsButtonPressed(int button) const
{
    if (button < 0 || button >= 32)
    {
        return false;
    }
    
    if (!m_isConnected)
    {
        return false;
    }
    
    return m_currentButtons[button];
}

bool DirectInputDevice::IsButtonJustPressed(int button) const
{
    if (button < 0 || button >= 32)
    {
        return false;
    }
    
    if (!m_isConnected)
    {
        return false;
    }
    
    return !m_previousButtons[button] && m_currentButtons[button];
}

bool DirectInputDevice::IsButtonJustReleased(int button) const
{
    if (button < 0 || button >= 32)
    {
        return false;
    }
    
    if (!m_isConnected)
    {
        return false;
    }
    
    return m_previousButtons[button] && !m_currentButtons[button];
}

void DirectInputDevice::Cleanup()
{
    if (m_pJoystick)
    {
        m_pJoystick->Unacquire();
        m_pJoystick->Release();
        m_pJoystick = nullptr;
    }
    
    if (m_pDI)
    {
        m_pDI->Release();
        m_pDI = nullptr;
    }
    
    m_isConnected = false;
}

