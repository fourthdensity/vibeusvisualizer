#pragma once

#include <string>
#include <cstdint>

/// Abstract interface for visualization backends.
/// Provides a clean abstraction layer between the Vibeus application logic
/// and the underlying visualization engine (ProjectM, custom renderer, etc.).
///
/// This follows the Dependency Inversion Principle:
/// - High-level app code depends on this interface (not concrete implementations)
/// - Multiple backends can be swapped without changing app logic
/// - Each backend implements this contract independently
///
/// Key design goals:
/// - Keep all existing app logic unchanged (audio capture, ImGui, presets, controls)
/// - Allow swapping backends at compile-time or runtime
/// - Enable full creative control over visualization engine
/// - Remove LGPL dependency concerns
class IVisualizer {
public:
    virtual ~IVisualizer() = default;

    // ----- Initialization & Lifecycle -----

    /// Initialize the visualizer with the given window dimensions.
    /// Returns true on success.
    virtual bool initialize(int width, int height) = 0;

    /// Clean up resources. Called before destruction.
    virtual void shutdown() = 0;

    // ----- Window & Display -----

    /// Update the rendering viewport size.
    virtual void setWindowSize(int width, int height) = 0;

    /// Set the current frame time (for animation playback).
    /// Time is in seconds, typically accumulated delta time.
    virtual void setFrameTime(double seconds) = 0;

    // ----- Audio Input -----

    /// Feed stereo audio PCM data to the visualizer.
    /// @param samples  Interleaved stereo float samples (L,R,L,R,...)
    /// @param numFrames  Number of stereo frames (samples.size() / 2)
    virtual void addAudioPCM(const float* samples, uint32_t numFrames) = 0;

    // ----- Rendering -----

    /// Render the current visualization frame to the OpenGL backbuffer.
    /// Assumes an active OpenGL context.
    virtual void renderFrame() = 0;

    // ----- Preset Management -----

    /// Load presets from a directory (recursive scan for .milk files).
    /// Returns the number of presets loaded.
    virtual uint32_t loadPresetsFromDirectory(const std::string& path) = 0;

    /// Get the total number of loaded presets.
    virtual uint32_t getPresetCount() const = 0;

    /// Set the current preset by index.
    /// @param index  Preset index (0-based)
    /// @param hardCut  If true, immediately jump to preset (no transition)
    virtual void setPreset(uint32_t index, bool hardCut = false) = 0;

    /// Get the current preset index.
    virtual uint32_t getCurrentPresetIndex() const = 0;

    /// Get the name/path of the preset at the given index.
    virtual std::string getPresetName(uint32_t index) const = 0;

    /// Remove a preset from the playlist by index.
    virtual void removePreset(uint32_t index) = 0;

    // ----- Visualization Parameters -----

    /// Set the beat sensitivity (0.0 = no reaction, 5.0 = extreme).
    virtual void setBeatSensitivity(float sensitivity) = 0;

    /// Get the current beat sensitivity.
    virtual float getBeatSensitivity() const = 0;

    /// Set the preset duration in seconds (for auto-advance).
    virtual void setPresetDuration(double seconds) = 0;

    /// Set the soft-cut transition duration in seconds.
    virtual void setSoftCutDuration(double seconds) = 0;

    /// Get the current soft-cut duration.
    virtual double getSoftCutDuration() const = 0;

    /// Enable or disable hard cuts (beat-reactive preset changes).
    virtual void setHardCutEnabled(bool enabled) = 0;

    /// Set hard cut sensitivity (how reactive to beats).
    virtual void setHardCutSensitivity(float sensitivity) = 0;

    /// Set hard cut duration (time between hard cuts).
    virtual void setHardCutDuration(double seconds) = 0;

    /// Lock the current preset (disable auto-advance).
    virtual void setPresetLocked(bool locked) = 0;

    /// Set the mesh resolution for per-pixel effects.
    /// @param width  Mesh width in vertices
    /// @param height  Mesh height in vertices
    virtual void setMeshSize(int width, int height) = 0;

    /// Enable or disable aspect ratio correction.
    virtual void setAspectCorrection(bool enabled) = 0;

    /// Enable or disable "easter egg" mode (preset variety/randomization).
    virtual void setEasterEgg(float value) = 0;

    /// Set texture search paths for preset textures.
    virtual void setTextureSearchPaths(const char** paths, size_t count) = 0;

    // ----- Interactive Features -----

    /// Simulate a touch/mouse interaction at normalized coordinates.
    /// @param x  Normalized X coordinate (0.0 = left, 1.0 = right)
    /// @param y  Normalized Y coordinate (0.0 = top, 1.0 = bottom)
    /// @param pressure  Pressure level (0 = light, 1 = medium, 2 = heavy)
    /// @param type  Touch waveform type (implementation-specific)
    virtual void touch(float x, float y, int pressure, int type) = 0;

    /// Drag a touch waveform (continuous interaction).
    virtual void touchDrag(float x, float y, int pressure) = 0;

    // ----- Information & Debugging -----

    /// Get the backend name/version string.
    virtual std::string getBackendInfo() const = 0;

    /// Set the log level for debug output.
    /// @param level  0=off, 1=error, 2=warn, 3=info, 4=debug, 5=trace
    virtual void setLogLevel(int level) = 0;
};
