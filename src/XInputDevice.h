#pragma once

#include <windows.h>
#include <XInput.h>

/**
 * XInputDevice - Encapsulates Xbox controller input reading using XInput API
 * 
 * This class provides a clean interface for reading the state of an Xbox controller.
 * It handles all XInput-related logic and provides methods to check button states
 * and analog stick positions.
 */
class XInputDevice
{
public:
    XInputDevice();
    ~XInputDevice();

    /**
     * Initialize the device for a specific controller index (0-3)
     * @param controllerIndex Controller index (0-3)
     * @return true if controller is connected, false otherwise
     */
    bool Initialize(int controllerIndex);

    /**
     * Update the controller state by reading from XInput
     * Should be called every frame
     * @return true if controller is connected, false otherwise
     */
    bool Update();

    /**
     * Check if a button is currently pressed
     * @param button Button flag (e.g., XINPUT_GAMEPAD_A)
     * @return true if button is pressed
     */
    bool IsButtonPressed(WORD button) const;

    /**
     * Check if a button was just pressed (transition from not pressed to pressed)
     * @param button Button flag
     * @return true if button was just pressed this frame
     */
    bool IsButtonJustPressed(WORD button) const;

    /**
     * Check if a button was just released (transition from pressed to not pressed)
     * @param button Button flag
     * @return true if button was just released this frame
     */
    bool IsButtonJustReleased(WORD button) const;

    /**
     * Get the current controller state
     * @return Reference to XINPUT_STATE
     */
    const XINPUT_STATE& GetState() const { return m_currentState; }

    /**
     * Get the previous controller state (for detecting transitions)
     * @return Reference to XINPUT_STATE
     */
    const XINPUT_STATE& GetPreviousState() const { return m_previousState; }

    /**
     * Check if controller is connected
     * @return true if connected
     */
    bool IsConnected() const { return m_isConnected; }

private:
    int m_controllerIndex;
    XINPUT_STATE m_currentState;
    XINPUT_STATE m_previousState;
    bool m_isConnected;
};

