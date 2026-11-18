# CONNTRACE

CONNTRACE is a native Windows 11 TCP connection monitor written in C17. This v0.1 implementation provides an IPv4 TCP snapshot CLI using the Windows IP Helper API. It displays the connection state, local and remote addresses and ports, and owning PID. It is observational: it does not terminate processes or change system/network settings.

## Requirements

- Visual Studio Community 2026 with **Desktop development with C++** (MSVC x64 build tools) and **C++ CMake tools for Windows** components
- Windows 11 SDK
- CMake 3.25 or newer

The project directory is `C:\Users\magik\Pictures\CONNTRACE`.

## Open and build in Visual Studio

1. In Visual Studio, select **File > Open > Folder...** and open the CONNTRACE directory.
2. Select the **Windows x64 Debug** CMake preset. Visual Studio configures the project from `CMakeLists.txt`.
3. Choose the `conntrace` startup item and select **Build > Build All** (or press **Ctrl+Shift+B**).
4. Run/debug with **F5**. To pass command-line arguments, configure the `conntrace` launch target's arguments in Visual Studio's CMake settings.
5. Use **Test > Run CTest for CONNTRACE** (or run CTest from the CMake test explorer) to run the unit and CLI smoke tests.

The `x64-release` preset is also provided. Visual Studio's CMake integration manages the configuration; there is no separate solution file to maintain.

## Build and test from a Visual Studio Developer PowerShell

From the project directory:

```powershell
cmake --preset x64-debug
cmake --build --preset debug
ctest --preset debug
```

Run a single snapshot:

```powershell
.\build\x64-debug\Debug\conntrace.exe --once
```

Refresh every five seconds (Ctrl+C to stop):

```powershell
.\build\x64-debug\Debug\conntrace.exe --interval 5
```

The refresh interval must be between 1 and 3600 seconds. `--help` lists the available options. For Release, use `cmake --preset x64-release` followed by `cmake --build --preset release`.

## Current scope

This first stage uses `GetExtendedTcpTable()` with owner-PID information and reports IPv4 TCP rows, including listeners. Later stages can add UDP, process image names, adapters, change monitoring, filtering, exports, and the interactive terminal interface without changing this v0.1 behavior.
