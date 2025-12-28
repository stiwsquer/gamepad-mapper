#pragma once

#include <windows.h>
#include <hidsdi.h>

/**
 * RawInputDevice - Reads Xbox controller input using Raw Input API
 * 
 * This class uses Raw Input API which can read controller input even when
 * XInput is locked by another application (like games). This solves the
 * issue where games take exclusive XInput access.
 */
class RawInputDevice
{
public:
    RawInputDevice();
    ~RawInputDevice();

    /**
     * Initialize Raw Input device registration
     * @param hwnd Window handle to receive Raw Input messages
     * @return true if successful
     */
    bool Initialize(HWND hwnd);

    /**
     * Process Raw Input message (call from window message handler)
     * @param lParam Raw Input message parameter
     * @return true if message was processed
     */
    bool ProcessRawInput(LPARAM lParam);

    /**
     * Check if a button is currently pressed
     * @param button Button index (0=A, 1=B, 2=X, 3=Y, 4=LB, 5=RB, 6=Back, 7=Start, 8=LS, 9=RS)
     * @return true if button is pressed
     */
    bool IsButtonPressed(int button) const;

    /**
     * Check if a button was just pressed
     * @param button Button index
     * @return true if button was just pressed this frame
     */
    bool IsButtonJustPressed(int button) const;

    /**
     * Check if a button was just released
     * @param button Button index
     * @return true if button was just released this frame
     */
    bool IsButtonJustReleased(int button) const;

    /**
     * Update button states (call at end of frame to track transitions)
     */
    void Update();

    /**
     * Check if controller is connected
     * @return true if connected
     */
    bool IsConnected() const { return m_isConnected; }

private:
    /**
     * Convert Xbox controller button to our button index
     */
    int GetButtonIndex(USHORT usagePage, USHORT usage) const;

    /**
     * Check if device is an Xbox controller
     */
    bool IsXboxController(const RAWINPUTHEADER& header) const;

    HWND m_hwnd;
    bool m_isConnected;
    
    // Button states (16 buttons max)
    bool m_currentButtons[16];
    bool m_previousButtons[16];
    
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

