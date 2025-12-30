#include "XInputDevice.h"
#include <cstring>

XInputDevice::XInputDevice()
    : m_controllerIndex(-1)
    , m_isConnected(false)
{
    std::memset(&m_currentState, 0, sizeof(XINPUT_STATE));
    std::memset(&m_previousState, 0, sizeof(XINPUT_STATE));
}

XInputDevice::~XInputDevice()
{
}

bool XInputDevice::Initialize(int controllerIndex)
{
    if (controllerIndex < 0 || controllerIndex > 3)
    {
        return false;
    }

    m_controllerIndex = controllerIndex;
    
    // Try to read the controller state to check if it's connected
    DWORD result = XInputGetState(m_controllerIndex, &m_currentState);
    m_isConnected = (result == ERROR_SUCCESS);
    
    if (m_isConnected)
    {
        m_previousState = m_currentState;
    }
    
    return m_isConnected;
}

bool XInputDevice::Update()
{
    if (m_controllerIndex < 0)
    {
        return false;
    }

    // Store previous state before reading new state
    m_previousState = m_currentState;

    // Read current state
    DWORD result = XInputGetState(m_controllerIndex, &m_currentState);
    m_isConnected = (result == ERROR_SUCCESS);

    return m_isConnected;
}

bool XInputDevice::IsButtonPressed(WORD button) const
{
    if (!m_isConnected)
    {
        return false;
    }

    return (m_currentState.Gamepad.wButtons & button) != 0;
}

bool XInputDevice::IsButtonJustPressed(WORD button) const
{
    if (!m_isConnected)
    {
        return false;
    }

    bool wasPressed = (m_previousState.Gamepad.wButtons & button) != 0;
    bool isPressed = (m_currentState.Gamepad.wButtons & button) != 0;

    return !wasPressed && isPressed;
}

bool XInputDevice::IsButtonJustReleased(WORD button) const
{
    if (!m_isConnected)
    {
        return false;
    }

    bool wasPressed = (m_previousState.Gamepad.wButtons & button) != 0;
    bool isPressed = (m_currentState.Gamepad.wButtons & button) != 0;

    return wasPressed && !isPressed;
}

SHORT XInputDevice::GetLeftStickX() const
{
    if (!m_isConnected)
    {
        return 0;
    }
    return m_currentState.Gamepad.sThumbLX;
}

SHORT XInputDevice::GetLeftStickY() const
{
    if (!m_isConnected)
    {
        return 0;
    }
    return m_currentState.Gamepad.sThumbLY;
}

SHORT XInputDevice::GetRightStickX() const
{
    if (!m_isConnected)
    {
        return 0;
    }
    return m_currentState.Gamepad.sThumbRX;
}

SHORT XInputDevice::GetRightStickY() const
{
    if (!m_isConnected)
    {
        return 0;
    }
    return m_currentState.Gamepad.sThumbRY;
}

BYTE XInputDevice::GetLeftTrigger() const
{
    if (!m_isConnected)
    {
        return 0;
    }
    return m_currentState.Gamepad.bLeftTrigger;
}

BYTE XInputDevice::GetRightTrigger() const
{
    if (!m_isConnected)
    {
        return 0;
    }
    return m_currentState.Gamepad.bRightTrigger;
}

