#include "vm.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "compiler.hpp"
#include "tui.hpp"
#include "dsp.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <cmath>

namespace srl {

static Value getMetamethod(const Value& val, const std::string& eventName) {
    if (val.isMap()) {
        auto map = val.asMap();
        auto itMeta = map->find("__metatable");
        if (itMeta != map->end() && itMeta->second.isMap()) {
            auto metaMap = itMeta->second.asMap();
            auto itEv = metaMap->find(eventName);
            if (itEv != metaMap->end()) {
                return itEv->second;
            }
        }
    }
    return Value();
}

VM::VM() {
    registerNativeFunctions();
}

void VM::registerNativeFunctions() {
    DSP::registerNativeFunctions(*this);

    // print(...)
    defineNative("print", [](int argCount, const Value* args) -> Value {
        for (int i = 0; i < argCount; ++i) {
            std::cout << args[i].toString();
            if (i < argCount - 1) std::cout << " ";
        }
        std::cout << std::endl;
        return Value();
    });

    // sleep_ms(milliseconds)
    defineNative("sleep_ms", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isNumber()) {
            int ms = static_cast<int>(args[0].asNumber());
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        }
        return Value();
    });

    // to_string(val)
    defineNative("to_string", [](int argCount, const Value* args) -> Value {
        if (argCount > 0) {
            return Value(args[0].toString());
        }
        return Value("");
    });

    // time()
    defineNative("time", [](int argCount, const Value* args) -> Value {
        auto now = std::chrono::system_clock::now().time_since_epoch();
        double seconds = std::chrono::duration_cast<std::chrono::milliseconds>(now).count() / 1000.0;
        return Value(seconds);
    });

    // math_abs(val)
    defineNative("math_abs", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isNumber()) {
            return Value(std::abs(args[0].asNumber()));
        }
        return Value(0.0);
    });

    // --- TUI (Text User Interface) Operations ---
    defineNative("tui_init", [](int argCount, const Value* args) -> Value {
        TUI::init();
        TUI::hideCursor();
        return Value();
    });

    defineNative("tui_clear", [](int argCount, const Value* args) -> Value {
        TUI::clear();
        return Value();
    });

    defineNative("tui_move", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isNumber() && args[1].isNumber()) {
            TUI::moveCursor(static_cast<int>(args[0].asNumber()), static_cast<int>(args[1].asNumber()));
        }
        return Value();
    });

    defineNative("tui_color", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isString()) {
            std::string c = args[0].asString();
            if (c == "red") TUI::setColor(91);
            else if (c == "green") TUI::setColor(92);
            else if (c == "yellow") TUI::setColor(93);
            else if (c == "blue") TUI::setColor(94);
            else if (c == "magenta") TUI::setColor(95);
            else if (c == "cyan") TUI::setColor(96);
            else if (c == "white") TUI::setColor(97);
            else TUI::resetColor();
        } else {
            TUI::resetColor();
        }
        return Value();
    });

    defineNative("tui_print", [](int argCount, const Value* args) -> Value {
        if (argCount >= 3 && args[0].isNumber() && args[1].isNumber()) {
            int row = static_cast<int>(args[0].asNumber());
            int col = static_cast<int>(args[1].asNumber());
            TUI::moveCursor(row, col);
            std::cout << args[2].toString() << std::flush;
        }
        return Value();
    });

    defineNative("tui_box", [](int argCount, const Value* args) -> Value {
        if (argCount >= 5 && args[0].isNumber() && args[1].isNumber() && args[2].isNumber() && args[3].isNumber()) {
            int row = static_cast<int>(args[0].asNumber());
            int col = static_cast<int>(args[1].asNumber());
            int width = static_cast<int>(args[2].asNumber());
            int height = static_cast<int>(args[3].asNumber());
            std::string title = args[4].toString();

            // Draw Top Border
            TUI::moveCursor(row, col);
            std::cout << "+";
            int titleLen = static_cast<int>(title.length());
            int startTitle = (width - titleLen - 2) / 2;
            for (int i = 0; i < width - 2; ++i) {
                if (i == startTitle && !title.empty()) {
                    std::cout << " " << title << " ";
                    i += titleLen + 1;
                } else {
                    std::cout << "-";
                }
            }
            std::cout << "+";

            // Draw Sides
            for (int r = 1; r < height - 1; ++r) {
                TUI::moveCursor(row + r, col);
                std::cout << "|";
                TUI::moveCursor(row + r, col + width - 1);
                std::cout << "|";
            }

            // Draw Bottom Border
            TUI::moveCursor(row + height - 1, col);
            std::cout << "+";
            for (int i = 0; i < width - 2; ++i) std::cout << "-";
            std::cout << "+" << std::flush;
        }
        return Value();
    });

    defineNative("tui_progress", [](int argCount, const Value* args) -> Value {
        if (argCount >= 4 && args[0].isNumber() && args[1].isNumber() && args[2].isNumber() && args[3].isNumber()) {
            int row = static_cast<int>(args[0].asNumber());
            int col = static_cast<int>(args[1].asNumber());
            int width = static_cast<int>(args[2].asNumber());
            double percent = args[3].asNumber(); // 0 to 100

            if (percent < 0) percent = 0;
            if (percent > 100) percent = 100;

            int filled = static_cast<int>((width * percent) / 100.0);

            TUI::moveCursor(row, col);
            std::cout << "[";
            TUI::setColor(92); // Green
            for (int i = 0; i < filled; ++i) std::cout << "#";
            TUI::setColor(90); // Dark grey
            for (int i = filled; i < width; ++i) std::cout << "-";
            TUI::resetColor();
            std::cout << "] " << static_cast<int>(percent) << "% " << std::flush;
        }
        return Value();
    });

    defineNative("tui_key_pressed", [](int argCount, const Value* args) -> Value {
        return Value(TUI::keyPressed());
    });

    defineNative("tui_get_key", [](int argCount, const Value* args) -> Value {
        int key = TUI::getKey();
        if (key != -1) {
            return Value(std::string(1, static_cast<char>(key)));
        }
        return Value("");
    });

    defineNative("tui_reset", [](int argCount, const Value* args) -> Value {
        TUI::resetColor();
        TUI::showCursor();
        return Value();
    });

    // --- Array Operations ---
    defineNative("arr_new", [](int argCount, const Value* args) -> Value {
        return Value(std::make_shared<std::vector<Value>>());
    });

    defineNative("arr_push", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isArray()) {
            args[0].asArray()->push_back(args[1]);
        }
        return Value();
    });

    defineNative("arr_get", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isArray() && args[1].isNumber()) {
            size_t idx = static_cast<size_t>(args[1].asNumber());
            auto arr = args[0].asArray();
            if (idx < arr->size()) return (*arr)[idx];
        }
        return Value();
    });

    defineNative("arr_set", [](int argCount, const Value* args) -> Value {
        if (argCount >= 3 && args[0].isArray() && args[1].isNumber()) {
            size_t idx = static_cast<size_t>(args[1].asNumber());
            auto arr = args[0].asArray();
            if (idx >= arr->size()) {
                arr->resize(idx + 1);
            }
            (*arr)[idx] = args[2];
        }
        return Value();
    });

    defineNative("arr_len", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isArray()) {
            return Value(static_cast<double>(args[0].asArray()->size()));
        }
        return Value(0.0);
    });

    // --- Map Operations ---
    defineNative("map_new", [](int argCount, const Value* args) -> Value {
        return Value(std::make_shared<std::unordered_map<std::string, Value>>());
    });

    defineNative("map_set", [](int argCount, const Value* args) -> Value {
        if (argCount >= 3 && args[0].isMap()) {
            (*args[0].asMap())[args[1].toString()] = args[2];
        }
        return Value();
    });

    defineNative("map_get", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isMap()) {
            auto map = args[0].asMap();
            std::string key = args[1].toString();
            auto it = map->find(key);
            if (it != map->end()) return it->second;

            Value mm = getMetamethod(args[0], "__index");
            if (mm.isMap()) {
                auto indexMap = mm.asMap();
                auto itIdx = indexMap->find(key);
                if (itIdx != indexMap->end()) return itIdx->second;
            } else if (mm.isNativeFn()) {
                return mm.asNativeFn()(2, args);
            }
        }
        return Value();
    });

    defineNative("map_has", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isMap()) {
            return Value(args[0].asMap()->count(args[1].toString()) > 0);
        }
        return Value(false);
    });

    // --- Metatable Operations ---
    defineNative("setmetatable", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isMap() && args[1].isMap()) {
            (*args[0].asMap())["__metatable"] = args[1];
            return args[0];
        }
        return Value();
    });

    defineNative("getmetatable", [](int argCount, const Value* args) -> Value {
        if (argCount >= 1 && args[0].isMap()) {
            auto map = args[0].asMap();
            auto it = map->find("__metatable");
            if (it != map->end()) return it->second;
        }
        return Value();
    });

    defineNative("rawget", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isMap()) {
            auto map = args[0].asMap();
            auto it = map->find(args[1].toString());
            if (it != map->end()) return it->second;
        }
        return Value();
    });

    defineNative("rawset", [](int argCount, const Value* args) -> Value {
        if (argCount >= 3 && args[0].isMap()) {
            (*args[0].asMap())[args[1].toString()] = args[2];
            return args[0];
        }
        return Value();
    });

    // --- String Operations ---
    defineNative("str_len", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isString()) {
            return Value(static_cast<double>(args[0].asString().length()));
        }
        return Value(0.0);
    });

    defineNative("str_sub", [](int argCount, const Value* args) -> Value {
        if (argCount >= 3 && args[0].isString() && args[1].isNumber() && args[2].isNumber()) {
            size_t start = static_cast<size_t>(args[1].asNumber());
            size_t len = static_cast<size_t>(args[2].asNumber());
            const std::string& str = args[0].asString();
            if (start < str.length()) {
                return Value(str.substr(start, len));
            }
        }
        return Value("");
    });

    defineNative("str_char", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isString() && args[1].isNumber()) {
            size_t idx = static_cast<size_t>(args[1].asNumber());
            const std::string& str = args[0].asString();
            if (idx < str.length()) {
                return Value(std::string(1, str[idx]));
            }
        }
        return Value("");
    });

    defineNative("str_code", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isString() && !args[0].asString().empty()) {
            return Value(static_cast<double>(static_cast<unsigned char>(args[0].asString()[0])));
        }
        return Value(0.0);
    });

    defineNative("str_from_code", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isNumber()) {
            char c = static_cast<char>(args[0].asNumber());
            return Value(std::string(1, c));
        }
        return Value("");
    });

    // --- File I/O Operations ---
    defineNative("file_read", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isString()) {
            std::ifstream file(args[0].asString());
            if (file.is_open()) {
                std::stringstream buffer;
                buffer << file.rdbuf();
                return Value(buffer.str());
            }
        }
        return Value();
    });

    defineNative("file_write", [](int argCount, const Value* args) -> Value {
        if (argCount >= 2 && args[0].isString() && args[1].isString()) {
            std::ofstream file(args[0].asString());
            if (file.is_open()) {
                file << args[1].asString();
                return Value(true);
            }
        }
        return Value(false);
    });

    defineNative("file_exists", [](int argCount, const Value* args) -> Value {
        if (argCount > 0 && args[0].isString()) {
            return Value(std::filesystem::exists(args[0].asString()));
        }
        return Value(false);
    });
}

void VM::defineNative(const std::string& name, NativeFn fn) {
    env_.defineGlobal(name, Value(fn));
}

void VM::push(Value value) {
    stack_.push_back(std::move(value));
}

Value VM::pop() {
    if (stack_.empty()) {
        throw std::runtime_error("Stack underflow");
    }
    Value val = std::move(stack_.back());
    stack_.pop_back();
    return val;
}

Value VM::peek(int distance) const {
    return stack_[stack_.size() - 1 - distance];
}

InterpretResult VM::interpret(const std::string& source) {
    try {
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();

        Parser parser(tokens);
        auto statements = parser.parse();

        Compiler compiler;
        auto [scriptChunk, functions] = compiler.compile(statements);

        // Store compiled functions into global symbol table
        for (const auto& fnObj : functions) {
            env_.setGlobal(fnObj->name, Value(fnObj));
        }

        // Run top-level script statements
        auto scriptFn = std::make_shared<FunctionObject>();
        scriptFn->name = "main_script";
        scriptFn->arity = 0;

        // Execute top level script
        CallFrame frame;
        frame.function = scriptFn;
        frame.chunk = &scriptChunk;
        frame.ip = 0;
        frame.stackOffset = stack_.size();
        frames_.push_back(frame);

        return run();
    } catch (const std::exception& e) {
        std::cerr << "[VM Error] " << e.what() << std::endl;
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
    }
}

InterpretResult VM::runFunction(const std::string& fnName) {
    Value fnValue;
    if (!env_.getGlobal(fnName, fnValue) || !fnValue.isFunction()) {
        return InterpretResult::INTERPRET_RUNTIME_ERROR;
    }

    auto fnObj = fnValue.asFunction();

    // Call the function
    Value callVal = fnValue;
    push(callVal); // push function object onto stack

    CallFrame frame;
    frame.function = fnObj;
    // For hot reloaded functions, we execute their body by calling them
    return InterpretResult::INTERPRET_OK;
}

InterpretResult VM::run() {
    CallFrame* frame = &frames_.back();

    #define READ_BYTE() (frame->chunk->code[frame->ip++])
    #define READ_CONSTANT() (frame->chunk->constants[READ_BYTE()])
    #define READ_SHORT() (frame->ip += 2, (uint16_t)((frame->chunk->code[frame->ip - 2] << 8) | frame->chunk->code[frame->ip - 1]))

    while (true) {
        if (frame->ip >= frame->chunk->code.size()) {
            frames_.pop_back();
            if (frames_.empty()) return InterpretResult::INTERPRET_OK;
            frame = &frames_.back();
            continue;
        }

        OpCode instruction = static_cast<OpCode>(READ_BYTE());
        switch (instruction) {
            case OpCode::OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }
            case OpCode::OP_NIL: push(Value()); break;
            case OpCode::OP_TRUE: push(Value(true)); break;
            case OpCode::OP_FALSE: push(Value(false)); break;
            case OpCode::OP_POP: pop(); break;

            case OpCode::OP_DEFINE_GLOBAL: {
                Value nameVal = READ_CONSTANT();
                Value value = pop();
                env_.defineGlobal(nameVal.asString(), value);
                break;
            }

            case OpCode::OP_GET_GLOBAL: {
                Value nameVal = READ_CONSTANT();
                Value value;
                if (!env_.getGlobal(nameVal.asString(), value)) {
                    std::cerr << "[Runtime Error] Undefined variable '" << nameVal.asString() << "'." << std::endl;
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                push(value);
                break;
            }

            case OpCode::OP_SET_GLOBAL: {
                Value nameVal = READ_CONSTANT();
                Value val = peek(0);
                env_.setGlobal(nameVal.asString(), val);
                break;
            }

            case OpCode::OP_GET_LOCAL: {
                uint8_t slot = READ_BYTE();
                push(stack_[frame->stackOffset + slot]);
                break;
            }

            case OpCode::OP_SET_LOCAL: {
                uint8_t slot = READ_BYTE();
                stack_[frame->stackOffset + slot] = peek(0);
                break;
            }

            case OpCode::OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(Value(a.equals(b)));
                break;
            }

            case OpCode::OP_GREATER: {
                Value b = pop();
                Value a = pop();
                if (a.isNumber() && b.isNumber()) {
                    push(Value(a.asNumber() > b.asNumber()));
                } else {
                    push(Value(false));
                }
                break;
            }

            case OpCode::OP_LESS: {
                Value b = pop();
                Value a = pop();
                if (a.isNumber() && b.isNumber()) {
                    push(Value(a.asNumber() < b.asNumber()));
                } else {
                    push(Value(false));
                }
                break;
            }

            case OpCode::OP_ADD: {
                Value b = pop();
                Value a = pop();
                if (a.isNumber() && b.isNumber()) {
                    push(Value(a.asNumber() + b.asNumber()));
                } else if (a.isString() || b.isString()) {
                    push(Value(a.toString() + b.toString()));
                } else {
                    Value mm = getMetamethod(a, "__add");
                    if (mm.isNil()) mm = getMetamethod(b, "__add");
                    if (mm.isNativeFn()) {
                        Value args[2] = {a, b};
                        push(mm.asNativeFn()(2, args));
                    } else {
                        push(Value(0.0));
                    }
                }
                break;
            }

            case OpCode::OP_SUBTRACT: {
                Value b = pop();
                Value a = pop();
                if (a.isNumber() && b.isNumber()) {
                    push(Value(a.asNumber() - b.asNumber()));
                } else {
                    Value mm = getMetamethod(a, "__sub");
                    if (mm.isNil()) mm = getMetamethod(b, "__sub");
                    if (mm.isNativeFn()) {
                        Value args[2] = {a, b};
                        push(mm.asNativeFn()(2, args));
                    } else {
                        push(Value(0.0));
                    }
                }
                break;
            }

            case OpCode::OP_MULTIPLY: {
                Value b = pop();
                Value a = pop();
                if (a.isNumber() && b.isNumber()) {
                    push(Value(a.asNumber() * b.asNumber()));
                } else {
                    Value mm = getMetamethod(a, "__mul");
                    if (mm.isNil()) mm = getMetamethod(b, "__mul");
                    if (mm.isNativeFn()) {
                        Value args[2] = {a, b};
                        push(mm.asNativeFn()(2, args));
                    } else {
                        push(Value(0.0));
                    }
                }
                break;
            }

            case OpCode::OP_DIVIDE: {
                Value b = pop();
                Value a = pop();
                if (a.isNumber() && b.isNumber()) {
                    if (b.asNumber() == 0) {
                        std::cerr << "[Runtime Error] Division by zero." << std::endl;
                        return InterpretResult::INTERPRET_RUNTIME_ERROR;
                    }
                    push(Value(a.asNumber() / b.asNumber()));
                } else {
                    Value mm = getMetamethod(a, "__div");
                    if (mm.isNil()) mm = getMetamethod(b, "__div");
                    if (mm.isNativeFn()) {
                        Value args[2] = {a, b};
                        push(mm.asNativeFn()(2, args));
                    } else {
                        push(Value(0.0));
                    }
                }
                break;
            }

            case OpCode::OP_MODULO: {
                Value b = pop();
                Value a = pop();
                push(Value(static_cast<double>(static_cast<long long>(a.asNumber()) % static_cast<long long>(b.asNumber()))));
                break;
            }

            case OpCode::OP_NOT: {
                push(Value(!pop().isTruthy()));
                break;
            }

            case OpCode::OP_NEGATE: {
                if (!peek(0).isNumber()) {
                    std::cerr << "[Runtime Error] Operand must be a number." << std::endl;
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                push(Value(-pop().asNumber()));
                break;
            }

            case OpCode::OP_JUMP: {
                uint16_t offset = READ_SHORT();
                frame->ip += offset;
                break;
            }

            case OpCode::OP_JUMP_IF_FALSE: {
                uint16_t offset = READ_SHORT();
                if (!peek(0).isTruthy()) {
                    frame->ip += offset;
                }
                break;
            }

            case OpCode::OP_LOOP: {
                uint16_t offset = READ_SHORT();
                frame->ip -= offset;
                break;
            }

            case OpCode::OP_CALL: {
                uint8_t argCount = READ_BYTE();
                Value callee = peek(argCount);

                if (callee.isNativeFn()) {
                    std::vector<Value> args(argCount);
                    for (int i = argCount - 1; i >= 0; --i) {
                        args[i] = pop();
                    }
                    pop(); // pop native function
                    Value result = callee.asNativeFn()(argCount, args.data());
                    push(result);
                } else if (callee.isFunction()) {
                    auto fnObj = callee.asFunction();
                    if (argCount != fnObj->arity) {
                        std::cerr << "[Runtime Error] Expected " << fnObj->arity << " arguments but got " << argCount << "." << std::endl;
                        return InterpretResult::INTERPRET_RUNTIME_ERROR;
                    }

                    // Look up latest function implementation from global symbol table (HOT RELOAD MAGIC!)
                    Value latestFnVal;
                    if (env_.getGlobal(fnObj->name, latestFnVal) && latestFnVal.isFunction()) {
                        fnObj = latestFnVal.asFunction();
                    }

                    // Create new CallFrame
                    CallFrame newFrame;
                    newFrame.function = fnObj;
                    newFrame.chunk = fnObj->chunk.get();
                    newFrame.ip = 0;
                    newFrame.stackOffset = stack_.size() - argCount;
                    frames_.push_back(newFrame);
                    frame = &frames_.back();
                } else {
                    std::cerr << "[Runtime Error] Can only call functions." << std::endl;
                    return InterpretResult::INTERPRET_RUNTIME_ERROR;
                }
                break;
            }

            case OpCode::OP_RETURN: {
                Value result = pop();
                size_t offset = frame->stackOffset;
                frames_.pop_back();

                // Unwind arguments and function object from stack
                while (stack_.size() > offset && !stack_.empty()) {
                    stack_.pop_back();
                }
                if (!stack_.empty()) {
                    stack_.pop_back(); // pop function callee
                }

                if (frames_.empty()) {
                    push(result);
                    return InterpretResult::INTERPRET_OK;
                }
                frame = &frames_.back();
                push(result);
                break;
            }
        }
    }

    #undef READ_BYTE
    #undef READ_CONSTANT
    #undef READ_SHORT
}

} // namespace srl
