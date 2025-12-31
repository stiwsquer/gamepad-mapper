#include <windows.h>
#include <iostream>
#include "XInputDevice.h"
#include "KeyboardMouse.h"
#include "Mapper.h"
#include "VirtualController.h"

/**
 * Check if the application is running with administrator privileges
 * @return true if running as administrator
 */
bool IsRunningAsAdministrator()
{
    BOOL isAdmin = FALSE;
    PSID adminGroup = nullptr;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    
    if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup))
    {
        CheckTokenMembership(nullptr, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }
    
    return isAdmin == TRUE;
}

/**
 * Main entry point for GamepadMapper application
 * 
 * This application maps Xbox controller input to keyboard and mouse events
 * for The Witcher 1, and optionally creates a virtual Xbox 360 controller
 * using ViGEm. The main loop runs at approximately 200 Hz (5ms per frame).
 */
int main()
{
    std::cout << "GamepadMapper - The Witcher 1 Controller Support" << std::endl;
    std::cout << "================================================" << std::endl;
    
    // Check for administrator privileges (required for SendInput to work with games)
    if (!IsRunningAsAdministrator())
    {
        std::cout << "WARNING: Not running as Administrator!" << std::endl;
        std::cout << "Keyboard input may not work in games." << std::endl;
        std::cout << "Please run this application as Administrator for full functionality." << std::endl;
        std::cout << std::endl;
    }
    else
    {
        std::cout << "Running with Administrator privileges." << std::endl;
    }
    
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

    std::cout << "Physical controller connected successfully!" << std::endl;

    // Initialize virtual controller (required per Requirements.md - needs ViGEmBus)
    VirtualController virtualController;
    bool virtualControllerAvailable = virtualController.Initialize();
    if (!virtualControllerAvailable)
    {
        std::cout << "WARNING: Virtual controller not available." << std::endl;
        std::cout << "Keyboard/mouse mapping will still work, but virtual controller is disabled." << std::endl;
        std::cout << "For the complete solution per Requirements.md, install ViGEmBus driver and ViGEmClient SDK." << std::endl;
        std::cout << "See SETUP_VIGEM.md for SDK integration instructions." << std::endl;
    }

    // Initialize keyboard/mouse emulator
    KeyboardMouse keyboardMouse;

    // Initialize mapper
    Mapper mapper;
    if (virtualControllerAvailable)
    {
        mapper.Initialize(&controller, &keyboardMouse, &virtualController);
    }
    else
    {
        mapper.Initialize(&controller, &keyboardMouse, nullptr);
    }

    std::cout << std::endl;
    std::cout << "Controller mappings (The Witcher 1):" << std::endl;
    std::cout << "  Left Stick -> WASD (Movement)" << std::endl;
    std::cout << "  Right Stick -> Mouse (Camera)" << std::endl;
    std::cout << "  Right Stick Click -> TAB (Tryb Rozmowy)" << std::endl;
    std::cout << "  A -> Space (Zatrzymanie gry)" << std::endl;
    std::cout << "  B -> Escape" << std::endl;
    std::cout << "  X -> Left Mouse Button" << std::endl;
    std::cout << "  Y -> Right Mouse Button" << std::endl;
    std::cout << "  LB -> 1, 6 (Eliksiry szybki dostęp)" << std::endl;
    std::cout << "  RB -> 2, 7 (Eliksiry szybki dostęp)" << std::endl;
    std::cout << "  LT -> X (Styl Szybki)" << std::endl;
    std::cout << "  RT -> Z (Styl Silny)" << std::endl;
    std::cout << "  LT + RT -> C (Styl Grupowy)" << std::endl;
    std::cout << "  D-Pad Up -> - (Następny Znak)" << std::endl;
    std::cout << "  D-Pad Down -> = (Poprzedni Znak)" << std::endl;
    std::cout << "  D-Pad Left -> [ (Poprzednia broń)" << std::endl;
    std::cout << "  D-Pad Right -> ] (Następna broń)" << std::endl;
    std::cout << "  Start -> H (Bohater)" << std::endl;
    std::cout << "  Back -> I (Ekwipunek)" << std::endl;
    std::cout << std::endl;

    // Main loop - runs at ~200 Hz (5ms per frame)
    const DWORD frameTimeMs = 5; // 200 Hz = 5ms per frame
    DWORD lastTime = GetTickCount();

    std::cout << "Running... (Press Ctrl+C to exit)" << std::endl;
    std::cout << "IMPORTANT: Make sure The Witcher 1 window is in focus for keyboard input to work!" << std::endl;
    std::cout << std::endl;
    std::cout << "DEBUG: If buttons don't work, check the console for debug messages (Debug build only)." << std::endl;
    std::cout << std::endl;

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

    // Cleanup
    virtualController.Shutdown();

    std::cout << "Exiting..." << std::endl;
    return 0;
}

