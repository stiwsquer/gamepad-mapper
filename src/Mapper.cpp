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
    , m_bothTriggersPressed(false)
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
    // A -> Space (Zatrzymanie gry / Pause game)
    HandleButtonMapping(XINPUT_GAMEPAD_A, VK_SPACE);

    // B -> Escape
    HandleButtonMapping(XINPUT_GAMEPAD_B, VK_ESCAPE);

    // X -> Left Mouse Button (Lewa mysz)
    if (m_controller->IsButtonJustPressed(XINPUT_GAMEPAD_X))
    {
        m_keyboardMouse->SendMouseButtonDown(0);
    }
    else if (m_controller->IsButtonJustReleased(XINPUT_GAMEPAD_X))
    {
        m_keyboardMouse->SendMouseButtonUp(0);
    }

    // Y -> Right Mouse Button (Prawa mysz)
    if (m_controller->IsButtonJustPressed(XINPUT_GAMEPAD_Y))
    {
        m_keyboardMouse->SendMouseButtonDown(1);
    }
    else if (m_controller->IsButtonJustReleased(XINPUT_GAMEPAD_Y))
    {
        m_keyboardMouse->SendMouseButtonUp(1);
    }

    // Right Stick Click -> TAB (Tryb Rozmowy / Conversation mode)
    HandleButtonMapping(XINPUT_GAMEPAD_RIGHT_THUMB, VK_TAB);

    // LB -> 1 and 6 (Eliksiry szybki dostęp)
    // Press 1 on press, 6 on release (or just 1 - you can adjust)
    if (m_controller->IsButtonJustPressed(XINPUT_GAMEPAD_LEFT_SHOULDER))
    {
        m_keyboardMouse->SendKeyDown('1');
        m_keyboardMouse->SendKeyUp('1');
        m_keyboardMouse->SendKeyDown('6');
        m_keyboardMouse->SendKeyUp('6');
    }

    // RB -> 2 and 7 (Eliksiry szybki dostęp)
    if (m_controller->IsButtonJustPressed(XINPUT_GAMEPAD_RIGHT_SHOULDER))
    {
        m_keyboardMouse->SendKeyDown('2');
        m_keyboardMouse->SendKeyUp('2');
        m_keyboardMouse->SendKeyDown('7');
        m_keyboardMouse->SendKeyUp('7');
    }

    // LT -> X (Styl Szybki / Fast Style) - handled in ProcessTriggers
    // RT -> Z (Styl Silny / Strong Style) - handled in ProcessTriggers
    // LT + RT -> C (Styl Grupowy / Group Style) - handled in ProcessTriggers

    // D-Pad Up -> - (Następny Znak / Next Sign)
    HandleButtonMapping(XINPUT_GAMEPAD_DPAD_UP, VK_OEM_MINUS); // - key

    // D-Pad Down -> = (Poprzedni Znak / Previous Sign)
    HandleButtonMapping(XINPUT_GAMEPAD_DPAD_DOWN, VK_OEM_PLUS); // = key

    // D-Pad Left -> [ (Poprzednia broń / Previous weapon)
    HandleButtonMapping(XINPUT_GAMEPAD_DPAD_LEFT, VK_OEM_4); // [ key

    // D-Pad Right -> ] (Następna broń / Next weapon)
    HandleButtonMapping(XINPUT_GAMEPAD_DPAD_RIGHT, VK_OEM_6); // ] key

    // Start (Menu button) -> H (Bohater / Hero)
    HandleButtonMapping(XINPUT_GAMEPAD_START, 'H');

    // Back (View button) -> I (Ekwipunek / Inventory)
    HandleButtonMapping(XINPUT_GAMEPAD_BACK, 'I');
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
    // W (forward) - positive Y (inverted from XInput where negative Y is up)
    // S (backward) - negative Y
    // A (left) - negative X
    // D (right) - positive X

    bool shouldPressW = (leftY > DEAD_ZONE);  // Inverted: positive Y = forward
    bool shouldPressS = (leftY < -DEAD_ZONE); // Inverted: negative Y = backward
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
    // Using a lower sensitivity multiplier to reduce fast movement
    const float mouseSensitivity = 0.0015f; // Extremely low sensitivity for very precise camera control
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

    const BYTE triggerThreshold = 128; // 50% threshold
    
    BYTE leftTrigger = m_controller->GetLeftTrigger();
    BYTE rightTrigger = m_controller->GetRightTrigger();
    
    bool leftPressed = leftTrigger > triggerThreshold;
    bool rightPressed = rightTrigger > triggerThreshold;
    bool bothPressed = leftPressed && rightPressed;

    // Handle LT + RT combination first (Styl Grupowy / Group Style -> C)
    if (bothPressed && !m_bothTriggersPressed)
    {
        // Release individual triggers if they were pressed
        if (m_leftTriggerPressed)
        {
            m_keyboardMouse->SendKeyUp('X');
            m_leftTriggerPressed = false;
        }
        if (m_rightTriggerPressed)
        {
            m_keyboardMouse->SendKeyUp('Z');
            m_rightTriggerPressed = false;
        }
        
        // Press C for group style
        m_keyboardMouse->SendKeyDown('C');
        m_bothTriggersPressed = true;
    }
    else if (!bothPressed && m_bothTriggersPressed)
    {
        // Release C
        m_keyboardMouse->SendKeyUp('C');
        m_bothTriggersPressed = false;
    }
    
    // Handle LT alone (Styl Szybki / Fast Style -> X)
    // Only if both triggers are not pressed
    if (!bothPressed)
    {
        if (leftPressed && !m_leftTriggerPressed)
        {
            m_keyboardMouse->SendKeyDown('X');
            m_leftTriggerPressed = true;
        }
        else if (!leftPressed && m_leftTriggerPressed)
        {
            m_keyboardMouse->SendKeyUp('X');
            m_leftTriggerPressed = false;
        }

        // Handle RT alone (Styl Silny / Strong Style -> Z)
        if (rightPressed && !m_rightTriggerPressed)
        {
            m_keyboardMouse->SendKeyDown('Z');
            m_rightTriggerPressed = true;
        }
        else if (!rightPressed && m_rightTriggerPressed)
        {
            m_keyboardMouse->SendKeyUp('Z');
            m_rightTriggerPressed = false;
        }
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

