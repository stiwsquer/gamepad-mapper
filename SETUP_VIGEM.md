# ViGEmClient SDK Setup Guide

This guide explains how to integrate the ViGEmClient SDK to enable virtual Xbox 360 controller support.

## Prerequisites

1. **ViGEmBus Driver** - Must be installed first
   - Download from: https://github.com/ViGEm/ViGEmBus/releases
   - Install and reboot

2. **ViGEmClient SDK** - Required for compilation
   - Download from: https://github.com/ViGEm/ViGEmClient
   - Clone or download the repository

## Integration Steps

### Option 1: Using Git Submodule (Recommended)

```bash
cd X:\Repos\gamepad-mapper
git submodule add https://github.com/ViGEm/ViGEmClient lib/ViGEmClient
git submodule update --init --recursive
```

### Option 2: Manual Download

1. Download or clone ViGEmClient repository
2. Extract/copy to `lib/ViGEmClient` directory in your project

### Visual Studio Configuration

1. **Add Include Directory:**
   - Right-click project → Properties
   - Configuration Properties → C/C++ → General
   - Additional Include Directories: Add `$(ProjectDir)lib\ViGEmClient\include`

2. **Add Library Directory:**
   - Configuration Properties → Linker → General
   - Additional Library Directories: Add `$(ProjectDir)lib\ViGEmClient\lib\$(Platform)\$(Configuration)`

3. **Link Library:**
   - Configuration Properties → Linker → Input
   - Additional Dependencies: Add `ViGEmClient.lib`

4. **Define Preprocessor Macro:**
   - Configuration Properties → C/C++ → Preprocessor
   - Preprocessor Definitions: Add `VIGEM_SDK_AVAILABLE`

### Project File Configuration (Alternative)

If you prefer to edit the `.vcxproj` file directly, add:

```xml
<PropertyGroup>
  <AdditionalIncludeDirectories>$(ProjectDir)lib\ViGEmClient\include;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
</PropertyGroup>
<PropertyGroup>
  <AdditionalLibraryDirectories>$(ProjectDir)lib\ViGEmClient\lib\$(Platform)\$(Configuration);%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
</PropertyGroup>
<ItemDefinitionGroup>
  <Link>
    <AdditionalDependencies>ViGEmClient.lib;%(AdditionalDependencies)</AdditionalDependencies>
  </Link>
  <ClCompile>
    <PreprocessorDefinitions>VIGEM_SDK_AVAILABLE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
  </ClCompile>
</ItemDefinitionGroup>
```

## Building ViGEmClient Library

**You need to build the library if the `.lib` file doesn't exist.**

Check if `lib\ViGEmClient\lib\x64\Debug\ViGEmClient.lib` (or Release) exists. If not, build it:

1. Open `lib\ViGEmClient\ViGEmClient.sln` in Visual Studio
2. Select x64 platform and your desired configuration (Debug or Release)
3. Build the solution (Ctrl+Shift+B)
4. The `ViGEmClient.lib` file will be generated in:
   - `lib\ViGEmClient\lib\x64\Debug\` (for Debug builds)
   - `lib\ViGEmClient\lib\x64\Release\` (for Release builds)

**Note**: You may need to create the `lib\x64\Debug` and `lib\x64\Release` directories first if they don't exist. The build output location depends on the ViGEmClient project configuration - check the project's output directory settings if the file isn't where expected.

## Verification

After setup, rebuild the project. The application should:
- Compile without errors
- Show "Virtual Xbox 360 controller created successfully!" on startup (if ViGEmBus is installed)
- Create a virtual controller that appears in Windows Game Controllers

## Troubleshooting

- **"ViGEmClient.h: No such file"** - Check include directory path
- **"ViGEmClient.lib: Cannot open file"** - Check library directory and ensure library is built
- **"Failed to connect to ViGEmBus"** - Ensure ViGEmBus driver is installed and system was rebooted
- **Virtual controller not appearing** - Run application as Administrator

## Notes

- The application will compile and run without ViGEmClient SDK, but virtual controller support will be disabled
- Keyboard/mouse mapping will still work without ViGEmClient
- Virtual controller is optional - the main functionality (keyboard/mouse mapping) works independently

