#pragma once

#include <windows.h>

/**
 * KeyboardMouse - Wrapper for sending keyboard and mouse input using Win32 SendInput API
 * 
 * This class provides methods to send keyboard key events and mouse actions.
 * All input is sent using the SendInput function, which works entirely in user-mode.
 */
class KeyboardMouse
{
public:
    KeyboardMouse();
    ~KeyboardMouse();

    /**
     * Send a keyboard key down event
     * @param virtualKey Virtual key code (e.g., VK_SPACE, VK_ESCAPE)
     * @return true if successful
     */
    bool SendKeyDown(WORD virtualKey);

    /**
     * Send a keyboard key up event
     * @param virtualKey Virtual key code
     * @return true if successful
     */
    bool SendKeyUp(WORD virtualKey);

    /**
     * Send a keyboard key press (down then up)
     * @param virtualKey Virtual key code
     * @return true if successful
     */
    bool SendKeyPress(WORD virtualKey);

    /**
     * Send mouse button down event
     * @param button Mouse button (0=left, 1=right, 2=middle)
     * @return true if successful
     */
    bool SendMouseButtonDown(int button);

    /**
     * Send mouse button up event
     * @param button Mouse button (0=left, 1=right, 2=middle)
     * @return true if successful
     */
    bool SendMouseButtonUp(int button);

    /**
     * Send mouse movement (relative)
     * @param deltaX Movement in X direction (pixels)
     * @param deltaY Movement in Y direction (pixels)
     * @return true if successful
     */
    bool SendMouseMove(int deltaX, int deltaY);

private:
    /**
     * Helper to convert button index to MOUSEEVENTF flag
     */
    DWORD GetMouseButtonFlag(int button) const;
};

