# Bolero

C++17 OpenGL 4.6 Core forward PBR renderer. A spiritual successor to [PeanutCracker](https://github.com/KaindraDjoemena/PeanutCracker)

---

## Tech Stack

| Component | Version / Standard |
|-----------|-------------------|
| **C++** | C++17 |
| **C** | C11 |
| **OpenGL** | 4.6 Core Profile |
| **CMake** | 3.20+ |

## Dependencies

| Library | Version | Source |
|---------|---------|--------|
| **GLFW** | 3.4 | FetchContent (auto-download) |
| **GLM** | 1.0.3 | FetchContent (auto-download) |
| **GLAD** | 4.6 Core | Included in repo (`extern/glad/`) |
| **stb_image** | Latest | Included in repo (`extern/stb/`) |

---

## Building

### Prerequisites

- **CMake 3.20 or newer**
- **C++17 compatible compiler**:
  - MSVC 2019+ (Windows)
  - GCC 9+ (Linux)
  - Clang 10+ (macOS / Linux)
- **Git** (for FetchContent dependencies)

### Configure & Build

```bash
# Clone
git clone https://github.com/KaindraDjoemena/Bolero.git
cd Bolero

# Configure
cmake -B build -S .

# Build
cmake --build build --config Release

# Run
./build/bin/Release/Bolero.exe      # Windows
./build/bin/Bolero                  # Linux / macOS
```

### IDE Support

| IDE | Setup |
|-----|-------|
| **Visual Studio** | Open folder directly (CMake support built-in) or `cmake -B build -G "Visual Studio 17 2022"` |
| **VS Code** | Install CMake Tools extension, open folder |
| **CLion** | Open folder, CMakeLists.txt auto-detected |

---

## Platform Notes

| Platform | Specifics |
|----------|-----------|
| **Windows** | Links `opengl32.lib` automatically. MSVC recommended. |
| **Linux** | Requires OpenGL development packages (`libgl1-mesa-dev` on Debian/Ubuntu). |
| **macOS** | Links Cocoa, IOKit, CoreVideo frameworks. Note: Apple deprecates OpenGL; consider MoltenVK path for future. |

---

## Architecture Goals

- **Render Graph**: Explicit dependency graph for passes and resources
- **PBR Pipeline**: Metallic-roughness workflow with IBL
- **Documentation-First**: Every subsystem documented in code and commits