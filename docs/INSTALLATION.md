# SRL Toolchain Installation Guide (v0.3.2)

This guide covers installing the SRL (Serial Run Language) toolchain across Windows, Linux, and macOS.

---

## 1. Quick Automated Installation (Windows)

### Option A: PowerShell Automated Installer
Open PowerShell and run:
```powershell
.\install.ps1
```

### Option B: Double-Click Batch Installer
Simply double-click or run [install.bat](file:///c:/Users/emirt/Desktop/CPP/Stl/srl-lang/install.bat) in Command Prompt:
```cmd
install.bat
```

> **What the Windows Installer does:**
> 1. Compiles SRL binaries (`srl.exe`) using CMake.
> 2. Deploys executable and libraries to `%USERPROFILE%\.srl\bin`.
> 3. Automatically registers `%USERPROFILE%\.srl\bin` in your User Environment `PATH`.

---

## 2. POSIX Automated Installation (Linux & macOS)

Run the installer bash script:
```bash
chmod +x install.sh
./install.sh
```

---

## 3. Native Modern GUI Custom Setup Wizard

SRL features a 100% native modern GUI setup wizard built in C++ and SRL:

### Option A: Modern Graphical GUI Installer Wizard
Simply double-click or run the compiled native installer binary:
```cmd
srl-installer.exe
```
This opens a modern setup wizard featuring path selection, browse dialog, animated progress bar, real-time log, desktop shortcut creation, and automatic `PATH` environment setup.

### Option B: Universal Project GUI Installer Generator (`srl installer`)
You can auto-generate a standalone modern GUI setup installer (`app_setup.exe`) for **ANY** SRL project:

**CLI Command:**
```cmd
srl installer main.srl -o my_app_setup.exe --name "My SRL Application"
```

**Programmatic SRL API (`std/installer.srl`):**
```srl
import("std/installer.srl");

var builder = create_installer("My Custom App", "1.0.0");
builder.set_script("src/main.srl");
builder.build("my_app_setup.exe");
```



---

## 4. Verifying Installation

Open a fresh terminal window and verify:
```bash
srl version
```
Expected Output:
```text
SRL Language Toolchain v0.3.2 (100% Self-Hosted Compiler + Standalone Bundler + Bytecode VM + JIT Engine)
```
