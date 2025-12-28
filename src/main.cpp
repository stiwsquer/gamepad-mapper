#include <windows.h>
#include <iostream>
#include "XInputDevice.h"
#include "KeyboardMouse.h"
#include "Mapper.h"
#include "RawInputDevice.h"

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
    std::cout << "  2. Raw Input (works even when games lock XInput)" << std::endl;
    std::cout << std::endl;
    std::cout << "Enter choice (1 or 2): ";
    
    int choice = 0;
    std::cin >> choice;
    
    if (choice != 1 && choice != 2)
    {
        std::cout << "Invalid choice. Please enter 1 or 2." << std::endl;
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.ignore();
        std::cin.get();
        return 1;
    }
    
    std::cout << std::endl;
    std::cout << "Press Ctrl+C to exit" << std::endl << std::endl;

    bool useXInput = (choice == 1);
    HWND hwnd = nullptr;
    RawInputDevice rawInputController;
    XInputDevice xinputController;
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
    else
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
            std::cout << "Raw Input initialized. Waiting for controller input..." << std::endl;
            std::cout << "Please press any button on your controller to verify connection." << std::endl;
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
    else
    {
        std::cout << "Using Raw Input mode." << std::endl;
        std::cout << "NOTE: This works even when games lock XInput!" << std::endl;
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
                std::cout << "Consider restarting with Raw Input mode (option 2)." << std::endl;
                // Continue trying, but warn the user
            }
        }
        else
        {
            // Use Raw Input
            rawInputController.Update();
            
            // Process Raw Input button mappings manually
            if (rawInputController.IsButtonAJustPressed())
            {
                keyboardMouse.SendKeyDown(VK_SPACE);
            }
            else if (rawInputController.IsButtonAJustReleased())
            {
                keyboardMouse.SendKeyUp(VK_SPACE);
            }
            
            if (rawInputController.IsButtonBJustPressed())
            {
                keyboardMouse.SendKeyDown(VK_ESCAPE);
            }
            else if (rawInputController.IsButtonBJustReleased())
            {
                keyboardMouse.SendKeyUp(VK_ESCAPE);
            }
        }

        // Sleep to maintain ~200 Hz update rate
        if (elapsed < frameTimeMs)
        {
            Sleep(frameTimeMs - elapsed);
        }

        lastTime = GetTickCount();
    }

    if (hwnd) DestroyWindow(hwnd);
    std::cout << "Exiting..." << std::endl;
    return 0;
}

