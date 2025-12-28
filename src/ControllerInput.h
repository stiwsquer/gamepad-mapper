#pragma once

/**
 * ControllerInput - Abstract interface for reading controller input
 * 
 * This allows the mapper to work with either XInput or Raw Input
 * without knowing which one is being used.
 */
class ControllerInput
{
public:
    virtual ~ControllerInput() = default;
    
    /**
     * Update the controller state
     * @return true if controller is connected and updated
     */
    virtual bool Update() = 0;
    
    /**
     * Check if button A is pressed
     */
    virtual bool IsButtonAPressed() const = 0;
    
    /**
     * Check if button A was just pressed
     */
    virtual bool IsButtonAJustPressed() const = 0;
    
    /**
     * Check if button A was just released
     */
    virtual bool IsButtonAJustReleased() const = 0;
    
    /**
     * Check if button B is pressed
     */
    virtual bool IsButtonBPressed() const = 0;
    
    /**
     * Check if button B was just pressed
     */
    virtual bool IsButtonBJustPressed() const = 0;
    
    /**
     * Check if button B was just released
     */
    virtual bool IsButtonBJustReleased() const = 0;
    
    /**
     * Check if controller is connected
     */
    virtual bool IsConnected() const = 0;
};

