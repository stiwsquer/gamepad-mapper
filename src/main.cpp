#include <windows.h>
#include <iostream>
#include "XInputDevice.h"
#include "KeyboardMouse.h"
#include "Mapper.h"
#include "RawInputDevice.h"
#include "DirectInputDevice.h"

// Window procedure for hidden window (needed for Raw Input)
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static RawInputDevice* pRawInput = nullptr;
    
    if (uMsg == WM_CREATE)
    {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        pRawInput = (RawInputDevice*)pCreate->lpCreateParams;
        return 0;
    }
    
    if (uMsg == WM_INPUT)
    {
        if (pRawInput)
        {
            pRawInput->ProcessRawInput(lParam);
        }
        return 0;
    }
    
    // Handle other messages that might be needed
    if (uMsg == WM_DEVICECHANGE)
    {
        // Device connection/disconnection
        return 0;
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

/**
 * Main entry point for GamepadMapper application
 * 
 * This application maps Xbox controller input to keyboard and mouse events.
 * Uses Raw Input API to work even when games lock XInput exclusively.
 * The main loop runs at approximately 200 Hz (5ms per frame) for low latency.
 */
int main()
{
    std::cout << "GamepadMapper - Xbox Controller to Keyboard/Mouse Mapper" << std::endl;
    std::cout << "========================================================" << std::endl;
    std::cout << std::endl;

    // Ask user to choose input method
    std::cout << "Select input method:" << std::endl;
    std::cout << "  1. XInput (faster, but may be locked by games)" << std::endl;
    std::cout << "  2. Raw Input (may not work with Xbox controllers)" << std::endl;
    std::cout << "  3. DirectInput (alternative, may work when XInput is locked)" << std::endl;
    std::cout << std::endl;
    std::cout << "Enter choice (1, 2, or 3): ";
    
    int choice = 0;
    std::cin >> choice;
    
    if (choice != 1 && choice != 2 && choice != 3)
    {
        std::cout << "Invalid choice. Please enter 1, 2, or 3." << std::endl;
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.ignore();
        std::cin.get();
        return 1;
    }
    
    std::cout << std::endl;
    std::cout << "Press Ctrl+C to exit" << std::endl << std::endl;

    bool useXInput = (choice == 1);
    bool useRawInput = (choice == 2);
    bool useDirectInput = (choice == 3);
    HWND hwnd = nullptr;
    RawInputDevice rawInputController;
    XInputDevice xinputController;
    DirectInputDevice directInputController;
    bool controllerConnected = false;

    if (useXInput)
    {
        // Use XInput
        std::cout << "Initializing XInput..." << std::endl;
        if (xinputController.Initialize(0))
        {
            std::cout << "Controller connected via XInput!" << std::endl;
            controllerConnected = true;
        }
        else
        {
            std::cout << "ERROR: No Xbox controller detected via XInput." << std::endl;
            std::cout << "Please connect an Xbox controller and try again." << std::endl;
            std::cout << "Press Enter to exit..." << std::endl;
            std::cin.ignore();
            std::cin.get();
            return 1;
        }
    }
    else if (useRawInput)
    {
        // Use Raw Input
        std::cout << "Initializing Raw Input..." << std::endl;
        
        // Create a hidden window for Raw Input messages
        WNDCLASS wc = { 0 };
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = L"GamepadMapperWindow";
        
        if (!RegisterClass(&wc))
        {
            std::cout << "ERROR: Failed to register window class." << std::endl;
            std::cout << "Press Enter to exit..." << std::endl;
            std::cin.ignore();
            std::cin.get();
            return 1;
        }

        hwnd = CreateWindowEx(
            0,
            L"GamepadMapperWindow",
            L"GamepadMapper",
            0, 0, 0, 0, 0,
            HWND_MESSAGE, // Message-only window (hidden)
            nullptr,
            GetModuleHandle(nullptr),
            &rawInputController
        );

        if (!hwnd)
        {
            std::cout << "ERROR: Failed to create window." << std::endl;
            std::cout << "Press Enter to exit..." << std::endl;
            std::cin.ignore();
            std::cin.get();
            return 1;
        }

        if (rawInputController.Initialize(hwnd))
        {
            std::cout << "Raw Input initialized successfully." << std::endl;
            std::cout << "Waiting for controller input..." << std::endl;
            std::cout << "Please press any button on your controller." << std::endl;
            controllerConnected = true;
        }
        else
        {
            std::cout << "ERROR: Failed to initialize Raw Input." << std::endl;
            std::cout << "Press Enter to exit..." << std::endl;
            std::cin.ignore();
            std::cin.get();
            DestroyWindow(hwnd);
            return 1;
        }
    }
    else if (useDirectInput)
    {
        // Use DirectInput
        std::cout << "Initializing DirectInput..." << std::endl;
        if (directInputController.Initialize(GetModuleHandle(nullptr), nullptr))
        {
            std::cout << "Controller connected via DirectInput!" << std::endl;
            controllerConnected = true;
        }
        else
        {
            std::cout << "ERROR: No controller detected via DirectInput." << std::endl;
            std::cout << "Please connect a controller and try again." << std::endl;
            std::cout << "Press Enter to exit..." << std::endl;
            std::cin.ignore();
            std::cin.get();
            return 1;
        }
    }

    if (!controllerConnected)
    {
        std::cout << "ERROR: Controller not connected." << std::endl;
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.ignore();
        std::cin.get();
        if (hwnd) DestroyWindow(hwnd);
        return 1;
    }

    std::cout << "Current mappings:" << std::endl;
    std::cout << "  Button A -> Space" << std::endl;
    std::cout << "  Button B -> Escape" << std::endl;
    std::cout << std::endl;
    
    if (useXInput)
    {
        std::cout << "Using XInput mode." << std::endl;
        std::cout << "NOTE: If a game locks XInput, the controller will stop working." << std::endl;
    }
    else if (useRawInput)
    {
        std::cout << "Using Raw Input mode." << std::endl;
        std::cout << "NOTE: Raw Input may not work with Xbox controllers." << std::endl;
        std::cout << "      Xbox controllers prefer XInput, and may not send Raw Input data." << std::endl;
        std::cout << "      If this doesn't work, try DirectInput mode (option 3) instead." << std::endl;
    }
    else if (useDirectInput)
    {
        std::cout << "Using DirectInput mode." << std::endl;
        std::cout << "NOTE: DirectInput may work even when games lock XInput." << std::endl;
    }
    std::cout << std::endl;

    // Initialize keyboard/mouse emulator
    KeyboardMouse keyboardMouse;

    // Initialize mapper (only for XInput)
    Mapper mapper;
    if (useXInput)
    {
        mapper.Initialize(&xinputController, &keyboardMouse);
    }

    // Main loop - runs at ~200 Hz (5ms per frame)
    const DWORD frameTimeMs = 5; // 200 Hz = 5ms per frame
    DWORD lastTime = GetTickCount();

    std::cout << "Running... (Press Ctrl+C to exit)" << std::endl << std::endl;

    while (true)
    {
        // Process Windows messages (needed for Raw Input)
        if (hwnd)
        {
            MSG msg;
            while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        // Calculate frame time for consistent loop timing
        DWORD currentTime = GetTickCount();
        DWORD elapsed = currentTime - lastTime;

        if (useXInput)
        {
            // Use XInput
            if (xinputController.Update())
            {
                mapper.Update();
            }
            else
            {
                // XInput failed (probably locked by game)
                std::cout << "WARNING: XInput failed. Controller may be locked by another application." << std::endl;
                std::cout << "Consider restarting with DirectInput mode (option 3)." << std::endl;
                // Continue trying, but warn the user
            }
        }
        else if (useRawInput)
        {
            // Use Raw Input
            rawInputController.Update();
            
            // Check if we're receiving any input at all
            static bool firstInputReceived = false;
            static DWORD lastWarningTime = 0;
            bool anyButtonPressed = false;
            for (int i = 0; i < 10; i++)
            {
                if (rawInputController.IsButtonPressed(i))
                {
                    anyButtonPressed = true;
                    if (!firstInputReceived)
                    {
                        std::cout << "Raw Input: Controller input detected!" << std::endl;
                        firstInputReceived = true;
                    }
                    break;
                }
            }
            
            // Warn if no input received after 5 seconds
            if (!firstInputReceived && (currentTime - lastWarningTime > 5000))
            {
                std::cout << "WARNING: No Raw Input data received from controller." << std::endl;
                std::cout << "         Xbox controllers may not send Raw Input when XInput is available." << std::endl;
                std::cout << "         Try using DirectInput mode (option 3) instead." << std::endl;
                lastWarningTime = currentTime;
            }
            
            // Process Raw Input button mappings manually
            // Button A (index 0) -> Space
            if (rawInputController.IsButtonJustPressed(0)) // BUTTON_A = 0
            {
                keyboardMouse.SendKeyDown(VK_SPACE);
            }
            else if (rawInputController.IsButtonJustReleased(0))
            {
                keyboardMouse.SendKeyUp(VK_SPACE);
            }
            
            // Button B (index 1) -> Escape
            if (rawInputController.IsButtonJustPressed(1)) // BUTTON_B = 1
            {
                keyboardMouse.SendKeyDown(VK_ESCAPE);
            }
            else if (rawInputController.IsButtonJustReleased(1))
            {
                keyboardMouse.SendKeyUp(VK_ESCAPE);
            }
        }
        else if (useDirectInput)
        {
            // Use DirectInput
            if (directInputController.Update())
            {
                // Process DirectInput button mappings manually
                // Button A (index 0) -> Space
                if (directInputController.IsButtonJustPressed(0))
                {
                    keyboardMouse.SendKeyDown(VK_SPACE);
                }
                else if (directInputController.IsButtonJustReleased(0))
                {
                    keyboardMouse.SendKeyUp(VK_SPACE);
                }
                
                // Button B (index 1) -> Escape
                if (directInputController.IsButtonJustPressed(1))
                {
                    keyboardMouse.SendKeyDown(VK_ESCAPE);
                }
                else if (directInputController.IsButtonJustReleased(1))
                {
                    keyboardMouse.SendKeyUp(VK_ESCAPE);
                }
            }
            else
            {
                // DirectInput failed (device may have been disconnected)
                static DWORD lastWarningTime = 0;
                if (currentTime - lastWarningTime > 5000)
                {
                    std::cout << "WARNING: DirectInput update failed. Controller may be disconnected." << std::endl;
                    lastWarningTime = currentTime;
                }
            }
        }

        // Sleep to maintain ~200 Hz update rate
        if (elapsed < frameTimeMs)
        {
            Sleep(frameTimeMs - elapsed);
        }

        lastTime = GetTickCount();
    }

    // Cleanup
    if (useDirectInput)
    {
        directInputController.Cleanup();
    }
    if (hwnd) DestroyWindow(hwnd);
    std::cout << "Exiting..." << std::endl;
    return 0;
}

