# SRL Toolchain Installation Guide (v0.3.0)

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

## 3. Graphical GUI Installer (Inno Setup)

For creating a standalone Windows installer wizard (`SRL_v0.3.0_Setup.exe`):
1. Install [Inno Setup](https://jrsoftware.org/isinfo.php).
2. Open [installer.iss](file:///c:/Users/emirt/Desktop/CPP/Stl/srl-lang/installer.iss) in Inno Setup Compiler.
3. Click **Compile** (F9) to generate the installer executable in `build_installer/`.

---

## 4. Verifying Installation

Open a fresh terminal window and verify:
```bash
srl version
```
Expected Output:
```text
SRL Language Toolchain v0.3.0 (LLVM IR + Self-Hosted Compiler + Bytecode VM + JIT Engine)
```
