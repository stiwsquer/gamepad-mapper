#include "VirtualController.h"
#include <iostream>

// ViGEmClient SDK includes
// Note: User must download ViGEmClient SDK and place it in lib/ViGEmClient directory
// For now, we'll provide a stub implementation that can be completed once SDK is available
#ifdef VIGEM_SDK_AVAILABLE
#include <ViGEm/Client.h>
#include <ViGEm/Common.h>
#else
// Stub definitions for when SDK is not yet available
// User should define VIGEM_SDK_AVAILABLE and include the actual SDK headers
typedef void* PVIGEM_CLIENT;
typedef void* PVIGEM_TARGET;
typedef enum _VIGEM_ERROR {
    VIGEM_ERROR_NONE = 0x20000000,
    VIGEM_ERROR_BUS_NOT_FOUND = 0xE0000001,
    VIGEM_ERROR_NO_FREE_SLOT = 0xE0000002,
    VIGEM_ERROR_INVALID_TARGET = 0xE0000003,
    VIGEM_ERROR_REMOVAL_FAILED = 0xE0000004,
    VIGEM_ERROR_ALREADY_CONNECTED = 0xE0000005,
    VIGEM_ERROR_TARGET_UNINITIALIZED = 0xE0000006,
    VIGEM_ERROR_TARGET_NOT_PLUGGED_IN = 0xE0000007,
    VIGEM_ERROR_BUS_VERSION_MISMATCH = 0xE0000008,
    VIGEM_ERROR_BUS_ACCESS_FAILED = 0xE0000009,
    VIGEM_ERROR_CALLBACK_ALREADY_REGISTERED = 0xE0000010,
    VIGEM_ERROR_CALLBACK_NOT_FOUND = 0xE0000011,
    VIGEM_ERROR_BUS_ALREADY_CONNECTED = 0xE0000012,
    VIGEM_ERROR_BUS_INVALID_HANDLE = 0xE0000013,
    VIGEM_ERROR_XUSB_USERINDEX_OUT_OF_RANGE = 0xE0000014
} VIGEM_ERROR;
#define VIGEM_SUCCESS(x) ((x) == VIGEM_ERROR_NONE)
typedef struct _XUSB_REPORT {
    USHORT wButtons;
    BYTE bLeftTrigger;
    BYTE bRightTrigger;
    SHORT sThumbLX;
    SHORT sThumbLY;
    SHORT sThumbRX;
    SHORT sThumbRY;
} XUSB_REPORT, *PXUSB_REPORT;
typedef enum _XUSB_BUTTON {
    XUSB_GAMEPAD_DPAD_UP = 0x0001,
    XUSB_GAMEPAD_DPAD_DOWN = 0x0002,
    XUSB_GAMEPAD_DPAD_LEFT = 0x0004,
    XUSB_GAMEPAD_DPAD_RIGHT = 0x0008,
    XUSB_GAMEPAD_START = 0x0010,
    XUSB_GAMEPAD_BACK = 0x0020,
    XUSB_GAMEPAD_LEFT_THUMB = 0x0040,
    XUSB_GAMEPAD_RIGHT_THUMB = 0x0080,
    XUSB_GAMEPAD_LEFT_SHOULDER = 0x0100,
    XUSB_GAMEPAD_RIGHT_SHOULDER = 0x0200,
    XUSB_GAMEPAD_GUIDE = 0x0400,
    XUSB_GAMEPAD_A = 0x1000,
    XUSB_GAMEPAD_B = 0x2000,
    XUSB_GAMEPAD_X = 0x4000,
    XUSB_GAMEPAD_Y = 0x8000
} XUSB_BUTTON;
#endif

VirtualController::VirtualController()
    : m_client(nullptr)
    , m_controller(nullptr)
    , m_isConnected(false)
{
}

VirtualController::~VirtualController()
{
    Shutdown();
}

bool VirtualController::Initialize()
{
#ifdef VIGEM_SDK_AVAILABLE
    // Create ViGEm client
    m_client = vigem_alloc();
    if (!m_client)
    {
        std::cerr << "ERROR: Failed to allocate ViGEm client" << std::endl;
        return false;
    }

    // Connect to ViGEmBus driver
    VIGEM_ERROR error = vigem_connect(reinterpret_cast<PVIGEM_CLIENT>(m_client));
    if (!VIGEM_SUCCESS(error))
    {
        std::cerr << "ERROR: Failed to connect to ViGEmBus. Error: 0x" << std::hex << error << std::endl;
        std::cerr << "Make sure ViGEmBus driver is installed and running." << std::endl;
        vigem_free(reinterpret_cast<PVIGEM_CLIENT>(m_client));
        m_client = nullptr;
        return false;
    }

    // Create virtual Xbox 360 controller
    m_controller = vigem_target_x360_alloc();
    if (!m_controller)
    {
        std::cerr << "ERROR: Failed to allocate virtual Xbox 360 controller" << std::endl;
        vigem_disconnect(reinterpret_cast<PVIGEM_CLIENT>(m_client));
        vigem_free(reinterpret_cast<PVIGEM_CLIENT>(m_client));
        m_client = nullptr;
        return false;
    }

    // Add controller to bus
    error = vigem_target_add(reinterpret_cast<PVIGEM_CLIENT>(m_client), reinterpret_cast<PVIGEM_TARGET>(m_controller));
    if (!VIGEM_SUCCESS(error))
    {
        std::cerr << "ERROR: Failed to add virtual controller to bus. Error: 0x" << std::hex << error << std::endl;
        vigem_target_free(reinterpret_cast<PVIGEM_TARGET>(m_controller));
        vigem_disconnect(reinterpret_cast<PVIGEM_CLIENT>(m_client));
        vigem_free(reinterpret_cast<PVIGEM_CLIENT>(m_client));
        m_controller = nullptr;
        m_client = nullptr;
        return false;
    }

    m_isConnected = true;
    std::cout << "Virtual Xbox 360 controller created successfully!" << std::endl;
    return true;
#else
    std::cerr << "WARNING: ViGEmClient SDK not available. Virtual controller disabled." << std::endl;
    std::cerr << "To enable virtual controller support:" << std::endl;
    std::cerr << "1. Download ViGEmClient SDK from https://github.com/ViGEm/ViGEmClient" << std::endl;
    std::cerr << "2. Extract to a 'lib' or 'include' directory in the project" << std::endl;
    std::cerr << "3. Add include path and link against ViGEmClient.lib" << std::endl;
    std::cerr << "4. Define VIGEM_SDK_AVAILABLE preprocessor macro" << std::endl;
    return false;
#endif
}

bool VirtualController::Update(const XINPUT_STATE& state)
{
#ifdef VIGEM_SDK_AVAILABLE
    if (!m_isConnected || !m_controller || !m_client)
    {
        return false;
    }

    // Convert XINPUT_STATE to XUSB_REPORT
    XUSB_REPORT report = { 0 };
    
    // Map buttons
    if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) report.wButtons |= XUSB_GAMEPAD_DPAD_UP;
    if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) report.wButtons |= XUSB_GAMEPAD_DPAD_DOWN;
    if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) report.wButtons |= XUSB_GAMEPAD_DPAD_LEFT;
    if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) report.wButtons |= XUSB_GAMEPAD_DPAD_RIGHT;
    if (state.Gamepad.wButtons & XINPUT_GAMEPAD_START) report.wButtons |= XUSB_GAMEPAD_START;
    if (state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) report.wButtons |= XUSB_GAMEPAD_BACK;
    if (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) report.wButtons |= XUSB_GAMEPAD_LEFT_THUMB;
    if (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) report.wButtons |= XUSB_GAMEPAD_RIGHT_THUMB;
    if (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) report.wButtons |= XUSB_GAMEPAD_LEFT_SHOULDER;
    if (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) report.wButtons |= XUSB_GAMEPAD_RIGHT_SHOULDER;
    if (state.Gamepad.wButtons & XINPUT_GAMEPAD_A) report.wButtons |= XUSB_GAMEPAD_A;
    if (state.Gamepad.wButtons & XINPUT_GAMEPAD_B) report.wButtons |= XUSB_GAMEPAD_B;
    if (state.Gamepad.wButtons & XINPUT_GAMEPAD_X) report.wButtons |= XUSB_GAMEPAD_X;
    if (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y) report.wButtons |= XUSB_GAMEPAD_Y;

    // Map triggers (XInput uses 0-255, XUSB uses 0-255, so direct mapping)
    report.bLeftTrigger = state.Gamepad.bLeftTrigger;
    report.bRightTrigger = state.Gamepad.bRightTrigger;

    // Map thumbsticks (XInput uses -32768 to 32767, XUSB uses -32768 to 32767, so direct mapping)
    report.sThumbLX = state.Gamepad.sThumbLX;
    report.sThumbLY = state.Gamepad.sThumbLY;
    report.sThumbRX = state.Gamepad.sThumbRX;
    report.sThumbRY = state.Gamepad.sThumbRY;

    // Submit report to virtual controller
    VIGEM_ERROR error = vigem_target_x360_update(reinterpret_cast<PVIGEM_CLIENT>(m_client), 
                                                  reinterpret_cast<PVIGEM_TARGET>(m_controller), 
                                                  report);
    
    return VIGEM_SUCCESS(error);
#else
    (void)state; // Suppress unused parameter warning
    return false;
#endif
}

void VirtualController::Shutdown()
{
#ifdef VIGEM_SDK_AVAILABLE
    if (m_controller && m_client)
    {
        vigem_target_remove(reinterpret_cast<PVIGEM_CLIENT>(m_client), reinterpret_cast<PVIGEM_TARGET>(m_controller));
        vigem_target_free(reinterpret_cast<PVIGEM_TARGET>(m_controller));
        m_controller = nullptr;
    }

    if (m_client)
    {
        vigem_disconnect(reinterpret_cast<PVIGEM_CLIENT>(m_client));
        vigem_free(reinterpret_cast<PVIGEM_CLIENT>(m_client));
        m_client = nullptr;
    }
#endif

    m_isConnected = false;
}

