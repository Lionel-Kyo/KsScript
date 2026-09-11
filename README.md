# KsScript
A Lightweight C++/MFC Macro Recorder, Editor & Script Launcher for Windows

![License](https://img.shields.io/badge/License-MIT-blue.svg)

---

## Overview

**KsScript** is a Windows desktop application built with C++ and MFC that allows users to record, edit, manage, and execute mouse and keyboard macro scripts. Designed for performance and ease of use, it features in-memory script pre-parsing, multi-file script management, and built-in reference guides for scripting functions and virtual key codes.

---

## Features

* **Script Manager & Launcher**: Load multiple UTF-8 `.txt` script files, assign hotkeys, and configure execution modes (*Single*, *Switch*, *Continuous*).
* **Interactive Macro Recorder**: Capture real-time mouse movements, clicks, and keystrokes directly into an editable script format.
* **In-Memory Syntax Validation**: Pre-parses and validates script code before execution to prevent mid-run failures.
* **Built-in Reference Manuals**:
  * **Functions Reference**: Quick API documentation for all supported macro commands.
  * **Keys Reference**: Virtual key codes (VK codes) mapping and syntax guide.

---

## UI Architecture & Dialogs

KsScript is structured around four primary dialog interfaces:

| Dialog | Resource ID | Description |
| :--- | :--- | :--- |
| **Main Launcher** | `IDD_MAIN_DIALOG` | The primary hub for loading scripts, managing active hotkeys, and launching reference windows or the recorder. |
| **Script Recorder** | `IDD_RECORD_DIALOG` | Records mouse/keyboard inputs in real-time and provides a multiline editor for script adjustments. |
| **Functions Reference** | `IDD_FUNCTIONS_REF_DIALOG` | Lists supported scripting functions, parameters, and syntax examples. |
| **Keys Reference** | `IDD_KEYS_REF_DIALOG` | Searchable table for key mapping and virtual key (VK) codes. |

---

## Script Syntax Quick Reference

Scripts are saved as UTF-8 text files (`.txt`). Commands are parsed line-by-line:

```
# Example KsScript Syntax
Delay(500)
AbsoluteMove(800, 600)
MouseDown(Left)
MouseUp(Left)
KeyboardDown(A)
KeyboardUp(A)
OutputText("Hello World!")
```
