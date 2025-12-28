#include <windows.h>
#include <iostream>
#include "XInputDevice.h"
#include "KeyboardMouse.h"
#include "Mapper.h"

/**
 * Main entry point for GamepadMapper application
 * 
 * This application maps Xbox controller input to keyboard and mouse events.
 * The main loop runs at approximately 200 Hz (5ms per frame) for low latency.
 */
int main()
{
    std::cout << "GamepadMapper - Xbox Controller to Keyboard/Mouse Mapper" << std::endl;
    std::cout << "========================================================" << std::endl;
    std::cout << "Press Ctrl+C to exit" << std::endl << std::endl;

    // Initialize XInput device (controller index 0)
    XInputDevice controller;
    if (!controller.Initialize(0))
    {
        std::cout << "ERROR: No Xbox controller detected on index 0." << std::endl;
        std::cout << "Please connect an Xbox controller and try again." << std::endl;
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();
        return 1;
    }

    std::cout << "Controller connected successfully!" << std::endl;
    std::cout << "Current mappings:" << std::endl;
    std::cout << "  Button A -> Space" << std::endl;
    std::cout << "  Button B -> Escape" << std::endl;
    std::cout << std::endl;

    // Initialize keyboard/mouse emulator
    KeyboardMouse keyboardMouse;

    // Initialize mapper
    Mapper mapper;
    mapper.Initialize(&controller, &keyboardMouse);

    // Main loop - runs at ~200 Hz (5ms per frame)
    const DWORD frameTimeMs = 5; // 200 Hz = 5ms per frame
    DWORD lastTime = GetTickCount();

    std::cout << "Running... (Press Ctrl+C to exit)" << std::endl << std::endl;

    while (true)
    {
        // Calculate frame time for consistent loop timing
        DWORD currentTime = GetTickCount();
        DWORD elapsed = currentTime - lastTime;

        // Update controller state
        if (!controller.Update())
        {
            std::cout << "Controller disconnected. Exiting..." << std::endl;
            break;
        }

        // Process mappings
        mapper.Update();

        // Sleep to maintain ~200 Hz update rate
        if (elapsed < frameTimeMs)
        {
            Sleep(frameTimeMs - elapsed);
        }

        lastTime = GetTickCount();

        // Exit on controller disconnect (handled above)
        // User can exit with Ctrl+C in console
    }

    std::cout << "Exiting..." << std::endl;
    return 0;
}

