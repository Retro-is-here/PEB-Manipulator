# PEB-Manipulator

PEB-Manipulator is an x64dbg plugin designed for inspecting and manipulating
Process Environment Block (PEB) structures in Windows processes.

This plugin is intended for reverse engineering, malware analysis,
and anti-debugging research.

---

## Features

- Read PEB fields:
  - BeingDebugged
  - NtGlobalFlag
  - ProcessParameters
  - CommandLine
- Patch common PEB-based anti-debug checks
- Display process command-line arguments (argv-style)
- Supports both **x86 (32-bit)** and **x64 (64-bit)** targets
- Clean WinAPI-based implementation

---

## Supported Architectures

- x86 (32-bit processes)
- x64 (64-bit processes)

> The plugin automatically detects the target architecture inside x64dbg.

---

## Build Requirements

- Windows 10 / 11
- Visual Studio 2022 (MSVC)
- Windows SDK
- x64dbg SDK

---

## Build Instructions

1. Open the solution file in Visual Studio
2. Select build configuration:
   - `Release | Win32` for x86
   - `Release | x64` for x64
3. Build the project

The output files will be:
- `PEB-Manipulator.dp32`
- `PEB-Manipulator.dp64`

---

## Installation

1. Build the plugin or download it from **Releases**
2. Copy:
   - `*.dp32` → `x64dbg\plugins\x32\`
   - `*.dp64` → `x64dbg\plugins\x64\`
3. Restart x64dbg
4. Load the plugin from the **Plugins** menu

---

## Usage

- Open a process in x64dbg
- Navigate to:
Plugins → PEB-Manipulator

- Select the desired action:
- Read PEB fields
- Patch anti-debug flags
- Show command-line arguments

---

## Disclaimer

This project is provided for **educational and research purposes only**.

The author is not responsible for any misuse of this software.
Use it only on software you own or have permission to analyze.

---

## License

MIT License

---

## Author

Developed by **Retr0**
Telegram --> @AreYou_Lost
