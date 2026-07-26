# SRL (Serial Run Language)

**SRL (Serial Run Language)** is a high-performance, lightweight hybrid programming language designed for **Digital Signal Processing (DSP)**, **Real-Time Audio Analysis (FFT)**, **Self-Hosted LLVM Compilation**, **Live Hot-Reloading**, and **System Tooling**.

Featuring a hybrid C/Lua syntax, a fast bytecode virtual machine, and a self-hosted compiler written in SRL (`compiler/srlc.srl`), SRL combines developer agility with low-level execution speed.

---

## Core Capabilities

- **Self-Hosted Compiler (`srlc.srl`):** Fully bootstrapped compiler written in SRL that parses source code, constructs ASTs, and generates standalone x86_64 LLVM IR assembly (`.ll`).
- **Package Manager (`srl pm`):** Built-in package initialization (`srl init`) and GitHub library installation (`srl install user/repo`) into `srl_modules/`.
- **Live Code Hot-Reloading:** Modify `.srl` scripts on-the-fly while preserving runtime variable states. Ideal for live audio synthesis and real-time monitoring.
- **Built-in DSP & FFT Engine:** Native Fast Fourier Transform (`dsp_fft`, `dsp_ifft`), signal generators (Sine, Square, Noise), windowing (Hann, Hamming), IIR filtering, and ASCII waveform plotting (`dsp_plot`).
- **Standard Library Suite:** Modular standard kütüphane components for vectors & 3D math (`std/math.srl`), REST HTTP requests (`std/http.srl`), assertions & testing (`std/testing.srl`), and OS environment utilities (`std/env.srl`).
- **Testing & Benchmarking:** Integrated unit test runner (`srl test`) and microsecond-precision benchmark engine (`srl bench`).
- **Structs & Metatables:** C-style struct definitions combined with Lua-style prototype inheritance and metamethod fallbacks.

---

## CLI Usage Guide

### 1. Run Script (Bytecode VM)

```bash
srl run examples/package_demo.srl
```

### 2. Self-Hosted Compiler Mode

```bash
srl compile examples/self_host_demo.srl -o self_demo.exe
```

### 3. Run Unit Test Suite

```bash
srl test examples/math.test.srl
```

### 4. Benchmark Script Performance

```bash
srl bench examples/package_demo.srl
```

### 5. Live Hot-Reloading Mode

```bash
srl watch examples/dsp_demo.srl
```

### 6. Initialize Package

```bash
srl init my_srl_app
```

---

## Code Examples

### 1. Audio Signal Generation & Fast Fourier Transform (FFT)

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

### 2. Math & Vector Operations

```srl
import("std/math.srl");

var v1 = vec2(3, 4);
var v2 = vec2(1, 2);
var v_sum = vec2_add(v1, v2);

print("Vector 1 Length: " + to_string(vec2_length(v1)));
print("Vector Sum: (" + to_string(map_get(v_sum, "x")) + ", " + to_string(map_get(v_sum, "y")) + ")");
```

### 3. Structs & Metatable Prototypes

```srl
struct Point3D { x, y, z }

var p1 = Point3D(10, 20, 30);
print("Point x: " + to_string(map_get(p1, "x")));

var Prototype = map_new();
map_set(Prototype, "tag", "SRL_NATIVE_OBJECT");

var meta = map_new();
map_set(meta, "__index", Prototype);
setmetatable(p1, meta);
```

---

## VS Code Extension

Language support extension located in `srl-vscode-extension`:
- Syntax Highlighting (`.srl`)
- Snippet Auto-completion
- `.vsix` pre-packaged installer bundle included

---

## License

Distributed under the **GNU General Public License v3.0 (GPLv3)**. See `LICENSE` for details.
