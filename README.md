# KsScript
A Modern C++20 / DirectX 11 / Dear ImGui Macro Recorder, Editor & Script Launcher for Windows

![License](https://img.shields.io/badge/License-MIT-blue.svg)
![C++20](https://img.shields.io/badge/Language-C%2B%2B20-blue.svg)
![DirectX 11](https://img.shields.io/badge/DirectX-11-green.svg)
![Dear ImGui](https://img.shields.io/badge/GUI-Dear%20ImGui-orange.svg)

---

## Overview

**KsScript** is a high-performance Windows desktop application built with modern **C++20**, **DirectX 11**, and **Dear ImGui**. It enables users to record, edit, manage, and execute mouse and keyboard macro scripts in real-time.

Designed for responsiveness and ease of use, KsScript features in-memory script pre-parsing, multi-script management with customizable hotkeys, execution modes (*Single*, *Continuous*, *Switch*), a built-in macro recorder, comprehensive reference guides, and dynamic **runtime multi-language support (i18n)** across 10 languages.

---

## Features

* **Dynamic Multi-Language Support (i18n)**:
  * Switch languages on the fly at runtime without restarting or recompiling.
  * Auto-detects system UI language on initial launch and persists user preferences in `config.ini`.
  * Multi-font merging engine supporting Latin, CJK (Traditional, Simplified, Cantonese, Japanese), and Hangul (Korean) scripts.
  * Supports 10 languages:
    * English
    * Cantonese / 廣東話
    * Traditional Chinese / 繁體中文
    * implified Chinese / 简体中文
    * Japanese / 日本語
    * Korean / 한국어
    * Spanish / Español
    * Portuguese / Português
    * French / Français
    * German / Deutsch
* **Script Manager & Launcher**:
  * Load multiple UTF-8 `.txt` script files simultaneously.
  * Enable/disable scripts individually or in batches.
  * Assign independent start and stop hotkeys with modifiers (`Ctrl`, `Shift`, `Alt`).
  * Configure execution modes:
    * **Single** / **FullSingle**: Runs once per trigger.
    * **Continuous** / **FullContinuous**: Loops continuously until the stop key is pressed.
    * **Switch** / **FullSwitch**: Toggles execution on/off with the start hotkey.
* **Interactive Macro Recorder**:
  * Real-time recording of mouse movements, button clicks, wheel scrolls, and keystrokes.
  * Live playback preview before saving.
  * Built-in multi-line text editor with automatic synchronization.
* **In-Memory Syntax Validation**:
  * Pre-parses and validates script code upon loading to prevent mid-run failures.
  * Detailed syntax error diagnostics with line numbers and explanations in the active language.
* **Built-in Reference Guides**:
  * **Functions Reference**: In-app documentation covering function signatures, parameters, and descriptions for all supported commands.
  * **Keys Reference**: Virtual key (VK) codes and key names searchable table.

---

## User Interface

| Window | Description |
| :--- | :--- |
| **Main Window / Script Manager** | The primary dashboard to load scripts, configure hotkeys/execution modes, trigger test runs, manage active scripts, and change the UI language. |
| **Script Recorder** | Records keyboard and mouse inputs, provides real-time playback testing, and allows script editing before exporting. |
| **Functions Reference** | Interactive API reference table for built-in macro functions, parameter requirements, and usage examples. |
| **Keys Reference** | Searchable virtual key code (VK code) and key name mapping table. |

---

## Script Syntax Reference

Scripts are saved as UTF-8 text files (`.txt`). Each line represents a directive, comment, or action command.

### Header Directives

You can define script triggers and execution modes directly in script headers:

```ini
# Start key: F1, Stop key: F2, Run mode: Continuous
# SetStartKey(F1)
# SetEndKey(F2)
# SetScriptRunMode(Continuous)
```

*(Traditional and Simplified Chinese directive syntax are also supported, e.g. `# 設定開始按鍵(F1)`, `# 設定停止按鍵(F2)`, `# 設定運行模式(連續)`)*

### Supported Commands

| Command | Arguments | Description |
| :--- | :--- | :--- |
| `Delay` | `(ms)` | Pauses execution for the specified milliseconds. |
| `AbsoluteMove` | `(x, y)` | Moves the mouse cursor to absolute screen coordinates `(x, y)`. |
| `RelativeMove` | `(dx, dy)` | Moves the mouse cursor relative to current position by `(dx, dy)`. |
| `MouseDown` | `(button)` | Presses down a mouse button (`Left`, `Right`, `Middle`, `X1`, `X2`). |
| `MouseUp` | `(button)` | Releases a mouse button (`Left`, `Right`, `Middle`, `X1`, `X2`). |
| `KeyboardDown` | `(key)` | Presses down a key by name or virtual key (VK) code. |
| `KeyboardUp` | `(key)` | Releases a key by name or virtual key (VK) code. |
| `WheelDown` | `()` | Scrolls the mouse wheel downwards. |
| `WheelUp` | `()` | Scrolls the mouse wheel upwards. |
| `HoldTitleBar` | `(windowName)` | Simulates holding down the title bar of the specified window. |
| `ReleaseTitleBar` | `(windowName)` | Simulates releasing the title bar of the specified window. |
| `OutputText` | `(text, [delayMs])` | Types text with optional character delay (`'...'` for literal text, `"..."` for escape sequences). |
| `ShowCoordinate` | `()` | Displays current mouse cursor coordinates. |

### Script Example

```text
# SetStartKey(F1)
# SetEndKey(F2)
# SetScriptRunMode(Single)

# Move to button and click
AbsoluteMove(500, 300)
Delay(100)
MouseDown(Left)
Delay(50)
MouseUp(Left)

# Type text
Delay(200)
OutputText("Hello!\n", 20)
```

---

## Building from Source

### Prerequisites

* **OS**: Windows 10 / 11 (x64)
* **IDE**: Visual Studio 2022 (Community or higher)
* **Workloads**: Desktop development with C++
* **SDK / Toolset**: C++20 standard, Windows 10/11 SDK, DirectX 11

### Build Steps

1. Clone the repository:
   ```bash
   git clone https://github.com/Lionel-Kyo/KsScript.git
   cd KsScript
   ```
2. Open `src/KsScript.sln` in Visual Studio 2022.
3. Select **Release** and **x64** configuration.
4. Build the solution (`Ctrl+Shift+B` or via MSBuild):
   ```powershell
   & "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" src\KsScript.sln /p:Configuration=Release /p:Platform=x64
   ```
5. The compiled executable will be generated at `src/x64/Release/KsScript.exe`.

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
