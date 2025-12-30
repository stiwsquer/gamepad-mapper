#pragma once

#include <windows.h>
#include <XInput.h>

// Forward declarations for ViGEmClient
// Note: User needs to include ViGEmClient.h from the SDK
// For now, we'll use void* to avoid requiring the SDK header at compile time
// The implementation will handle the actual ViGEmClient types

/**
 * VirtualController - Manages a virtual Xbox 360 controller using ViGEm
 * 
 * This class creates and manages a virtual Xbox 360 controller that appears
 * to the system as a real XInput device. The game will see this virtual
 * controller instead of the physical one (when HidHide is configured).
 */
class VirtualController
{
public:
    VirtualController();
    ~VirtualController();

    /**
     * Initialize the virtual controller
     * @return true if successful, false otherwise
     */
    bool Initialize();

    /**
     * Update the virtual controller state
     * @param state XInput state to forward to the virtual controller
     * @return true if successful
     */
    bool Update(const XINPUT_STATE& state);

    /**
     * Check if the virtual controller is connected
     * @return true if connected
     */
    bool IsConnected() const { return m_isConnected; }

    /**
     * Cleanup and disconnect the virtual controller
     */
    void Shutdown();

private:
    void* m_client;           // ViGEmClient* - opaque pointer
    void* m_controller;        // ViGEmTargetXbox360* - opaque pointer
    bool m_isConnected;
};

