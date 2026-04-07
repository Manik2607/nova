# NovaGFX Projects Guide

This guide explains how to compile and run the various examples and projects available in the NovaGFX repository natively on your Windows machine using CMake and MSVC.

## Prerequisites

- **CMake** (Must be reachable via Command Prompt/PowerShell)
- **Visual Studio 2022 Community** (or other MSVC compilers supporting C++17/C++20)
- **PowerShell**

---

## 1. Generating Build Files

Before compiling any specific target, you must generate the build directory using CMake. Open PowerShell in the root directory (`NovaGFX`) and run:

```powershell
cmake -B build
```

This will automatically fetch/build any dependencies (like Box2D, GLAD, GLFW, etc.) into the `build` folder using your default compiler.

---

## 2. Compiling an Example

To compile an individual example, use the `--target` flag so you don't rebuild the entire workspace every time. 

```powershell
cmake --build build --target <project_name> --config Release
```

Or using the direct path to CMake:
```powershell
& "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --build build --target <project_name> --config Release
```

### Available Targets (Examples)
- `car_game`
- `neon_drift`
- `particle_showcase`
- `physics_demo`
- `ui_demo`
- `hello_world`

---

## 3. Running an Example

Executables are routed to their respective `Release` or `Debug` folders inside the `build/examples` directory.

To run an application (e.g., `car_game`), use:

```powershell
.\build\examples\car_game\Release\car_game.exe
```

### Quick Commands Cheat-Sheet:

**Car Game:**
```powershell
cmake --build build --target car_game --config Release
.\build\examples\car_game\Release\car_game.exe
```

**Neon Drift:**
```powershell
cmake --build build --target neon_drift --config Release
.\build\examples\neon_drift\Release\neon_drift.exe
```

**Particle Showcase:**
```powershell
cmake --build build --target particle_showcase --config Release
.\build\examples\particle_showcase\Release\particle_showcase.exe
```

**UI Demo:**
```powershell
cmake --build build --target ui_demo --config Release
.\build\examples\ui_demo\Release\ui_demo.exe
```
