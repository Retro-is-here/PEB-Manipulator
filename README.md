# PEB-Manipulator

PEB-Manipulator is an **x64dbg plugin** for inspecting and manipulating the  
**Process Environment Block (PEB)** of Windows processes.

This project was developed as a **research and learning-oriented plugin**, with
a focus on **Windows internals, reverse engineering, malware analysis, and
PEB-based anti-debugging techniques**.

---

## 🎯 Purpose

- Practice low-level Windows internals (PEB & TEB)
- Understand common PEB-based anti-debug techniques
- Build a clean, real-world x64dbg plugin using WinAPI
- Provide a solid base that can be extended with new features

This plugin is **not commercial-ready** and is intentionally kept minimal and
transparent for educational and research purposes.

---

## ✨ Features

- Read key PEB fields:
  - `BeingDebugged`
  - `NtGlobalFlag`
  - `ProcessParameters`
  - `CommandLine`
- Patch common **PEB-based anti-debug checks**
- Display process command-line arguments (argv-style)
- Supports **both x86 (32-bit) and x64 (64-bit)** target processes
- Clean and minimal **WinAPI-based implementation**
- Automatic architecture detection inside x64dbg

---

## 🧱 Supported Architectures

| Architecture | Status |
|--------------|--------|
| x86 (32-bit) | ✅ Supported |
| x64 (64-bit) | ✅ Supported |

The plugin automatically detects the debugged process architecture.

---

## 📁 Project Structure

PEB-Manipulator/
│
├── src/ # Shared source code
├── include/ # Headers
├── resources/ # Icons / resources
│
├── build/
│ ├── x86/ # Output: PEB-Manipulator.dp32
│ └── x64/ # Output: PEB-Manipulator.dp64
│
├── PEB-Manipulator.sln
├── README.md
└── LICENSE

markdown
Copy code

A **single Visual Studio solution** is used.  
Depending on the selected build configuration, the correct plugin output
(`dp32` or `dp64`) is generated automatically.

---

## 🔧 Build Requirements

- Windows 10 / 11
- **Visual Studio 2022** (MSVC)
- Windows SDK
- **x64dbg SDK**

### Required Links

- x64dbg Debugger  
  👉 https://x64dbg.com/

- x64dbg Plugin SDK  
  👉 https://github.com/x64dbg/x64dbg/tree/development/src/pluginsdk

- x64dbg Plugin Documentation  
  👉 https://help.x64dbg.com/en/latest/developers/plugins/index.html

---

## 🛠️ Build Instructions

1. Open `PEB-Manipulator.sln` in **Visual Studio 2022**
2. Select build configuration:
   - `Release | Win32` → builds **32-bit plugin**
   - `Release | x64` → builds **64-bit plugin**
3. Build the solution

### Output Files

PEB-Manipulator.dp32
PEB-Manipulator.dp64

yaml
Copy code

---

## 📦 Installation

1. Build the plugin **or** download from the **Releases** section
2. Copy the files to x64dbg plugin directories:

PEB-Manipulator.dp32 → x64dbg\plugins\x32
PEB-Manipulator.dp64 → x64dbg\plugins\x64\

yaml
Copy code

3. Restart **x64dbg**
4. Load the plugin from the **Plugins** menu

---

## ▶️ Usage

1. Open a process in x64dbg
2. Navigate to:

Plugins → PEB-Manipulator

yaml
Copy code

3. Select one of the available actions:
   - Read PEB fields
   - Patch anti-debug flags
   - Display command-line arguments

---

## 🚧 Project Status

This project is **actively evolving** and was created primarily for:

- Skill development
- Research
- Portfolio demonstration

If you have ideas, feature requests, or improvements,
**feel free to open an Issue or suggest enhancements**.

---

## ⚠️ Disclaimer

This project is provided **for educational and research purposes only**.

The author is **not responsible for any misuse** of this software.  
Use it **only on software you own or have explicit permission to analyze**.

---

## 📄 License

MIT License  
See the `LICENSE` file for details.

---

## 👤 Author

Developed by **Retr0**  
Telegram: [@AreYou_Lost](https://t.me/AreYou_Lost)
