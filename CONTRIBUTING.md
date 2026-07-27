# Contributing to SRL (Serial Run Language)

Thank you for your interest in contributing to **SRL (Serial Run Language)**! We welcome contributions from developers of all skill levels.

---

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [How Can I Contribute?](#how-can-i-contribute)
- [Development Setup](#development-setup)
- [Running Tests](#running-tests)
- [Coding Style & Guidelines](#coding-style--guidelines)
- [Submitting a Pull Request](#submitting-a-pull-request)

---

## Code of Conduct

This project and everyone participating in it is governed by the [SRL Code of Conduct](CODE_OF_CONDUCT.md). By participating, you are expected to uphold this code.

---

## How Can I Contribute?

- **Reporting Bugs:** Submit an issue using our [Bug Report Template](.github/ISSUE_TEMPLATE/bug_report.md).
- **Suggesting Features:** Propose new ideas or enhancements via the [Feature Request Template](.github/ISSUE_TEMPLATE/feature_request.md).
- **Improving Documentation:** Clarify manuals, update examples, or fix typos.
- **Submitting Code:** Fix open issues, add standard library functions, or optimize compiler passes.

---

## Development Setup

### Prerequisites

- **C++ Compiler:** Supporting C++17 or later (GCC 9+, Clang 10+, or MSVC 2019+).
- **Build System:** CMake 3.15+.
- **LLVM / Clang (Optional for Self-Host Native Builds):** LLVM 12+ for binary generation.

### Building from Source

```bash
# Clone the repository
git clone https://github.com/srl-lang/srl-lang.git
cd srl-lang

# Configure build with CMake
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build the executable
cmake --build build --config Release
```

---

## Running Tests

Always run the full test suite before submitting a Pull Request:

```bash
# Run Standard Library Tests
./build/srl run examples/test_libs.srl

# Run C++ Parity & System Capabilities Test
./build/srl run examples/test_cpp_features.srl

# Run Regression Fixes Test
./build/srl run examples/test_fixes.srl
```

---

## Coding Style & Guidelines

- **C++ Standard:** C++17.
- **Naming Conventions:**
  - `CamelCase` for Classes/Structs (`Lexer`, `Parser`, `Value`).
  - `camelCase` for methods and member functions.
  - `snake_case` for native SRL functions exposed in VM (`sys_exec`, `dsp_fft`).
- **Safety:** Prevent null pointer dereferences and ensure platform abstraction `#ifdef _WIN32` for Windows-specific APIs.

---

## Submitting a Pull Request

1. Fork the repository and create your branch from `main`:
   ```bash
   git checkout -b feat/your-feature-name
   ```
2. Make your changes and write/update tests.
3. Commit your changes with clear, descriptive commit messages:
   - `feat: Add new audio filter DSP module`
   - `fix: Fix socket handle leak on Linux`
   - `docs: Update standard library reference`
4. Push to your fork and submit a Pull Request against `main`.
