#include "Mapper.h"
#include <XInput.h>
#include <algorithm>
#include <iostream>

// Dead zone threshold (about 24% of full range)
const SHORT DEAD_ZONE = 7849;

Mapper::Mapper()
    : m_controller(nullptr)
    , m_keyboardMouse(nullptr)
    , m_virtualController(nullptr)
    , m_wPressed(false)
    , m_aPressed(false)
    , m_sPressed(false)
    , m_dPressed(false)
    , m_leftTriggerPressed(false)
    , m_rightTriggerPressed(false)
{
}

Mapper::~Mapper()
{
}

void Mapper::Initialize(XInputDevice* controller, KeyboardMouse* keyboardMouse, VirtualController* virtualController)
{
    m_controller = controller;
    m_keyboardMouse = keyboardMouse;
    m_virtualController = virtualController;
}

void Mapper::Update()
{
    if (!m_controller || !m_keyboardMouse)
    {
        return;
    }

    // Process all button mappings
    ProcessButtonMappings();

    // Process analog stick mappings
    ProcessAnalogSticks();

    // Process trigger mappings
    ProcessTriggers();

    // Forward state to virtual controller if available
    if (m_virtualController && m_virtualController->IsConnected())
    {
        m_virtualController->Update(m_controller->GetState());
    }
}

void Mapper::ProcessButtonMappings()
{
    // According to Requirements.md:
    // A -> Enter (Interact)
    HandleButtonMapping(XINPUT_GAMEPAD_A, VK_RETURN);

    // B -> Escape (Cancel / Dodge)
    HandleButtonMapping(XINPUT_GAMEPAD_B, VK_ESCAPE);

    // X -> Left Mouse Button (Fast Attack)
    if (m_controller->IsButtonJustPressed(XINPUT_GAMEPAD_X))
    {
        m_keyboardMouse->SendMouseButtonDown(0);
    }
    else if (m_controller->IsButtonJustReleased(XINPUT_GAMEPAD_X))
    {
        m_keyboardMouse->SendMouseButtonUp(0);
    }

    // Y -> Right Mouse Button (Strong Attack)
    if (m_controller->IsButtonJustPressed(XINPUT_GAMEPAD_Y))
    {
        m_keyboardMouse->SendMouseButtonDown(1);
    }
    else if (m_controller->IsButtonJustReleased(XINPUT_GAMEPAD_Y))
    {
        m_keyboardMouse->SendMouseButtonUp(1);
    }

    // LB -> Ctrl (Cast Sign)
    HandleButtonMapping(XINPUT_GAMEPAD_LEFT_SHOULDER, VK_CONTROL);

    // RB -> Shift (Combat Style)
    HandleButtonMapping(XINPUT_GAMEPAD_RIGHT_SHOULDER, VK_SHIFT);

    // LT -> Key 1 (Fast Style) - handled in ProcessTriggers

    // RT -> Key 2 (Strong Style) - handled in ProcessTriggers

    // D-Pad Up -> Key 3
    HandleButtonMapping(XINPUT_GAMEPAD_DPAD_UP, '3');

    // D-Pad Down -> Key 4
    HandleButtonMapping(XINPUT_GAMEPAD_DPAD_DOWN, '4');

    // D-Pad Left -> Key 5
    HandleButtonMapping(XINPUT_GAMEPAD_DPAD_LEFT, '5');

    // D-Pad Right -> Key 6
    HandleButtonMapping(XINPUT_GAMEPAD_DPAD_RIGHT, '6');

    // Start -> Escape (Game Menu)
    HandleButtonMapping(XINPUT_GAMEPAD_START, VK_ESCAPE);

    // Back -> M (Meditation)
    HandleButtonMapping(XINPUT_GAMEPAD_BACK, 'M');
}

void Mapper::ProcessAnalogSticks()
{
    if (!m_controller || !m_keyboardMouse)
    {
        return;
    }

    // Left Stick -> WASD movement
    SHORT leftX = ApplyDeadZone(m_controller->GetLeftStickX());
    SHORT leftY = ApplyDeadZone(m_controller->GetLeftStickY());

    // Determine movement direction based on stick position
    // W (forward) - negative Y
    // S (backward) - positive Y
    // A (left) - negative X
    // D (right) - positive X

    bool shouldPressW = (leftY < -DEAD_ZONE);
    bool shouldPressS = (leftY > DEAD_ZONE);
    bool shouldPressA = (leftX < -DEAD_ZONE);
    bool shouldPressD = (leftX > DEAD_ZONE);

    // Handle W key
    if (shouldPressW && !m_wPressed)
    {
        m_keyboardMouse->SendKeyDown('W');
        m_wPressed = true;
    }
    else if (!shouldPressW && m_wPressed)
    {
        m_keyboardMouse->SendKeyUp('W');
        m_wPressed = false;
    }

    // Handle S key
    if (shouldPressS && !m_sPressed)
    {
        m_keyboardMouse->SendKeyDown('S');
        m_sPressed = true;
    }
    else if (!shouldPressS && m_sPressed)
    {
        m_keyboardMouse->SendKeyUp('S');
        m_sPressed = false;
    }

    // Handle A key
    if (shouldPressA && !m_aPressed)
    {
        m_keyboardMouse->SendKeyDown('A');
        m_aPressed = true;
    }
    else if (!shouldPressA && m_aPressed)
    {
        m_keyboardMouse->SendKeyUp('A');
        m_aPressed = false;
    }

    // Handle D key
    if (shouldPressD && !m_dPressed)
    {
        m_keyboardMouse->SendKeyDown('D');
        m_dPressed = true;
    }
    else if (!shouldPressD && m_dPressed)
    {
        m_keyboardMouse->SendKeyUp('D');
        m_dPressed = false;
    }

    // Right Stick -> Mouse movement (Camera)
    SHORT rightX = ApplyDeadZone(m_controller->GetRightStickX());
    SHORT rightY = ApplyDeadZone(m_controller->GetRightStickY());

    // Scale stick movement to mouse movement
    // XInput range is -32768 to 32767, scale to reasonable mouse delta
    // Using a sensitivity multiplier (adjust as needed)
    const float mouseSensitivity = 0.1f;
    int mouseDeltaX = static_cast<int>(rightX * mouseSensitivity);
    int mouseDeltaY = static_cast<int>(-rightY * mouseSensitivity); // Invert Y for natural camera movement

    if (mouseDeltaX != 0 || mouseDeltaY != 0)
    {
        m_keyboardMouse->SendMouseMove(mouseDeltaX, mouseDeltaY);
    }
}

void Mapper::ProcessTriggers()
{
    if (!m_controller || !m_keyboardMouse)
    {
        return;
    }

    // LT -> Key 1 (Fast Style)
    BYTE leftTrigger = m_controller->GetLeftTrigger();
    const BYTE triggerThreshold = 128; // 50% threshold

    if (leftTrigger > triggerThreshold && !m_leftTriggerPressed)
    {
        m_keyboardMouse->SendKeyDown('1');
        m_leftTriggerPressed = true;
    }
    else if (leftTrigger <= triggerThreshold && m_leftTriggerPressed)
    {
        m_keyboardMouse->SendKeyUp('1');
        m_leftTriggerPressed = false;
    }

    // RT -> Key 2 (Strong Style)
    BYTE rightTrigger = m_controller->GetRightTrigger();

    if (rightTrigger > triggerThreshold && !m_rightTriggerPressed)
    {
        m_keyboardMouse->SendKeyDown('2');
        m_rightTriggerPressed = true;
    }
    else if (rightTrigger <= triggerThreshold && m_rightTriggerPressed)
    {
        m_keyboardMouse->SendKeyUp('2');
        m_rightTriggerPressed = false;
    }
}

void Mapper::HandleButtonMapping(WORD button, WORD virtualKey)
{
    if (!m_controller || !m_keyboardMouse)
    {
        return;
    }

    // Check if button was just pressed (transition from not pressed to pressed)
    if (m_controller->IsButtonJustPressed(button))
    {
        #ifdef _DEBUG
        std::cout << "Button pressed: 0x" << std::hex << button << " -> VK 0x" << virtualKey << std::dec << std::endl;
        #endif
        bool result = m_keyboardMouse->SendKeyDown(virtualKey);
        #ifdef _DEBUG
        if (!result)
        {
            std::cerr << "WARNING: SendKeyDown failed for VK 0x" << std::hex << virtualKey << std::dec << std::endl;
        }
        #endif
    }
    // Check if button was just released (transition from pressed to not pressed)
    else if (m_controller->IsButtonJustReleased(button))
    {
        #ifdef _DEBUG
        std::cout << "Button released: 0x" << std::hex << button << " -> VK 0x" << virtualKey << std::dec << std::endl;
        #endif
        bool result = m_keyboardMouse->SendKeyUp(virtualKey);
        #ifdef _DEBUG
        if (!result)
        {
            std::cerr << "WARNING: SendKeyUp failed for VK 0x" << std::hex << virtualKey << std::dec << std::endl;
        }
        #endif
    }
}

SHORT Mapper::ApplyDeadZone(SHORT value, SHORT deadZone) const
{
    if (value > deadZone)
    {
        return value - deadZone;
    }
    else if (value < -deadZone)
    {
        return value + deadZone;
    }
    return 0;
}

