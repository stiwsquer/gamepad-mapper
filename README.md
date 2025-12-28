# GamepadMapper

A C++17 Windows application that maps Xbox controller input to keyboard and mouse events using XInput and Win32 SendInput APIs.

## Features

- Maps Xbox controller buttons to keyboard keys
- Low-latency input processing (~200 Hz update rate)
- User-mode only (no drivers or kernel-mode code required)
- Clean, modular architecture designed for future expansion

## Requirements

- Windows 10 or Windows 11
- Visual Studio 2022
- Xbox controller (wired or wireless with adapter)
- x64 platform

## Project Structure

```
GamepadMapper/
├── src/
│   ├── main.cpp              # Application entry point and main loop
│   ├── XInputDevice.h/.cpp   # Xbox controller input handling
│   ├── KeyboardMouse.h/.cpp   # Keyboard/mouse emulation via SendInput
│   └── Mapper.h/.cpp         # Mapping logic (controller → keyboard/mouse)
├── GamepadMapper.sln         # Visual Studio solution file
└── GamepadMapper.vcxproj     # Visual Studio project file
```

## Current Mappings

- **Button A** → Space key
- **Button B** → Escape key

## Building

1. Open `GamepadMapper.sln` in Visual Studio 2022
2. Select x64 platform (Debug or Release)
3. Build the solution (Ctrl+Shift+B)

The executable will be generated in `bin/x64/[Configuration]/GamepadMapper.exe`

## Usage

1. Connect an Xbox controller to your PC
2. Run `GamepadMapper.exe`
3. The application will detect the controller and start mapping input
4. Press Button A on the controller to send Space key
5. Press Button B on the controller to send Escape key
6. Press Ctrl+C or Escape to exit

## Architecture

### XInputDevice
Encapsulates all XInput functionality for reading controller state. Tracks button state transitions to detect button presses and releases.

### KeyboardMouse
Wrapper around Win32 `SendInput` API for sending keyboard and mouse events. Provides methods for key down/up events and mouse actions.

### Mapper
Handles the mapping logic between controller input and keyboard/mouse output. Processes button state changes and sends appropriate key events.

### Main Loop
Runs at approximately 200 Hz (5ms per frame) for low-latency input processing. Updates controller state and processes mappings each frame.

## Future Enhancements

- Analog stick → mouse movement mapping
- Configurable mapping profiles
- Support for multiple controllers
- Trigger button mappings
- Dead zone configuration for analog sticks

## Notes

- The application requires administrator privileges to send input to other applications in some scenarios
- XInput library is linked automatically (xinput.lib)
- All code is C++17 compliant

