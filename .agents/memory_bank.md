# SRL Memory Bank

## Project Status
- Current Version: v0.2.1
- Architecture: C++17 Bytecode VM & Self-Hosted SRL Compiler (`compiler/srlc.srl`)
- Build System: CMake 3.15+ (Cross-platform Windows & Linux)
- Continuous Integration: GitHub Actions workflow (`.github/workflows/ci.yml`)

## Recent Achievements
- Added C-style `for` loops and struct field dot-access notation (`v.x`).
- Integrated FFI engine (`std/c.srl`), Qt GUI (`std/qt.srl`), DSP/FFT, and Net modules.
- Created official VS Code language extension (`srl-vscode-extension`).
- Completed 100% GitHub Community Standards (Code of Conduct, Contributing, Security, Issue & PR templates).
- Standardized contact email to `help@srl-info.com`.
- Enforced strict Zero Emoji policy across all documentation, code, console logs, and AI responses.

## Key Decisions
- No Emojis: All emojis are strictly removed and prohibited across the repository.
- Cross-Platform Sockets: `srl_net.cpp` uses POSIX socket fallbacks on Linux.
