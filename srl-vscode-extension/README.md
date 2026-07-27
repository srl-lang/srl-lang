# SRL (Serial Run Language) VS Code Extension

Official Visual Studio Code extension providing language support, syntax highlighting, bracket matching, and code snippets for **SRL (Serial Run Language)**.

## Features

- **Syntax Highlighting:** Full token coloring for SRL keywords (`fn`, `var`, `if`, `else`, `while`, `for`, `struct`, `return`), strings, numbers, and stdlib builtin functions.
- **Code Snippets:** Auto-completion snippets for functions, C-style `for` loops, `struct` declarations, C FFI calls, DSP FFT signal generation, and Qt Desktop GUI windows.
- **Language Configuration:** Automatic bracket pairing (`()`, `[]`, `{}`), quote insertion, and double-slash `//` comment toggling.

## Installation

### Manual Installation

1. Copy the `srl-vscode-extension` directory to your VS Code extensions folder:
   - **Windows:** `%USERPROFILE%\.vscode\extensions\srl-vscode-extension`
   - **Linux / macOS:** `~/.vscode/extensions/srl-vscode-extension`
2. Restart VS Code.
3. Open any `.srl` file to experience full syntax highlighting and snippet support.

## Extension Settings

This extension contributes `.srl` file association automatically.

## License

Distributed under the **GNU General Public License v3.0 (GPLv3)**.
