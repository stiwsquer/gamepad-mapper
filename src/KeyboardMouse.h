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

    /**
     * Send keyboard input using keybd_event (alternative to SendInput)
     * Some games block SendInput but allow keybd_event
     * @param virtualKey Virtual key code
     * @param keyDown true for key down, false for key up
     * @return true if successful
     */
    bool SendKeyEvent(WORD virtualKey, bool keyDown);

    /**
     * Send keyboard input directly to a window using PostMessage
     * This bypasses input blocking in some games
     * @param hWnd Window handle (nullptr = foreground window)
     * @param virtualKey Virtual key code
     * @param keyDown true for key down, false for key up
     * @return true if successful
     */
    bool SendKeyToWindow(HWND hWnd, WORD virtualKey, bool keyDown);

    /**
     * Get the game window handle (cached for performance)
     * @return Window handle or nullptr if not found
     */
    HWND GetGameWindow();

    /**
     * Find window by title (case-insensitive partial match)
     * Supports both ANSI and Unicode window titles
     * @param titlePart Part of the window title to search for (UTF-8)
     * @return Window handle or nullptr if not found
     */
    static HWND FindWindowByTitle(const char* titlePart);

private:
    /**
     * Helper to convert button index to MOUSEEVENTF flag
     */
    DWORD GetMouseButtonFlag(int button) const;

    // Cached game window handle to avoid repeated searches
    HWND m_cachedGameWindow;
    DWORD m_lastWindowCheckTime;
    static const DWORD WINDOW_CACHE_TIMEOUT_MS = 5000; // Re-check every 5 seconds
};

