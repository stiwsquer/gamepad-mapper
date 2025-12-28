#include "Mapper.h"
#include <XInput.h>

Mapper::Mapper()
    : m_controller(nullptr)
    , m_keyboardMouse(nullptr)
{
}

Mapper::~Mapper()
{
}

void Mapper::Initialize(XInputDevice* controller, KeyboardMouse* keyboardMouse)
{
    m_controller = controller;
    m_keyboardMouse = keyboardMouse;
}

void Mapper::Update()
{
    if (!m_controller || !m_keyboardMouse)
    {
        return;
    }

    // Process all button mappings
    ProcessButtonMappings();

    // Future: Process analog stick mappings for mouse movement
    // Future: Process trigger mappings
}

void Mapper::ProcessButtonMappings()
{
    // Button A -> Space
    HandleButtonMapping(XINPUT_GAMEPAD_A, VK_SPACE);

    // Button B -> Escape
    HandleButtonMapping(XINPUT_GAMEPAD_B, VK_ESCAPE);

    // Future: Add more button mappings here
    // HandleButtonMapping(XINPUT_GAMEPAD_X, VK_...);
    // HandleButtonMapping(XINPUT_GAMEPAD_Y, VK_...);
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
        m_keyboardMouse->SendKeyDown(virtualKey);
    }
    // Check if button was just released (transition from pressed to not pressed)
    else if (m_controller->IsButtonJustReleased(button))
    {
        m_keyboardMouse->SendKeyUp(virtualKey);
    }
}

