# The Witcher 1 – Controller Support (Windows)

Enable reliable controller support for **The Witcher 1** on Windows by using a virtual Xbox controller and hiding the physical device from the game.

This project avoids DirectInput exclusive-mode issues by translating controller input into keyboard and mouse actions, then exposing a virtual XInput device to the game.

---

## 🎯 Goal

Create a C++ Windows application that:

- Reads input from a **physical game controller**
- Maps controller input to **keyboard + mouse**
- Exposes a **virtual Xbox 360 controller**
- Prevents *The Witcher 1* from detecting the physical controller

---

## 🚫 Problem Summary

- *The Witcher 1* has **no native controller support**
- Uses **DirectInput (exclusive mode)**
- Blocks Raw Input and DirectInput for other processes
- Only **XInput + virtual devices** work reliably

---

## 🧠 Final Architecture

[ Physical Controller ]
↓ (HidHide: hidden from the game)
[ Custom C++ Application ]
↓ (ViGEm: virtual controller)
[ Virtual Xbox 360 Controller ]
↓
[ The Witcher 1 ]

---

## 🧩 Core Responsibilities

### 1. Input Reading
- Use **XInput**
- Poll controller at ~200 Hz
- Assume Xbox-compatible device (PS controllers must be emulated)

---

### 2. Input Mapping

The game is mouse-driven; mapping must emulate **keyboard and mouse**, not native gamepad input.

| Controller | Action | Emulated Input |
|----------|-------|----------------|
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

Keyboard and mouse events must be sent using **SendInput()**.

---

### 3. Virtual Controller Output
- Create a **virtual Xbox 360 controller** using **ViGEm**
- Forward mapped input to the virtual device
- The game must see **only** the virtual controller

---

### 4. Physical Controller Hiding (Mandatory)

The physical controller must be invisible to the game to avoid double input and DirectInput conflicts.

Use **HidHide**:

- Hide the physical HID device system-wide
- Whitelist the custom application

Result:

Custom App → sees controller
Game → does NOT see controller


---

## 🔌 Required Components

### Drivers / SDKs

| Component | Purpose |
|--------|--------|
| **ViGEmBus** | Virtual Xbox controller driver |
| **ViGEmClient SDK** | C++ API for ViGEm |
| **HidHide** | HID device hiding driver |
| **XInput** | Physical controller input |

---

## 🛠️ Manual Setup (Required)

### 1. Install Drivers
1. Install **ViGEmBus**
2. Reboot
3. Install **HidHide**
4. Reboot

---

### 2. Configure HidHide

Using **HidHide Configuration Client**:

1. Add your application `.exe` to **Allowed Applications**
2. Select the physical controller and mark it as **Hidden**

---

### 3. Build & Run
- Build the application (**x64**)
- Run **as Administrator**
- Launch *The Witcher 1*
- Confirm that only the **virtual Xbox controller** is detected

---

## ❌ Explicit Non-Goals

- No DirectInput hooks
- No Raw Input reliance
- No DLL injection
- No game file modifications
- No anti-cheat bypass

---

## ✨ Optional Enhancements

- Force feedback passthrough (ViGEm)
- Configurable dead zones
- Profile system
- Runtime remapping
- Game focus detection

---

## 📌 Summary

This solution mirrors how **Steam Input** and **DS4Windows** operate internally.

The combination of:

- **XInput → mapping → ViGEm**
- **HidHide for physical device blocking**

is the **only stable, non-invasive approach** for legacy DirectInput games like *The Witcher 1*.

---

## 📄 License

This project is provided as-is for educational and personal use.



