# SRL (Serial Run Language)

**SRL (Serial Run Language)** is a high-performance, lightweight hybrid programming language designed for **Digital Signal Processing (DSP)**, **Real-Time Audio Analysis (FFT)**, **Universal C/C++ Interoperability**, **Self-Hosted LLVM Compilation**, **Live Hot-Reloading**, and **Native Desktop GUI Tooling**.

Featuring a hybrid C/Lua syntax, a fast bytecode virtual machine, and a self-hosted compiler written in SRL (`compiler/srlc.srl`), SRL combines developer agility with low-level execution speed.

> **Current Version:** `v0.3.0`

---

## Quick Installation

Automated installer scripts are available for all major platforms:

- **Windows (Batch / PowerShell):** Run [install.bat](install.bat) or `powershell .\install.ps1`
- **Linux & macOS (Bash):** Run `chmod +x install.sh && ./install.sh`
- **Detailed Installation Guide:** See [docs/INSTALLATION.md](docs/INSTALLATION.md) for PATH configurations and Inno Setup GUI installer generation.

---

## Documentation & Specifications

All technical specifications, manuals, and developer guidelines are organized in the [`docs/`](docs/) directory:

- **[Technical Specification Manual](docs/SRL_LANGUAGE_MANUAL.md):** Complete technical manual covering VM architecture, compiler pipeline, memory management, exception handling, generics, and RTTI.
- **[Formal EBNF Syntax Grammar](docs/ebnf_grammar.md):** Formal Extended Backus-Naur Form grammar specification for compiler and parser implementers.
- **[API & Stdlib Reference](docs/api_reference.md):** Comprehensive reference for standard library functions and CLI commands.

---

## Core Capabilities

- **Universal C/C++ Interoperability:** Call any C dynamic library (`.dll`, `.so`, `.dylib`) directly with `std/c.srl`, auto-generate SRL wrappers from `.h` headers using `srl bind`, or build native C++ extensions via `srl_plugin.h`.
- **Self-Hosted Compiler (`srlc.srl`):** Bootstrapped compiler written entirely in SRL that tokenizes code, builds ASTs, and generates standalone LLVM IR binaries (`.ll` / `.exe`).
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
| `srl bind <header.h> [-o out.srl]` | Auto-generate SRL bindings from C header file |
| `srl watch <script.srl>` | Run SRL script with Live Hot-Reloading |
| `srl init [project_name]` | Initialize new SRL package manifest (`srl.json`) |
| `srl install <user/repo>` | Install dependency package from GitHub |
| `srl test [test_file.srl]` | Execute SRL test suite |
| `srl bench <script.srl>` | Benchmark script execution time & memory |
| `srl doc` | Auto-generate Markdown documentation from `///` comments |

---

## Code Examples

### 1. Universal C Library Call & Memory Allocation

```srl
import("std/c.srl");

var c_lib = c_open("ucrtbase.dll");
var sqrt_res = c_call1(c_lib, "sqrt", "double", 144.0); // 12.0

var ptr = c_malloc(128);
c_write_string(ptr, "Hello C Memory from SRL!");
print(c_read_string(ptr));
c_free(ptr);
c_close(c_lib);
```

### 2. Audio Signal Generation & Fast Fourier Transform (FFT)

```srl
var sample_rate = 8000;
var num_samples = 64;
var signal = dsp_sine(440, sample_rate, num_samples);

// ASCII Waveform Plot in Terminal
dsp_plot(signal, 8, "440Hz Sine Waveform");

// Compute Fast Fourier Transform
var fft_res = dsp_fft(signal);
var mag = dsp_magnitude(fft_res["real"], fft_res["imag"]);
dsp_plot(mag, 8, "Frequency Spectrum");
```

### 3. Native Qt Desktop GUI

```srl
import("std/qt.srl");

qt_app_init();
var win = qt_window("SRL Qt Application", 450, 350);
qt_exec();
```

### 4. `for` Loop (v0.2.1)

```srl
// C-style for loop — counts from 0 to 4
for (var i = 0; i < 5; i = i + 1) {
    print("Step: " + to_string(i));
}
```

### 5. Struct Field Dot-Access (v0.2.1)

```srl
struct Vec2 { x, y }
var v = Vec2(3.0, 4.0);

// Natural dot notation — no map_get() needed
print(v.x);  // 3.0
v.y = 99.0;
print(v.y);  // 99.0
```

---

## Changelog

### v0.3.0
- **`foreach` / `in` loop** — `for (var item in list)` and `for (var key, val in map)` array and map iteration syntax
- **String Interpolation** — `${expr}` syntax embedded in string literals (e.g. `"Pos: ${p.x}, ${p.y}"`)
- **Pattern Matching** — `match (val) { case pattern => result, default => defaultResult }` expression syntax
- **Self-Hosted Compiler Parity** — `compiler/*.srl` synchronized with all v0.3.0 language features
- **VS Code IDE Extension Updates** — Syntax highlighting and snippets updated for `in`, `match`, `case`, `default` and string interpolation

### v0.2.1
- **`for` loop** — C-style `for (init; cond; incr)` loop added to lexer, parser and compiler
- **Dot-access** — `obj.field` read/write syntax replaces `map_get`/`map_set` for struct fields
- **Line-numbered errors** — Runtime error messages now include `[Line X]` source location
- **Float modulo** — `%` operator now uses `fmod()` for correct float behaviour (e.g. `3.7 % 1.2`)
- **English-only codebase** — All source code, comments, and example strings are now in English

## VS Code IDE Extension

Language support extension located in `srl-vscode-extension`:
- Syntax Highlighting (`.srl`)
- Snippet Auto-completion
- Pre-packaged `.vsix` installer included

---

## License

Distributed under the **GNU General Public License v3.0 (GPLv3)**. See `LICENSE` for details.
