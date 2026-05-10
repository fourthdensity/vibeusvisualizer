# Visualization Backend Abstraction Layer

## Overview

Vibeus now features a **pluggable visualization backend system** through the `IVisualizer` interface. This abstraction layer allows you to:

- **Swap visualization engines** without touching app logic (audio capture, UI, controls, config, etc.)
- **Remove LGPL dependencies** by using the custom Vibeus backend
- **Full creative control** over visualization effects and preset systems
- **Easy to ship** - no external library dependencies required (with Vibeus backend)
- **Modern effects** - add GPU compute, ML-based audio analysis, custom shaders, etc.

## Architecture

The abstraction follows the **Dependency Inversion Principle**:

```
┌─────────────────────────────────────────────┐
│     Application Layer (main.cpp, UI)       │
│  (audio capture, controls, config, etc.)   │
└─────────────────────────────────────────────┘
                    ↓ depends on
┌─────────────────────────────────────────────┐
│         IVisualizer Interface               │
│    (abstract visualization contract)        │
└─────────────────────────────────────────────┘
              ↑ implemented by
    ┌─────────┴─────────┐
    │                   │
┌───┴───────┐   ┌───────┴────────┐
│ ProjectM  │   │  Vibeus Custom │
│  Backend  │   │    Backend     │
└───────────┘   └────────────────┘
```

### Key Files

- **`src/IVisualizer.h`** - Abstract interface defining the visualization API
- **`src/ProjectMVisualizer.h/cpp`** - ProjectM backend (LGPL, wraps projectM library)
- **`src/VibeusVisualizer.h/cpp`** - Custom Vibeus backend (no external dependencies)

## Building with Different Backends

### Option 1: ProjectM Backend (Default)

Use the existing ProjectM library (requires projectM 4.x):

```bash
cmake -B build -DUSE_PROJECTM=ON -DUSE_VIBEUS_BACKEND=OFF
cmake --build build --config Release
```

This is the **default configuration** and maintains full compatibility with existing features.

### Option 2: Vibeus Custom Backend

Use the custom visualization engine (no ProjectM dependency):

```bash
cmake -B build -DUSE_PROJECTM=OFF -DUSE_VIBEUS_BACKEND=ON
cmake --build build --config Release
```

This builds **without any LGPL dependencies** and uses the custom renderer.

### Option 3: Both Backends (Development)

Build both backends for testing and comparison:

```bash
cmake -B build -DUSE_PROJECTM=ON -DUSE_VIBEUS_BACKEND=ON
cmake --build build --config Release
```

You can then switch backends via compile-time flags or runtime selection (requires additional app logic).

## IVisualizer Interface

The interface provides a complete abstraction over:

### Lifecycle
- `initialize(width, height)` - Initialize the backend
- `shutdown()` - Clean up resources

### Rendering
- `setWindowSize(width, height)` - Update viewport
- `setFrameTime(seconds)` - Set animation time
- `renderFrame()` - Render current frame to OpenGL backbuffer

### Audio Input
- `addAudioPCM(samples, numFrames)` - Feed stereo PCM audio data

### Preset Management
- `loadPresetsFromDirectory(path)` - Scan and load presets
- `getPresetCount()`, `setPreset(index, hardCut)`, etc.

### Parameters
- Beat sensitivity, preset duration, transitions, mesh size, etc.
- All existing Vibeus settings map cleanly to interface methods

### Interactive Features
- `touch(x, y, pressure, type)` - Interactive waveforms
- `touchDrag(x, y, pressure)` - Drag interactions

## Migration Status

### ✅ Completed

1. **Abstraction interface** - `IVisualizer.h` defines complete API
2. **ProjectM backend** - Full wrapper around projectM library
3. **Vibeus custom backend** - Basic placeholder implementation
4. **Build system** - CMake with backend selection options

### 🚧 In Progress

1. **Update main.cpp** - Replace `projectm_handle` with `IVisualizer*`
2. **Update AudioCapture** - Use interface instead of direct projectM calls
3. **Update PresetManager** - Abstract playlist operations
4. **Update Storyteller** - Work through interface
5. **Testing** - Verify both backends work correctly

### 🎯 Future Enhancements

1. **Vibeus Backend Features**
   - Custom shader system (GLSL-based)
   - Modern GPU compute for audio analysis
   - JSON/YAML preset format
   - ML-based feature extraction
   - Advanced state machines

2. **Runtime Backend Selection**
   - Config option to choose backend at startup
   - Hot-swapping between backends

3. **Additional Backends**
   - WebGPU backend for web deployment
   - Vulkan backend for max performance
   - ASCII art backend for terminal mode 😎

## Development Guide

### Adding a New Backend

1. Create `MyBackend.h` and inherit from `IVisualizer`:
   ```cpp
   class MyBackend : public IVisualizer {
   public:
       bool initialize(int width, int height) override { /* ... */ }
       void renderFrame() override { /* ... */ }
       // ... implement all interface methods
   };
   ```

2. Add implementation in `MyBackend.cpp`

3. Update `CMakeLists.txt`:
   ```cmake
   option(USE_MY_BACKEND "Use My Backend" OFF)
   if(USE_MY_BACKEND)
       list(APPEND SOURCES src/MyBackend.cpp)
       list(APPEND HEADERS src/MyBackend.h)
       target_compile_definitions(${PROJECT_NAME} PRIVATE USE_MY_BACKEND)
   endif()
   ```

4. Update app initialization code to instantiate your backend

### Testing Your Backend

1. Build with your backend enabled
2. Run the app - all existing controls should work
3. Verify audio reactivity, preset switching, parameters
4. Check for any missing functionality in your implementation

## Benefits of This Approach

### For Users
- **Easier to ship** - No complex library dependencies (with Vibeus backend)
- **Better performance** - Custom backends can be optimized for specific use cases
- **Modern effects** - Not limited by projectM's architecture

### For Developers
- **Clean codebase** - App logic decoupled from visualization engine
- **Easy testing** - Swap backends to compare behavior
- **Future-proof** - Add new backends without breaking existing code
- **No LGPL concerns** - Custom backend is fully permissive (MIT/BSD/Apache)

## Next Steps

1. **Complete migration** - Update all app code to use `IVisualizer` interface
2. **Enhance Vibeus backend** - Add proper shader system and effects
3. **Performance tuning** - Optimize both backends
4. **Documentation** - Add preset authoring guide for custom backend
5. **Testing** - Comprehensive test suite for both backends

## License Notes

- **ProjectM Backend**: LGPL-2.1 (inherited from projectM library)
- **Vibeus Custom Backend**: MIT/BSD/Apache (your choice, no external deps)
- **Abstraction Layer**: Same license as Vibeus (your choice)

The abstraction layer allows you to ship without LGPL dependencies by using only the custom backend.

## Questions?

- Check the interface definition in `src/IVisualizer.h`
- Review the ProjectM backend for a complete reference implementation
- Look at the Vibeus backend for a minimal starting point
- Open an issue for architecture questions or suggestions
