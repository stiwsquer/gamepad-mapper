#pragma once

#include <windows.h>
#include <dinput.h>

/**
 * DirectInputDevice - Reads Xbox controller input using DirectInput API
 * 
 * This class uses DirectInput which can sometimes access controllers even when
 * XInput is locked by games. DirectInput is an older API but provides an
 * alternative way to read controller input.
 */
class DirectInputDevice
{
public:
    DirectInputDevice();
    ~DirectInputDevice();

    /**
     * Initialize DirectInput device
     * @param hInstance Application instance handle
     * @param hwnd Window handle (can be NULL for console apps)
     * @return true if successful
     */
    bool Initialize(HINSTANCE hInstance, HWND hwnd);

    /**
     * Update the controller state by reading from DirectInput
     * Should be called every frame
     * @return true if controller is connected and updated
     */
    bool Update();

    /**
     * Check if a button is currently pressed
     * @param button Button index (0=A, 1=B, 2=X, 3=Y, 4=LB, 5=RB, 6=Back, 7=Start, 8=LS, 9=RS)
     * @return true if button is pressed
     */
    bool IsButtonPressed(int button) const;

    /**
     * Check if a button was just pressed (transition from not pressed to pressed)
     * @param button Button index
     * @return true if button was just pressed this frame
     */
    bool IsButtonJustPressed(int button) const;

    /**
     * Check if a button was just released (transition from pressed to not pressed)
     * @param button Button index
     * @return true if button was just released this frame
     */
    bool IsButtonJustReleased(int button) const;

    /**
     * Check if controller is connected
     * @return true if connected
     */
    bool IsConnected() const { return m_isConnected; }

    /**
     * Cleanup DirectInput resources
     */
    void Cleanup();

    // Helper methods for enumeration callback (public so callback can access)
    LPDIRECTINPUT8 GetDirectInput() { return m_pDI; }
    LPDIRECTINPUTDEVICE8* GetJoystickPtr() { return &m_pJoystick; }

private:
    LPDIRECTINPUT8 m_pDI;
    LPDIRECTINPUTDEVICE8 m_pJoystick;
    bool m_isConnected;
    
    // Button states (32 buttons max for DirectInput)
    bool m_currentButtons[32];
    bool m_previousButtons[32];
    
    // Xbox controller button mapping
    static const int BUTTON_A = 0;
    static const int BUTTON_B = 1;
    static const int BUTTON_X = 2;
    static const int BUTTON_Y = 3;
    static const int BUTTON_LB = 4;
    static const int BUTTON_RB = 5;
    static const int BUTTON_BACK = 6;
    static const int BUTTON_START = 7;
    static const int BUTTON_LS = 8;
    static const int BUTTON_RS = 9;
};

