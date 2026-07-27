# SRL API Reference Documentation

Generated automatically by `srl doc` on Jul 27 2026

## Core Modules & API Specification

### 1. Standard Mathematics (`std/math.srl`)
- `vec2(x, y)` - Creates 2D vector object
- `vec3(x, y, z)` - Creates 3D vector object
- `clamp(val, min, max)` - Clamps value within specified boundaries
- `lerp(a, b, t)` - Performs linear interpolation

### 2. Desktop Qt GUI Framework (`std/qt.srl`)
- `qt_app_init()` - Initializes Qt application context
- `qt_window(title, width, height)` - Creates native QMainWindow
- `qt_button(parent, text, callback)` - Creates QPushButton with signal binding
- `qt_exec()` - Enters Qt main event loop

### 3. Data Structures & Collections (`std/collections.srl`)
- `set_new()`, `set_add()`, `set_has()` - Unique set collection
- `queue_new()`, `queue_push()`, `queue_pop()` - FIFO queue structure
- `stack_new()`, `stack_push()`, `stack_pop()` - LIFO stack structure
- `ringbuffer_new()`, `ringbuffer_write()` - Circular audio buffer
- `map_keys(map)` - Returns array of keys for map iteration
- `is_map(val)` - Returns true if value is a map
- `is_array(val)` - Returns true if value is an array

### 4. Concurrency & Synchronization (`std/sync.srl`)
- `mutex_create()`, `mutex_lock()`, `mutex_unlock()` - Thread mutex lock
- `channel_create()`, `channel_send()`, `channel_recv()` - Thread-safe channel
- `atomic_create()`, `atomic_add()`, `atomic_load()` - Atomic primitives
