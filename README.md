# SRL - Serial Run Language 🚀

**SRL (Serial Run Language)** is a high-performance, lightweight hybrid programming language designed for **Digital Signal Processing (DSP)**, **Real-Time Audio Analysis (FFT)**, **Live Hot-Reloading**, and **Standalone Native Code Generation**.

Featuring a C/Lua hybrid syntax, a fast C++ Bytecode VM, and its own standalone LLVM IR / Native Compiler (`srlc`), SRL combines developer agility with low-level execution speed.

---

## ✨ Key Features

- **⚡ Live Code Hot-Reloading with State Retention:** Modify `.srl` scripts on-the-fly while preserving runtime variable states. Perfect for live audio synthesis, game loops, and real-time monitoring.
- **🎵 Built-in DSP & FFT Engine:** Native Fourier Transforms (`dsp_fft`, `dsp_ifft`), signal generators (Sine, Square, Noise), windowing (Hann, Hamming), low-pass IIR filtering, and ASCII waveform terminal plotting (`dsp_plot`).
- **🧩 C++ Structs & Lua Metatables:** Combine C++ style struct definitions (`struct Vector2 { x, y }`) with Lua-style prototype inheritance and metamethods (`setmetatable`, `__index`, `__add`).
- **⚡ Standalone LLVM IR Compiler (`srlc`):** Compile `.srl` source code directly to LLVM IR (`.ll`) and standalone native x86_64 executable binaries without VM overhead.
- **🖥️ Interactive TUI Engine:** Built-in Terminal User Interface primitives (`tui_box`, `tui_progress`, `tui_color`, `tui_get_key`).
- **🎨 VS Code Extension:** Full syntax highlighting, snippet integration, and customized file icons (`srl-language-support-0.1.0.vsix`).

---

## 🚀 Quickstart Examples

### 1. Audio Signal Generation & Fast Fourier Transform (FFT)

```srl
// Generating 440 Hz Sine wave
var sample_rate = 8000;
var num_samples = 64;
var signal = dsp_sine(440, sample_rate, num_samples);

// ASCII Waveform Plot in Terminal
dsp_plot(signal, 8, "440Hz Sine Input Waveform");

// Compute Fast Fourier Transform
var fft_res = dsp_fft(signal);
var mag = dsp_magnitude(fft_res["real"], fft_res["imag"]);
dsp_plot(mag, 8, "Frequency Spectrum Magnitude");
```

### 2. C++ Structs & Lua Metatables

```srl
// C++ Style Struct Declaration
struct Point3D { x, y, z }

var p1 = Point3D(10, 20, 30);
print("Point coordinates: x=" + to_string(map_get(p1, "x")) + ", z=" + to_string(map_get(p1, "z")));

// Lua Style Metatable Fallback
var Prototype = map_new();
map_set(Prototype, "tag", "SRL_NATIVE_OBJECT");

var meta = map_new();
map_set(meta, "__index", Prototype);

setmetatable(p1, meta);
print("Inherited Prototype Tag: " + to_string(map_get(p1, "tag")));
```

---

## 🛠️ Building & Running SRL

### 1. Build the SRL Engine (Interpreter & VM)

```bash
git clone https://github.com/your-username/srl-lang.git
cd srl-lang
cmake -B build -S .
cmake --build build --config Release
```

Run an example script:
```bash
./build/Release/srl examples/dsp_demo.srl
```

### 2. Build the Standalone Native Compiler (`srlc`)

```bash
cd srlc
cmake -B build -S .
cmake --build build --config Release
```

Compile an SRL script directly into LLVM IR & Standalone Executable:
```bash
./srlc/build/Release/srlc examples/dsp_demo.srl -o dsp_demo.exe
```

Generate LLVM IR assembly:
```bash
./srlc/build/Release/srlc examples/dsp_demo.srl --emit-llvm
```

---

## 📦 VS Code Extension

Install the SRL language support extension located in `srl-vscode-extension`:
- Syntax Highlighting (`.srl`)
- Snippet Auto-completion
- `.vsix` pre-packaged bundle included!

---

## 📜 License

Distributed under the **MIT License**. See `LICENSE` for more information.
