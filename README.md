# GamepadMapper - The Witcher 1 Controller Support

A C++17 Windows application that enables Xbox controller support for **The Witcher 1** by mapping controller input to keyboard and mouse events, and optionally creating a virtual Xbox 360 controller using ViGEm.

## Features

- Complete controller mapping for The Witcher 1
- Maps all buttons, triggers, and analog sticks to keyboard/mouse
- Optional virtual Xbox 360 controller support (ViGEm)
- Low-latency input processing (~200 Hz update rate)
- Supports HidHide for hiding physical controller from the game

## Requirements

- Windows 10 or Windows 11
- Visual Studio 2022
- Xbox controller (wired or wireless with adapter)
- x64 platform
- **ViGEmBus** driver (required for full solution - creates virtual Xbox 360 controller)
- **HidHide** driver (required - hides physical controller from game)

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

## Controller Mappings (The Witcher 1)

| Controller Input | Action | Keyboard/Mouse Output |
|-----------------|--------|----------------------|
| Left Stick | Movement | W / A / S / D |
| Right Stick | Camera | Mouse movement |
| A | Interact | Enter |
| B | Cancel / Dodge | Escape |
| X | Fast Attack | Left Mouse Button |
| Y | Strong Attack | Right Mouse Button |
| LB | Cast Sign | Ctrl |
| RB | Combat Style | Shift |
| LT | Fast Style | Key 1 |
| RT | Strong Style | Key 2 |
| D-Pad | Menus / Signs | Keys 3–6 |
| Start | Game Menu | Escape |
| Back | Meditation | M |

## Setup Instructions

### 1. Install Drivers (Required)

#### ViGEmBus (Required - for Virtual Controller)
1. Download ViGEmBus from [ViGEm Releases](https://github.com/ViGEm/ViGEmBus/releases)
2. Install the driver
3. **Reboot your computer**

#### HidHide (Required - to Hide Physical Controller)
1. Download HidHide from [HidHide Releases](https://github.com/ViGEm/HidHide/releases)
2. Install the driver
3. **Reboot your computer**

### 2. Configure HidHide

1. Open **HidHide Configuration Client** (installed with HidHide)
2. Add your `GamepadMapper.exe` to the **Allowed Applications** list
3. Select your physical Xbox controller and mark it as **Hidden**
4. This ensures The Witcher 1 won't detect the physical controller

### 3. Build the Application

1. Open `GamepadMapper.sln` in Visual Studio 2022
2. Select x64 platform (Debug or Release)
3. **Required**: To enable virtual controller support (as per Requirements.md):
   - Download [ViGEmClient SDK](https://github.com/ViGEm/ViGEmClient)
   - Extract to a `lib` or `include` directory
   - Add include path in project settings
   - Link against `ViGEmClient.lib`
   - Define `VIGEM_SDK_AVAILABLE` preprocessor macro
   - See `SETUP_VIGEM.md` for detailed instructions
4. Build the solution (Ctrl+Shift+B)

**Note**: The application will compile and run without ViGEmClient SDK (keyboard/mouse mapping will work), but the virtual controller is a core requirement per Requirements.md for the complete solution.

The executable will be generated in `bin/x64/[Configuration]/GamepadMapper.exe`

### 4. Run the Application

1. **Run as Administrator** (required for SendInput and ViGEm)
2. Connect an Xbox controller to your PC
3. Launch The Witcher 1
4. The application will detect the controller and start mapping input
5. Press Ctrl+C in the console to exit

## Architecture

### XInputDevice
Encapsulates all XInput functionality for reading physical controller state. Tracks button state transitions to detect button presses and releases. Provides access to analog stick positions and trigger values.

### KeyboardMouse
Wrapper around Win32 `SendInput` API for sending keyboard and mouse events. Provides methods for key down/up events, mouse button clicks, and mouse movement.

### VirtualController
Manages a virtual Xbox 360 controller using ViGEmClient SDK. Creates a virtual XInput device that appears to the system. Forwards controller state to the virtual device so games can detect it.

### Mapper
Handles the mapping logic between controller input and keyboard/mouse output. Processes button state changes, analog stick movements, and trigger inputs. Also forwards input to the virtual controller when available.

### Main Loop
Runs at approximately 200 Hz (5ms per frame) for low-latency input processing. Updates controller state, processes mappings, and updates virtual controller each frame.

## How It Works

1. **Physical Controller** → Read via XInput
2. **HidHide** → Hides physical controller from The Witcher 1
3. **Mapper** → Converts controller input to keyboard/mouse events
4. **SendInput** → Sends keyboard/mouse events to the game
5. **ViGEm** → Creates virtual Xbox 360 controller (optional)

This architecture ensures The Witcher 1 only sees keyboard/mouse input (or the virtual controller), avoiding DirectInput exclusive-mode conflicts.

## Notes

- The application requires **administrator privileges** to send input to other applications
- XInput library is linked automatically (xinput.lib)
- All code is C++17 compliant
- Virtual controller support requires ViGEmBus driver and ViGEmClient SDK
- Physical controller hiding requires HidHide driver and manual configuration
- See `Requirements.md` for detailed architecture and design decisions

