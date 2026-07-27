# SRL (Serial Run Language) - Comprehensive Language Specification & Architecture Manual

**Version:** `v0.2.1`  
**Architecture:** C++ Bytecode Virtual Machine (VM) & LLVM IR Ahead-of-Time (AOT) Standalone Compiler (`srlc`)  
**License:** GNU General Public License v3.0 (GPLv3)

---

## 📑 Table of Contents
1. [Introduction & Language Philosophy](#1-introduction--language-philosophy)
2. [Basic Syntax & Data Types](#2-basic-syntax--data-types)
3. [Control Flow & Loops](#3-control-flow--loops)
4. [Functions & Scope](#4-functions--scope)
5. [Arrays & Maps](#5-arrays--maps)
6. [Structs & Dot-Access Syntax](#6-structs--dot-access-syntax)
7. [Lua-Style Metatables & Metamethods](#7-lua-style-metatables--metamethods)
8. [Digital Signal Processing (DSP) & FFT Engine](#8-digital-signal-processing-dsp--fft-engine)
9. [Terminal User Interface (TUI Module)](#9-terminal-user-interface-tui-module)
10. [Live Zero-Downtime Hot-Reloading](#10-live-zero-downtime-hot-reloading)
11. [Standalone LLVM Machine Code Compiler (`srlc`)](#11-standalone-llvm-machine-code-compiler-srlc)
12. [CLI Tooling Command Reference](#12-cli-tooling-command-reference)
13. [Extended Standard Libraries & Audio Engine](#13-extended-standard-libraries--audio-engine)
14. [C++ System-Level Capabilities (FFI, GFX, NET, SYS, Threads)](#14-c-system-level-capabilities-ffi-gfx-net-sys-threads)
15. [Core Libraries (JSON, Database, Cryptography & GUI Widgets)](#15-core-libraries-json-database-cryptography--gui-widgets)

---

## 1. Introduction & Language Philosophy

**SRL (Serial Run Language)** combines the simplicity and low-level performance of C with the flexible object model of Lua. It is a hybrid programming language specifically designed for **real-time signal processing (DSP)**, **audio synthesis**, **live code modification (hot-reloading)**, and **native standalone executable** generation.

### Key Design Principles:
- **Zero-Downtime Hot-Reloading:** Modify `.srl` source code live without stopping application state or resetting global variables.
- **Hardware Signal Performance:** Built-in Cooley-Tukey Radix-2 FFT algorithms, filters, and oscillators running at native C++ speed.
- **Dual Execution Engine:** Execute instantly using the fast C++ Bytecode VM or compile directly to LLVM IR and standalone x86_64 binaries with `srlc`.

---

## 2. Basic Syntax & Data Types

Variables in SRL are dynamically typed and declared using the `var` keyword.

### Supported Data Types:
- `NUMBER`: 64-bit double-precision floating-point number (IEEE-754).
- `STRING`: UTF-8 string sequences.
- `BOOL`: `true` or `false`.
- `NIL`: Null/undefined representation (`nil`).
- `ARRAY`: Dynamically sized heterogeneous lists.
- `MAP`: Key-value dictionaries (tables).
- `FUNCTION`: First-class function objects.

```srl
// Variable Declarations
var audio_freq = 440;            // Number
var channel_name = "Left";       // String
var is_active = true;            // Bool
var empty_val = nil;             // Nil

// String Concatenation & Type Conversion
var message = channel_name + " -> Frequency: " + to_string(audio_freq) + " Hz";
print(message);
```

---

## 3. Control Flow & Loops

### Conditional Statements (`if` / `else`):
```srl
var amplitude = 0.85;

if amplitude > 0.8 {
    print("High signal warning!");
} else {
    print("Normal signal level.");
}
```

### `while` Loop:
```srl
var i = 0;
while i < 5 {
    print("Step: " + to_string(i));
    i = i + 1;
}
```

### `for` Loop (v0.2.1):
```srl
for (var i = 0; i < 5; i = i + 1) {
    print("Index: " + to_string(i));
}
```

---

## 4. Functions & Scope

Functions are defined with the `fn` keyword and can return values using `return`.

```srl
fn compute_magnitude(real, imag) {
    var mag = math_abs(real) + math_abs(imag);
    return mag;
}

var result = compute_magnitude(12.5, -4.2);
print("Magnitude: " + to_string(result));
```

---

## 5. Arrays & Maps

SRL provides built-in native functions for dynamic arrays and map dictionaries:

### Array Functions:
- `arr_new()`: Creates a new empty array.
- `arr_push(arr, val)`: Appends an element to the end of the array.
- `arr_get(arr, index)`: Reads the value at index.
- `arr_set(arr, index, val)`: Sets the value at index (automatically resizes array if out of bounds).
- `arr_len(arr)`: Returns the array length.

```srl
var list = arr_new();
arr_push(list, 100);
arr_push(list, 200);
arr_set(list, 2, 300);

print("Array element [1]: " + to_string(arr_get(list, 1)));
print("Array length: " + to_string(arr_len(list)));
```

### Map Functions:
- `map_new()`: Creates a new map dictionary.
- `map_set(map, key, val)`: Sets key-value pair.
- `map_get(map, key)`: Retrieves value by key.
- `map_has(map, key)`: Checks if key exists (`true`/`false`).

```srl
var player = map_new();
map_set(player, "name", "Alice");
map_set(player, "score", 1500);

print("Player: " + map_get(player, "name") + " | Score: " + to_string(map_get(player, "score")));
```

---

## 6. Structs & Dot-Access Syntax

Custom data structures are defined using `struct`. SRL v0.2.1 supports natural dot notation (`obj.field`) for reading and writing struct fields:

```srl
// Struct Declaration
struct Point2D { x, y }
struct Vector3D { x, y, z }

// Object Instantiation
var p1 = Point2D(15, 25);
var v1 = Vector3D(1.0, 2.5, 9.8);

// Dot-Access (Read and Write)
p1.x = 100;
print("P1 X: " + to_string(p1.x) + ", Y: " + to_string(p1.y));
```

---

## 7. Lua-Style Metatables & Metamethods

SRL supports metatables for prototype-based inheritance and operator overloading on map objects:

### Metatable Functions:
- `setmetatable(obj, metatable)`: Binds a metatable to an object.
- `getmetatable(obj)`: Returns the object's metatable.
- `rawget(obj, key)`: Directly reads from the object bypassing `__index`.
- `rawset(obj, key, val)`: Directly sets property.

### Supported Metamethods:
- `__index`: Prototype lookup fallback when a key is missing on the target object.
- `__add`, `__sub`, `__mul`, `__div`: Triggered when binary operators are applied to map objects.

```srl
var Prototype = map_new();
map_set(Prototype, "type", "AUDIO_OBJECT");
map_set(Prototype, "sample_rate", 44100);

var meta = map_new();
map_set(meta, "__index", Prototype);

var audio_obj = map_new();
map_set(audio_obj, "channel", "Master");
setmetatable(audio_obj, meta);

print("Object Type: " + map_get(audio_obj, "type")); // Output: AUDIO_OBJECT
```

---

## 8. Digital Signal Processing (DSP) & FFT Engine

SRL includes a high-performance C++ DSP engine:

| Function | Description |
| :--- | :--- |
| `dsp_sine(freq, sample_rate, num_samples)` | Generates a sine wave signal array. |
| `dsp_square(freq, sample_rate, num_samples)` | Generates a square wave signal array. |
| `dsp_noise(num_samples)` | Generates white noise. |
| `dsp_hann(num_samples)` | Returns Hann window coefficients. |
| `dsp_hamming(num_samples)` | Returns Hamming window coefficients. |
| `dsp_lowpass(signal, cutoff, sample_rate)` | Applies low-pass filter. |
| `dsp_fft(real_array, [imag_array])` | Computes Cooley-Tukey Radix-2 FFT. Returns `{"real": [...], "imag": [...]}`. |
| `dsp_ifft(real_array, imag_array)` | Reconstructs signal using Inverse FFT. |
| `dsp_magnitude(real, imag)` | Computes spectrum magnitude ($|Z| = \sqrt{R^2 + I^2}$). |
| `dsp_plot(signal, height, title)` | Renders interactive ASCII waveform/spectrum plots in terminal. |

---

## 9. Terminal User Interface (TUI Module)

Built-in ANSI TUI commands for drawing interactive terminal panels:

- `tui_init()` / `tui_reset()`: Initializes / resets terminal mode.
- `tui_clear()`: Clears screen.
- `tui_color(color_name)`: Sets ANSI text color (`red`, `green`, `yellow`, `blue`, `magenta`, `cyan`, `white`).
- `tui_move(row, col)`: Moves cursor.
- `tui_box(row, col, width, height, title)`: Draws styled box frame.
- `tui_progress(row, col, width, percent)`: Renders progress bar.
- `tui_get_key()`: Non-blocking key press reader.

---

## 10. Live Zero-Downtime Hot-Reloading

Launch applications in watch mode:

```bash
srl watch app.srl
```

In watch mode:
1. When `app.srl` is edited and saved, the SRL runtime detects changes instantly.
2. Global state and heap variables are preserved while live function logic is patched.

---

## 11. Standalone LLVM Machine Code Compiler (`srlc`)

Compile scripts to standalone, standalone x86_64 binary executables using the self-hosted LLVM compiler:

```bash
# Build standalone binary
srl build app.srl -o app.exe

# Emit LLVM IR
srlc app.srl --emit-llvm
```

---

## 12. CLI Tooling Command Reference

| Command | Description |
| :--- | :--- |
| `srl run <file.srl>` | Executes script on SRL Bytecode VM. |
| `srl build <file.srl> [-o app.exe]` | Compiles script to native binary using LLVM. |
| `srl watch <file.srl>` | Launches script with live zero-downtime hot-reloading. |
| `srl version` | Displays SRL engine version. |
| `srl help` | Displays help menu. |

---

## 13. Extended Standard Libraries & Audio Engine

### A. Native Audio Engine (`audio_*`)
Plays MP3, WAV, and MID audio without external dependencies:
- `audio_play(filepath)`, `audio_pause()`, `audio_resume()`, `audio_stop()`
- `audio_set_volume(0..100)`, `audio_get_position()`, `audio_get_length()`
- `audio_beep(freq, duration_ms)`

### B. Desktop Native GUI Dialogs (`gui_*`)
- `gui_file_dialog_open(title, filter)`
- `gui_file_dialog_save(title, filter)`
- `gui_msgbox(title, message, type)`

### C. Directory & File System (`dir_*` & `file_*`)
- `dir_list(path)`, `dir_list_ext(path, extension)`, `dir_exists(path)`, `dir_create(path)`
- `file_write(path, content)`, `file_append(path, content)`, `file_remove(path)`, `file_size(path)`

---

## 14. C++ System-Level Capabilities (FFI, GFX, NET, SYS, Threads)

- **C/C++ FFI (`ffi_*`):** Dynamic DLL loading (`ffi_load`, `ffi_call`, `ffi_free`).
- **2D Hardware Graphics (`gfx_*`):** Double-buffered 2D canvas drawing (`gfx_window_create`, `gfx_fill_rect`, `gfx_present`).
- **Networking (`net_*`):** Socket and HTTP client tools (`net_http_get`, `net_tcp_connect`).
- **System Utilities (`sys_*`):** Core count, PID, memory metrics, shell execution (`sys_cpu_count`, `sys_memory_usage`, `sys_exec`).
- **Multithreading (`thread_*`):** Asynchronous worker threads (`thread_create`, `thread_join`).

---

## 15. Core Libraries (JSON, Database, Cryptography & GUI Widgets)

- **JSON Engine (`json_*`):** `json_parse(json_str)`, `json_stringify(value)`.
- **Key-Value Database (`db_*`):** `db_open`, `db_set`, `db_get`, `db_close`.
- **Cryptography (`crypto_*`):** `crypto_sha256`, `crypto_md5`, `crypto_base64_encode`, `crypto_base64_decode`.
- **GUI Toolkit (`std/widget.srl`):** Native interactive desktop widget components.

---

*SRL (Serial Run Language) v0.2.1 - GNU General Public License v3.0*
