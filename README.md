# SRL (Serial Run Language)

**SRL (Serial Run Language)** is a high-performance, lightweight programming language designed for **Digital Signal Processing (DSP)**, **Real-Time Audio Analysis (FFT)**, **Universal Native Interoperability**, **Self-Hosted LLVM Compilation**, **Live Hot-Reloading**, and **Native Desktop GUI Tooling**.

Featuring a clean syntax, a fast bytecode virtual machine, and a 100% self-hosted native compiler written in SRL (`compiler/srlc.srl`), SRL combines developer agility with low-level execution speed.

> **Current Version:** `v0.3.1`

---

## Quick Installation

Automated installer scripts are available for all major platforms:

- **Windows (Batch / PowerShell):** Run [install.bat](install.bat) or `powershell .\install.ps1`
- **Linux & macOS (Bash):** Run `chmod +x install.sh && ./install.sh`
- **Detailed Installation Guide:** See [docs/INSTALLATION.md](docs/INSTALLATION.md) for PATH configurations and Inno Setup GUI installer generation.

---

## Technical Specifications & Documentation

All technical specifications, manuals, and developer guidelines are organized in the [`docs/`](docs/) directory:

- **[Technical Specification Manual](docs/SRL_LANGUAGE_MANUAL.md):** Complete technical manual covering VM architecture, compiler pipeline, memory management, exception handling, generics, and RTTI.
- **[Formal EBNF Syntax Grammar](docs/ebnf_grammar.md):** Formal Extended Backus-Naur Form grammar specification for compiler and parser implementers.
- **[API & Stdlib Reference](docs/api_reference.md):** Comprehensive reference for standard library functions and CLI commands.

---

## Core Capabilities

- **100% Self-Hosted Compiler (`srlc.srl`):** Bootstrapped compiler written entirely in SRL that tokenizes code, builds ASTs, and generates standalone LLVM IR binaries (`.ll` / `.exe`).
- **Modern Language Features:** Support for Operator Overloading, Class Access Controls (`public:`, `private:`, `protected:`), Exception Handling (`try/catch/throw`), Memory Overlays (`union`), and Generic Templates (`class Container<T>`).
- **Modern CLI Toolchain Engine:** Tooling commands (`srl new`, `srl init`, `srl check`, `srl fmt`, `srl clean`) with ANSI terminal color formatting.
- **Universal Native Interoperability & FFI:** Call any dynamic library (`.dll`, `.so`, `.dylib`) directly with `std/c.srl` and system FFI modules (`std/sys.srl`, `std/thread.srl`, `std/net.srl`).
- **Live Zero-Downtime Hot-Reloading (`srl watch`):** Modify `.srl` scripts on-the-fly while retaining active global runtime variable states. Ideal for live audio synthesis.
- **Built-in DSP & FFT Engine:** Native Fast Fourier Transform (`dsp_fft`, `dsp_ifft`), signal generators (Sine, Square, Noise), windowing (Hann, Hamming), IIR filters, and terminal ASCII plotting (`dsp_plot`).
- **Native Desktop Qt GUI (`std/qt.srl`):** Create desktop windows, widgets, and graphical dashboards directly from SRL scripts.
- **Package Manager (`srl pm`):** Project initialization (`srl init`) and GitHub package resolution (`srl install user/repo`) into `srl_modules/`.
- **Integrated Testing & Benchmarking:** Unit test runner (`srl test`) and microsecond-precision performance benchmark engine (`srl bench`).

---

## CLI Usage Guide

| Command | Description |
| :--- | :--- |
| `srl run <script.srl>` | Execute SRL script inside the Bytecode Virtual Machine |
| `srl compile <script.srl> [-o bin]` | Compile SRL script using self-hosted `compiler/srlc.srl` |
| `srl build <script.srl> [-o bin]` | Compile SRL script into standalone native binary |
| `srl new <project_name>` | Scaffold a new SRL project structure |
| `srl init [project_name]` | Initialize new SRL package manifest (`srl.json`) |
| `srl check <script.srl>` | Perform static type and syntax analysis |
| `srl fmt <script.srl>` | Format SRL source code |
| `srl clean` | Clean build artifacts and temporary binary outputs |
| `srl bind <header.h> [-o out.srl]` | Auto-generate SRL bindings from C header file |
| `srl watch <script.srl>` | Run SRL script with Live Hot-Reloading |
| `srl install <user/repo>` | Install dependency package from GitHub |
| `srl test [test_file.srl]` | Execute SRL test suite |
| `srl bench <script.srl>` | Benchmark script execution time & memory |

---

## Code Examples

### 1. Modern Generics & Custom Operator Overloading

```srl
class Vector2D {
    public:
        var x;
        var y;

        fn init(x_val, y_val) {
            this.x = x_val;
            this.y = y_val;
        }

        fn operator+(other) {
            return Vector2D(this.x + other.x, this.y + other.y);
        }
}

var v1 = Vector2D(10, 20);
var v2 = Vector2D(5, 15);
var v3 = v1 + v2; // Custom operator+ invocation
print("Result Vector: (${v3.x}, ${v3.y})");
```

### 2. Structured Exception Handling

```srl
fn divide(a, b) {
    if b == 0 {
        throw "DivisionByZeroError: Cannot divide by zero";
    }
    return a / b;
}

try {
    var res = divide(100, 0);
} catch (err) {
    print("Caught Exception: ${err}");
}
```

### 3. Native OS FFI & Process Operations

```srl
import("std/sys.srl");

print("Is Windows OS: " + to_string(sys_is_windows()));
print("Current Process ID: " + to_string(sys_pid()));
```

---

## Changelog

### v0.3.1 (Self-Hosted Release)
- **100% Self-Hosted Compiler Parity** — `compiler/srlc.srl` and `compiler/codegen_llvm.srl` completely bootstrapped for native executable generation.
- **Custom Operator Overloading** — Succeeded `fn operator+`, `fn operator-`, `fn operator*`, `fn operator/`, `fn operator[]`, `fn operator()`.
- **Class Access Control Specifiers** — Encapsulation with `public:`, `private:`, and `protected:` modifiers.
- **Structured Exception Handling** — `try`, `catch (err)`, and `throw` blocks.
- **Memory Overlays (Unions)** — Shared memory field overlays with `union`.
- **Modern Generics Syntax** — `class Container<T>` and `fn identity<T>(x: T)` support.
- **Modern Toolchain CLI Engine** — `srl new`, `srl init`, `srl check`, `srl fmt`, and `srl clean`.
- **Native FFI & OS Modules** — Platform-independent `std/sys.srl`, `std/thread.srl`, and `std/net.srl`.

---

## License

Distributed under the **GNU General Public License v3.0 (GPLv3)**. See `LICENSE` for details.
