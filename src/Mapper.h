#pragma once

#include "XInputDevice.h"
#include "KeyboardMouse.h"
#include "VirtualController.h"

/**
 * Mapper - Maps Xbox controller input to keyboard and mouse actions
 * 
 * This class handles the mapping logic between controller buttons/sticks
 * and keyboard/mouse events. It tracks button state transitions to avoid
 * key spamming and ensures proper key down/up events.
 * 
 * Also forwards input to a virtual Xbox 360 controller for games that
 * can detect it (when ViGEm is configured).
 */
class Mapper
{
public:
    Mapper();
    ~Mapper();

    /**
     * Initialize the mapper with controller, keyboard/mouse, and virtual controller interfaces
     * @param controller Reference to XInputDevice
     * @param keyboardMouse Reference to KeyboardMouse
     * @param virtualController Reference to VirtualController (can be nullptr if not available)
     */
    void Initialize(XInputDevice* controller, KeyboardMouse* keyboardMouse, VirtualController* virtualController = nullptr);

    /**
     * Update the mapper - processes controller input and sends mapped actions
     * Should be called every frame
     */
    void Update();

private:
    /**
     * Process button mappings according to Requirements.md
     */
    void ProcessButtonMappings();

    /**
     * Process analog stick mappings
     * Left Stick -> WASD movement
     * Right Stick -> Mouse camera movement
     */
    void ProcessAnalogSticks();

    /**
     * Process trigger mappings
     */
    void ProcessTriggers();

    /**
     * Handle a button state change
     * @param button XInput button flag
     * @param virtualKey Target virtual key code
     */
    void HandleButtonMapping(WORD button, WORD virtualKey);

    /**
     * Apply dead zone to analog stick value
     * @param value Raw stick value (-32768 to 32767)
     * @param deadZone Dead zone threshold (0-32767)
     * @return Adjusted value or 0 if within dead zone
     */
    SHORT ApplyDeadZone(SHORT value, SHORT deadZone = 7849) const; // ~24% dead zone

    XInputDevice* m_controller;
    KeyboardMouse* m_keyboardMouse;
    VirtualController* m_virtualController;

    // Track currently pressed movement keys to avoid spamming
    bool m_wPressed;
    bool m_aPressed;
    bool m_sPressed;
    bool m_dPressed;

    // Track trigger states
    bool m_leftTriggerPressed;
    bool m_rightTriggerPressed;
};

