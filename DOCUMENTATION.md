# SRL (Serial Run Language) - Comprehensive Language Specification & Architecture Manual

**Version:** `v0.3.2`  
**Architecture:** High-Performance Bytecode Virtual Machine (VM), 100% Self-Hosted Compiler (`srlc`), & LLVM IR Standalone Compiler  
**License:** GNU General Public License v3.0 (GPLv3)

---

## Table of Contents
1. [Introduction & Language Philosophy](#1-introduction--language-philosophy)
2. [Basic Syntax & Data Types](#2-basic-syntax--data-types)
3. [Control Flow, Loops & Pattern Matching](#3-control-flow-loops--pattern-matching)
4. [Functions & Scope](#4-functions--scope)
5. [Classes, Access Controls & Custom Operator Overloading](#5-classes-access-controls--custom-operator-overloading)
6. [Structured Exception Handling (`try` / `catch` / `throw`)](#6-structured-exception-handling-try--catch--throw)
7. [Memory Overlays (`union`) & Generic Templates](#7-memory-overlays-union--generic-templates)
8. [Arrays, Maps & Structs](#8-arrays-maps--structs)
9. [Prototype Metatables & Metamethods](#9-prototype-metatables--metamethods)
10. [Digital Signal Processing (DSP) & FFT Engine](#10-digital-signal-processing-dsp--fft-engine)
11. [Terminal User Interface (TUI Module)](#11-terminal-user-interface-tui-module)
12. [Live Zero-Downtime Hot-Reloading](#12-live-zero-downtime-hot-reloading)
13. [100% Self-Hosted Compiler (`srlc`) & LLVM IR Generation](#13-100-self-hosted-compiler-srlc--llvm-ir-generation)
14. [CLI Tooling Command Reference](#14-cli-tooling-command-reference)
15. [Extended Standard Libraries & Native OS FFI (SYS, NET, Thread)](#15-extended-standard-libraries--native-os-ffi-sys-net-thread)
16. [Core Libraries (JSON, Database, Cryptography & GUI Widgets)](#16-core-libraries-json-database-cryptography--gui-widgets)
17. [Dynamic Native Plugin System & C ABI SDK](#17-dynamic-native-plugin-system--c-abi-sdk)

---

## 1. Introduction & Language Philosophy

**SRL (Serial Run Language)** is a hybrid programming language specifically designed for **real-time signal processing (DSP)**, **audio synthesis**, **live code modification (hot-reloading)**, **100% self-hosted native compilation**, and **standalone executable** generation.

### Key Design Principles:
- **100% Self-Hosted Architecture:** Bootstrapped compiler engine written entirely in SRL (`compiler/srlc.srl`).
- **Zero-Downtime Hot-Reloading:** Modify `.srl` source code live without stopping application state or resetting global variables.
- **Hardware Signal Performance:** Built-in Cooley-Tukey Radix-2 FFT algorithms, filters, and oscillators running at native hardware speed.
- **Dual Execution Engine:** Execute instantly using the fast Bytecode VM or compile directly to LLVM IR and standalone x86_64 binaries with `srlc`.

---

## 2. Basic Syntax & Data Types

Variables in SRL are dynamically typed and declared using the `var` keyword or immutable `const`.

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
const MAX_BUFFER = 4096;         // Const Immutability
var channel_name = "Left";       // String
var is_active = true;            // Bool
var empty_val = nil;             // Nil

// String Interpolation
var message = "${channel_name} -> Frequency: ${audio_freq} Hz";
print(message);
```

---

## 3. Control Flow, Loops & Pattern Matching

### Conditional Statements (`if` / `else`):
```srl
var amplitude = 0.85;

if amplitude > 0.8 {
    print("High signal warning!");
} else {
    print("Normal signal level.");
}
```

### Foreach `in` Loops:
```srl
// Array iteration
var items = arr_new();
arr_push(items, "apple");
arr_push(items, "banana");

for (var item in items) {
    print("Item: ${item}");
}

// Map key-value iteration
var config = map_new();
map_set(config, "name", "SRL");
map_set(config, "version", "0.3.2");

for (var key, val in config) {
    print("${key} = ${val}");
}
```

### Pattern Matching `match` Expression:
```srl
var status = 200;
var response = match (status) {
    case 200 => "OK",
    case 404 => "Not Found",
    default => "Unknown Error"
};
print("Status Result: ${response}");
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

## 5. Classes, Access Controls & Custom Operator Overloading

SRL supports full object-oriented programming with explicit encapsulation specifiers (`public:`, `private:`, `protected:`) and custom operator overloading (`fn operator+`, `fn operator-`, `fn operator*`, `fn operator/`, `fn operator[]`, `fn operator()`).

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

        fn operator*(scalar) {
            return Vector2D(this.x * scalar, this.y * scalar);
        }
}

var v1 = Vector2D(10, 20);
var v2 = Vector2D(5, 15);
var v3 = v1 + v2; // Calls operator+
print("Vector Sum: (${v3.x}, ${v3.y})");
```

---

## 6. Structured Exception Handling (`try` / `catch` / `throw`)

SRL includes structured exception handling for robust error recovery:

```srl
fn safe_divide(a, b) {
    if b == 0 {
        throw "DivisionByZeroError: Cannot divide by zero";
    }
    return a / b;
}

try {
    var val = safe_divide(100, 0);
} catch (err) {
    print("Caught Exception: ${err}");
}
```

---

## 7. Memory Overlays (`union`) & Generic Templates

### Memory Overlays (`union`):
Unions allow multiple named fields to share the same underlying memory offset:

```srl
union ColorOverlay {
    var int_value;
    var raw_value;
}

var col = ColorOverlay();
col.int_value = 3735928559; // 0xDEADBEEF
print("Raw Overlay Value: ${col.raw_value}");
```

### Modern Generic Templates:
```srl
class Container<T> {
    public:
        var item;

        fn init(val) {
            this.item = val;
        }
}

fn identity<T>(val: T): T {
    return val;
}

var num_box = Container<number>(100);
print("Boxed Item: ${num_box.item}");
```

---

## 8. Arrays, Maps & Structs

### Array Functions:
- `arr_new()`, `arr_push(arr, val)`, `arr_get(arr, index)`, `arr_set(arr, index, val)`, `arr_len(arr)`

### Map Functions:
- `map_new()`, `map_set(map, key, val)`, `map_get(map, key)`, `map_has(map, key)`

### Structs & Dot Access:
```srl
struct Point2D { x, y }
var p1 = Point2D(15, 25);
p1.x = 100;
print("Point X: ${p1.x}, Y: ${p1.y}");
```

---

## 9. Prototype Metatables & Metamethods

SRL supports metatables for prototype-based inheritance and operator overloading on map objects:
- `setmetatable(obj, metatable)`, `getmetatable(obj)`, `rawget(obj, key)`, `rawset(obj, key, val)`

---

## 10. Digital Signal Processing (DSP) & FFT Engine

| Function | Description |
| :--- | :--- |
| `dsp_sine(freq, sample_rate, num_samples)` | Generates a sine wave signal array. |
| `dsp_square(freq, sample_rate, num_samples)` | Generates a square wave signal array. |
| `dsp_noise(num_samples)` | Generates white noise. |
| `dsp_hann(num_samples)` | Returns Hann window coefficients. |
| `dsp_fft(real_array, [imag_array])` | Computes Cooley-Tukey Radix-2 FFT. Returns `{"real": [...], "imag": [...]}`. |
| `dsp_plot(signal, height, title)` | Renders interactive ASCII waveform/spectrum plots in terminal. |

---

## 11. Terminal User Interface (TUI Module)

Built-in ANSI TUI commands for drawing interactive terminal panels:
- `tui_init()`, `tui_clear()`, `tui_color(name)`, `tui_move(row, col)`, `tui_box(...)`, `tui_progress(...)`

---

## 12. Live Zero-Downtime Hot-Reloading

Launch applications in watch mode:

```bash
srl watch app.srl
```

In watch mode, global heap state is preserved while modified function logic is hot-swapped live.

---

## 13. 100% Self-Hosted Compiler (`srlc`) & LLVM IR Generation

The self-hosted compiler engine (`compiler/srlc.srl`) parses source code, constructs AST nodes, and bundles zero-dependency standalone binaries or emits native LLVM IR:

```bash
# Build zero-dependency standalone binary
srl build app.srl -o app.exe

# Static analysis and checking
srl check app.srl

# Format source code
srl fmt app.srl
```

---

## 14. CLI Tooling Command Reference

| Command | Description |
| :--- | :--- |
| `srl run <file.srl>` | Executes script on SRL Bytecode VM |
| `srl build <file.srl> [-o app.exe]` | Compiles script to zero-dependency standalone binary |
| `srl new <project_name>` | Scaffolds a new SRL project |
| `srl init` | Initializes project manifest (`srl.json`) |
| `srl check <file.srl>` | Performs static analysis and syntax denotation checks |
| `srl fmt <file.srl>` | Formats SRL source code |
| `srl clean` | Cleans temporary build artifacts |
| `srl watch <file.srl>` | Launches script with live zero-downtime hot-reloading |
| `srl version` | Displays SRL engine version |

---

## 15. Extended Standard Libraries & Native OS FFI (SYS, NET, Thread)

- **Platform & OS FFI ([std/sys.srl](file:///c:/Users/emirt/Desktop/CPP/Stl/srl-lang/std/sys.srl)):** `sys_is_windows()`, `sys_pid()`, `sys_sleep(ms)`.
- **Multi-Threading ([std/thread.srl](file:///c:/Users/emirt/Desktop/CPP/Stl/srl-lang/std/thread.srl)):** `NativeThread` class, `thread_create()`.
- **Networking ([std/net.srl](file:///c:/Users/emirt/Desktop/CPP/Stl/srl-lang/std/net.srl)):** Object-oriented `SocketClient` (`connect`, `send`, `receive`, `close`).

---

## 16. Core Libraries (JSON, Database, Cryptography & GUI Widgets)

- **JSON Engine (`json_*`):** `json_parse(json_str)`, `json_stringify(value)`.
- **Key-Value Database (`db_*`):** `db_open`, `db_set`, `db_get`, `db_close`.
- **Cryptography (`crypto_*`):** `crypto_sha256`, `crypto_md5`, `crypto_base64_encode`, `crypto_base64_decode`.
- **GUI Toolkit (`std/widget.srl`):** Native interactive desktop widget components.

---

## 17. Dynamic Native Plugin System & C ABI SDK

SRL allows extending the runtime with external native dynamic libraries (`.dll` on Windows, `.so` on Linux) without needing to recompile the main executable.

```srl
import("std/plugin.srl");
import_native("plugins/my_plugin.dll", "my_plugin");
```

---

*SRL (Serial Run Language) v0.3.2 - GNU General Public License v3.0*
