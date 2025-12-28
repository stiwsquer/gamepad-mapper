#pragma once

#include "XInputDevice.h"
#include "KeyboardMouse.h"

/**
 * Mapper - Maps Xbox controller input to keyboard and mouse actions
 * 
 * This class handles the mapping logic between controller buttons/sticks
 * and keyboard/mouse events. It tracks button state transitions to avoid
 * key spamming and ensures proper key down/up events.
 */
class Mapper
{
public:
    Mapper();
    ~Mapper();

    /**
     * Initialize the mapper with controller and keyboard/mouse interfaces
     * @param controller Reference to XInputDevice
     * @param keyboardMouse Reference to KeyboardMouse
     */
    void Initialize(XInputDevice* controller, KeyboardMouse* keyboardMouse);

    /**
     * Update the mapper - processes controller input and sends mapped actions
     * Should be called every frame
     */
    void Update();

private:
    /**
     * Process button mappings
     */
    void ProcessButtonMappings();

    /**
     * Handle a button state change
     * @param button XInput button flag
     * @param virtualKey Target virtual key code
     */
    void HandleButtonMapping(WORD button, WORD virtualKey);

    XInputDevice* m_controller;
    KeyboardMouse* m_keyboardMouse;
};

