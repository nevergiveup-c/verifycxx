# verifycxx

## Description
Header-only runtime-time data verification library for C++20 and later.

## Reflections
I came up with the idea for developing a data-verification library while I was working on a small test assignment. I needed to reverse a simple binary and locate its C2. Without getting into unnecessary details, the thing that really caught my attention was the idea of data modification performed by an overridden C procedure - the memcpy function. When certain conditions were met, this function replaced specific bytes in the program, which resulted in different data being produced for different clients. I found the concept quite interesting and decided to build a meta-wrapper that would make it easy to validate, read, modify, and move data.

I outlined a set of criteria my library should meet:
1. The object must make it easy and fast to check whether the data has a wrapper.
2. It must support data manipulation and integrity checks in a multithreaded environment.
3. The library should be usable without the CRT or within a WDM environment.
4. It must compile with GCC/Clang/MSVC while providing the same level of data protection across all of them.

I want to clarify here: before and after implementing this project I didn’t research whether similar solutions already existed. I’m more than sure that there are implementations out there that are much “cooler and better” than what I built. My solution may also have several drawbacks, like: a weak key, a simplistic checksum, and overall vulnerability to basic dynamic debugging. Without additional measures, this project is not quite strong as a protection mechanism. Still, I believe that if used correctly, it can make an attacker’s job harder or help detect their presence in the process.

Everything above is to emphasize that this is still an unfinished PET-project with its own pros and cons, which I’ll be refining over time. Before using it for your own purposes, you should evaluate and test the library yourself to understand where and how it can be applied most effectively.

## How it works
The `verifycxx` object consists of 4 components: a header, the data, a checksum and a synchronization primitive (mutex or spinlock, depending on the environment).

### Structure
**verifycxx_header** — a bitmask, containing 3 fields:
- `magic` — a static object identifier(0x5644482F; "VDH/")
- `size` — the size of the date in bytes
- `cookie` — an encryption key, generated as `(this & 0xFFFF)`

**storage** — the user’s data in plaintext
**checksum** — the checksum, calculated using XOR with cookie и SAD (Sum of Absolute Differences)
**mutex** — a synchronization primitive (mutex or spinlock)

### Thread-safe API
The library provides 3 thread-safe methods for working with data:

**`modify()`** — returns RAII-wrapper, `verifycxx_modify_guard`, which:
- Provides a pointer to the data for modification
- Overloads operators for convenient access
- Automatically recalculates `checksum` when leaving the scope

**`get()`** — provides direct read-only access to data

**`verify()`** — compares the stored `checksum` with the result of `gen_checksum()` and returns the integrity-check result

# Benchmarks
| Operation | MSVC | LLVM | GCC |
|-----------|------|------|-----|
| ValueModify | 16.9 ns | **15.8 ns** ✓ | 67.2 ns (4.3x) |
| ValueVerify | 16.0 ns | **14.3 ns** ✓ | 80.0 ns (5.6x) |
| ChecksumRecalc | 1.34 ns | 0.241 ns | **0.118 ns** ✓ |
| ArrayModify | 22.9 ns | **18.4 ns** ✓ | 65.7 ns (3.6x) |
| ArrayVerify | 17.0 ns | **16.2 ns** ✓ | 73.6 ns (4.5x) |
| StringModify | 26.9 ns | **22.9 ns** ✓ | 72.4 ns (3.2x) |
| StringVerify | 18.2 ns | **13.5 ns** ✓ | 74.6 ns (5.5x) |

**Test Environment:**
- CPU: 16 cores @ 2496 MHz
- L1 Data Cache: 48 KiB (x8)
- L1 Instruction Cache: 32 KiB (x8)
- L2 Unified Cache: 512 KiB (x8)
- L3 Unified Cache: 16384 KiB (x1)
- Date: 2025-11-04

## Usage
This is a very simple example; in real projects, checks should be placed in more unpredictable locations. And remember, the library is
not just about such a basic form of verification.

```cpp
struct Vector3 {
    float pos[3]{};
};

struct Entity {
    float health{ 100.f }, armour{ 100.f };
    Vector3 position{};
};

struct Player : Entity {
    Player(const int uid, std::string n, const int l) :
        uniqueId(uid), name(std::move(n)), level(l) {};
    ~Player() = default;

    int uniqueId{};
    std::string name{};
    int level{};
};

int main(int argc, char** argv) {

    verifycxx<Player> value{ hash("nevergiveup-c"), "nevergiveup-c", 1 };

    std::thread workflow_t([&]() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            auto const val = value.modify()->level++;
            printf("Level up: %i\n", val);
        }
     });
    
     workflow_t.detach();
    
     std::thread verification_t([&]() {
        while (true) {
            if (!value.verify()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                printf("Warning: memory is corrupted\n");
            }
        }
     });

     verification_t.detach();
}
```

## Requirements:
- C++20 or later
- Compiler with SIMD support (AVX/SSE/NEON)
- CMake 3.15+ (for building tests)
- vcpkg (for dependencies)

## Building Tests and Benchmarks:
1. Install `vcpkg` and set `VCPKG_ROOT` environment variable
2. Fetch baseline: `cd $VCPKG_ROOT && git fetch origin 34823ada10080ddca99b60e85f80f55e18a44eea`
3. Configure: `cmake --preset <compiler>` (msvc/llvm/gcc)
4. Build: `cmake --build --preset <compiler>` (--config Release)

## Compiler Support:
- `MSVC (+WDM)`
- `CLANG`
- `GCC`

## Architecture Support:
- `x86-64`
- `ARM` (not tested yet)