#include "VibeusVisualizer.h"
#include <SDL_opengl.h>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <vector>
#include <cstdlib>

// Simple FFT for spectrum (radix-2, power of 2 size)
static void simpleFFT(const float* input, float* output, int n) {
    std::vector<float> real(n), imag(n);
    for (int i = 0; i < n; ++i) {
        real[i] = input[i * 2];
        imag[i] = 0.0f;
    }
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j >= bit; bit >>= 1) j -= bit;
        j += bit;
        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }
    for (int len = 2; len <= n; len <<= 1) {
        float ang = 2 * 3.14159265f / len;
        float wreal = cosf(ang), wimag = -sinf(ang);
        for (int i = 0; i < n; i += len) {
            float wr = 1.0f, wi = 0.0f;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j, v = i + j + len / 2;
                float tr = wr * real[v] - wi * imag[v];
                float ti = wr * imag[v] + wi * real[v];
                real[v] = real[u] - tr;
                imag[v] = imag[u] - ti;
                real[u] += tr;
                imag[u] += ti;
                float temp = wr;
                wr = wr * wreal - wi * wimag;
                wi = temp * wimag + wi * wreal;
            }
        }
    }
    for (int i = 0; i < n / 2; ++i) {
        output[i] = sqrtf(real[i] * real[i] + imag[i] * imag[i]) / n;
    }
}

// GLSL Shader code for future modernization (v0.5.0+)
// Compile in initialize() and use for particle glow / post-process bloom
// Example: Use with VBO + glDrawArrays for modern core profile compatibility
static const char* kGlowVertexShader = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in float aSize;
out vec4 vColor;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    gl_PointSize = aSize;
    vColor = aColor;
}
)";

static const char* kGlowFragmentShader = R"(
#version 330 core
in vec4 vColor;
out vec4 FragColor;
void main() {
    // Radial glow for particles
    vec2 coord = gl_PointCoord - vec2(0.5);
    float dist = length(coord);
    float alpha = vColor.a * (1.0 - smoothstep(0.0, 0.5, dist));
    FragColor = vec4(vColor.rgb, alpha);
}
)";

VibeusVisualizer::~VibeusVisualizer() {
    shutdown();
}

bool VibeusVisualizer::initialize(int width, int height) {
    m_width = width;
    m_height = height;
    m_audioBuffer.reserve(4096);
    m_spectrum.resize(64);
    m_particles.reserve(200);
    m_shockwaves.reserve(20);
    m_stars.resize(80);
    for (auto& s : m_stars) {
        s.x = (rand() % 2000 - 1000) / 1000.0f;
        s.y = (rand() % 2000 - 1000) / 1000.0f;
        s.twinkleSpeed = 0.5f + (rand() % 100) / 100.0f;
        s.brightness = 0.4f + (rand() % 40) / 100.0f;
    }
    m_effectTime = 0.0f;
    m_beatCount = 0;
    fprintf(stderr, "[VibeusVisualizer] Custom backend initialized (%dx%d) v0.5.0\n", width, height);
    return true;
}

void VibeusVisualizer::shutdown() {
    m_presets.clear();
    m_audioBuffer.clear();
    m_spectrum.clear();
    m_particles.clear();
    m_shockwaves.clear();
    m_stars.clear();
}

void VibeusVisualizer::setWindowSize(int width, int height) {
    m_width = width;
    m_height = height;
}

void VibeusVisualizer::setFrameTime(double seconds) {
    m_frameTime = seconds;
}

void VibeusVisualizer::addAudioPCM(const float* samples, uint32_t numFrames) {
    if (!samples || numFrames == 0) return;

    float sumSq = 0.0f;
    for (uint32_t i = 0; i < numFrames * 2; i++) {
        float s = samples[i];
        sumSq += s * s;
    }
    m_audioLevel = sqrtf(sumSq / (numFrames * 2.0f));

    m_audioBuffer.assign(samples, samples + numFrames * 2);

    int fftSize = 512;
    if (m_audioBuffer.size() >= fftSize * 2) {
        std::vector<float> fftOut(fftSize / 2);
        simpleFFT(m_audioBuffer.data(), fftOut.data(), fftSize);
        for (int b = 0; b < 64; ++b) {
            float sum = 0.0f;
            int start = b * (fftSize / 2 / 64);
            int end = (b + 1) * (fftSize / 2 / 64);
            for (int k = start; k < end; ++k) sum += fftOut[k];
            m_spectrum[b] = sum / (end - start) * 5.0f;
            m_spectrum[b] = std::min(1.0f, m_spectrum[b]);
        }
    }

    // Enhanced beat detection & mode switching
    static float lastLevel = 0.0f;
    bool isStrongBeat = (m_audioLevel > lastLevel * 1.55f && m_audioLevel > 0.32f);
    if (isStrongBeat) {
        m_beatCount++;
        m_lastBeatIntensity = m_audioLevel;
        if (m_beatCount % 5 == 0) {
            m_visualMode = (m_visualMode + 1) % 4;
            if (m_logLevel >= 3) {
                fprintf(stderr, "[VibeusVisualizer] Visual mode -> %d\n", m_visualMode);
            }
        }
        // Spawn shockwave rings
        for (int i = 0; i < 3 && m_shockwaves.size() < 18; ++i) {
            Shockwave sw;
            sw.x = (rand() % 600 - 300) / 1000.0f;
            sw.y = (rand() % 600 - 300) / 1000.0f;
            sw.radius = 0.08f + m_audioLevel * 0.05f;
            sw.alpha = 0.9f;
            sw.hue = static_cast<float>(rand() % 360) / 360.0f;
            m_shockwaves.push_back(sw);
        }
        // Extra particle burst on strong beats
        for (int i = 0; i < 15 && m_particles.size() < 220; ++i) {
            Particle p;
            p.x = (rand() % 2000 - 1000) / 1000.0f * 0.4f;
            p.y = (rand() % 2000 - 1000) / 1000.0f * 0.4f;
            p.vx = (rand() % 1000 - 500) / 250.0f;
            p.vy = (rand() % 1000 - 500) / 250.0f;
            p.life = 1.1f + (rand() % 30) / 100.0f;
            p.size = 0.02f + m_audioLevel * 0.06f;
            p.hue = static_cast<float>(rand() % 360) / 360.0f;
            m_particles.push_back(p);
        }
    }
    lastLevel = m_audioLevel * 0.85f + lastLevel * 0.15f;
}

void VibeusVisualizer::renderFrame() {
    float intensity = std::min(1.0f, m_audioLevel * 2.8f * m_beatSensitivity);
    m_effectTime += 0.016f;

    // Dynamic background with beat flash
    float flash = std::max(0.0f, (m_audioLevel - 0.55f) * 1.8f);
    glClearColor(0.025f + intensity * 0.035f + flash * 0.12f,
                 0.045f + intensity * 0.055f + flash * 0.06f,
                 0.11f + intensity * 0.075f + flash * 0.04f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Update shockwaves
    for (auto it = m_shockwaves.begin(); it != m_shockwaves.end(); ) {
        it->radius += 0.022f;
        it->alpha -= 0.028f;
        if (it->alpha <= 0.02f) {
            it = m_shockwaves.erase(it);
        } else {
            ++it;
        }
    }

    // Draw stars (twinkling background)
    glPointSize(1.8f);
    glBegin(GL_POINTS);
    for (const auto& s : m_stars) {
        float tw = 0.6f + 0.4f * sinf(m_effectTime * s.twinkleSpeed + s.x * 7.0f);
        float alpha = s.brightness * tw * (0.6f + intensity * 0.4f);
        glColor4f(0.75f, 0.85f, 1.0f, alpha);
        glVertex2f(s.x, s.y);
    }
    glEnd();

    // Mode-specific rendering
    switch (m_visualMode) {
    case 0: // CLASSIC - Enhanced particles + spectrum + core + network
        drawClassicMode(intensity, flash);
        break;
    case 1: // NEBULA - Soft volumetric clouds + aurora spectrum + floating orbs
        drawNebulaMode(intensity);
        break;
    case 2: // TUNNEL - Hyperspace tunnel with frequency walls + flying particles
        drawTunnelMode(intensity);
        break;
    case 3: // SYMMETRY - Kaleidoscopic mirrored effects + radial bursts
        drawSymmetryMode(intensity);
        break;
    default:
        drawClassicMode(intensity, flash);
    }

    // Draw shockwave rings (common to all modes)
    glLineWidth(2.2f);
    for (const auto& sw : m_shockwaves) {
        float r = 0.7f + sinf(sw.hue * 6.28f) * 0.3f;
        float g = 0.4f + cosf(sw.hue * 6.28f + 2.0f) * 0.4f;
        float b = 0.95f;
        glColor4f(r, g, b, sw.alpha * 0.9f);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 28; ++i) {
            float a = (i / 28.0f) * 2.0f * 3.14159265f;
            glVertex2f(sw.x + cosf(a) * sw.radius, sw.y + sinf(a) * sw.radius);
        }
        glEnd();
    }
    glLineWidth(1.0f);

    // Update & draw particles (enhanced physics, mode-aware attraction)
    std::vector<Particle> aliveParticles;
    glBegin(GL_LINES);
    glColor4f(0.5f, 0.75f, 1.0f, 0.22f);
    for (size_t i = 0; i < m_particles.size(); ++i) {
        for (size_t j = i + 1; j < m_particles.size(); ++j) {
            float dx = m_particles[i].x - m_particles[j].x;
            float dy = m_particles[i].y - m_particles[j].y;
            float dist = sqrtf(dx*dx + dy*dy);
            if (dist < 0.38f && dist > 0.008f) {
                glVertex2f(m_particles[i].x, m_particles[i].y);
                glVertex2f(m_particles[j].x, m_particles[j].y);
            }
        }
    }
    glEnd();

    glPointSize(4.0f);
    glBegin(GL_POINTS);
    for (auto& p : m_particles) {
        p.x += p.vx * 0.019f;
        p.y += p.vy * 0.019f;
        p.vx *= 0.982f;
        p.vy *= 0.982f;
        p.life -= 0.026f;

        // Mode-aware center attraction/repulsion
        float distToCenter = sqrtf(p.x*p.x + p.y*p.y);
        float attract = (m_visualMode == 2) ? -0.012f : 0.009f; // tunnel repels
        if (distToCenter > 0.04f) {
            p.vx -= p.x * attract;
            p.vy -= p.y * attract;
        }

        if (p.life > 0.0f) {
            float alpha = p.life * 0.88f;
            float r = 0.65f + sinf(p.hue * 6.28f) * 0.35f;
            float g = 0.55f + cosf(p.hue * 6.28f) * 0.45f;
            float b = 0.92f - sinf(p.hue * 6.28f) * 0.15f;
            glColor4f(r, g, b, alpha);
            glVertex2f(p.x, p.y);
            aliveParticles.push_back(p);
        }
    }
    glEnd();
    m_particles = std::move(aliveParticles);

    // Preset indicator - mode colored glowing pill
    if (!m_presets.empty()) {
        float hue = static_cast<float>(m_currentPreset) / std::max(1u, static_cast<uint32_t>(m_presets.size()));
        float modeShift = m_visualMode * 0.25f;
        float pr = 0.55f + sinf((hue + modeShift) * 6.28f) * 0.45f;
        float pg = 0.45f + cosf((hue + modeShift) * 6.28f + 1.2f) * 0.5f;
        float pb = 0.85f;
        glColor4f(pr, pg, pb, 0.6f);
        glBegin(GL_QUADS);
        glVertex2f(-0.96f, 0.86f);
        glVertex2f(-0.68f, 0.86f);
        glVertex2f(-0.68f, 0.96f);
        glVertex2f(-0.96f, 0.96f);
        glEnd();
        glColor4f(1.0f, 1.0f, 1.0f, 0.4f);
        glBegin(GL_QUADS);
        glVertex2f(-0.94f, 0.88f);
        glVertex2f(-0.70f, 0.88f);
        glVertex2f(-0.70f, 0.94f);
        glVertex2f(-0.94f, 0.94f);
        glEnd();
    }
}

// Mode drawing helpers (inlined for simplicity)
void VibeusVisualizer::drawClassicMode(float intensity, float flash) {
    // Central pulsing core (bass reactive orb)
    float coreSize = 0.09f + intensity * 0.13f;
    float coreAlpha = 0.65f + intensity * 0.35f;
    glColor4f(0.92f, 0.45f + intensity * 0.25f, 1.0f - intensity * 0.15f, coreAlpha);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0.0f, 0.0f);
    for (int i = 0; i <= 36; ++i) {
        float angle = (i / 36.0f) * 2.0f * 3.14159265f;
        glVertex2f(cosf(angle) * coreSize, sinf(angle) * coreSize);
    }
    glEnd();

    // Inner bright core
    glColor4f(1.0f, 0.92f, 0.75f, 0.92f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0.0f, 0.0f);
    for (int i = 0; i <= 36; ++i) {
        float angle = (i / 36.0f) * 2.0f * 3.14159265f;
        glVertex2f(cosf(angle) * coreSize * 0.42f, sinf(angle) * coreSize * 0.42f);
    }
    glEnd();

    // Spectrum bars with frequency-based colors
    float barWidth = 1.82f / 64.0f;
    glBegin(GL_QUADS);
    for (int b = 0; b < 64; ++b) {
        float h = m_spectrum[b] * 0.88f;
        float x = -0.91f + b * barWidth;
        float hue = (b / 64.0f) * 0.82f;
        float r = std::max(0.18f, sinf(hue * 3.14159f * 2.0f) * 0.82f + 0.28f);
        float g = std::max(0.18f, sinf((hue + 0.33f) * 3.14159f * 2.0f) * 0.82f + 0.28f);
        float bl = std::max(0.18f, sinf((hue + 0.66f) * 3.14159f * 2.0f) * 0.82f + 0.28f);
        glColor4f(r, g, bl, 0.88f);
        glVertex2f(x, -0.91f);
        glVertex2f(x + barWidth * 0.82f, -0.91f);
        glVertex2f(x + barWidth * 0.82f, -0.91f + h);
        glVertex2f(x, -0.91f + h);
    }
    glEnd();

    // Neon waveform
    if (!m_audioBuffer.empty()) {
        glLineWidth(2.8f);
        glColor4f(0.25f, 1.0f, 0.55f, 0.78f);
        glBegin(GL_LINE_STRIP);
        size_t skip = std::max<size_t>(1, m_audioBuffer.size() / 280);
        for (size_t i = 0; i < m_audioBuffer.size(); i += skip * 2) {
            float x = -0.95f + (1.9f * i) / m_audioBuffer.size();
            float y = m_audioBuffer[i] * 0.58f;
            glVertex2f(x, y);
        }
        glEnd();
        glLineWidth(1.0f);
    }
}

void VibeusVisualizer::drawNebulaMode(float intensity) {
    // Soft nebula clouds (many overlapping transparent quads)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (int c = 0; c < 22; ++c) {
        float cx = sinf(m_effectTime * 0.3f + c) * 0.6f;
        float cy = cosf(m_effectTime * 0.22f + c * 1.3f) * 0.55f;
        float cs = 0.25f + sinf(m_effectTime * 0.4f + c) * 0.12f;
        float ch = (c % 64) / 64.0f;
        float cr = 0.4f + sinf(ch * 6.28f) * 0.4f;
        float cg = 0.3f + cosf(ch * 6.28f + 1.5f) * 0.5f;
        float cb = 0.7f + sinf(ch * 6.28f + 3.0f) * 0.3f;
        glColor4f(cr, cg, cb, 0.12f + intensity * 0.08f);
        glBegin(GL_QUADS);
        glVertex2f(cx - cs, cy - cs * 0.7f);
        glVertex2f(cx + cs, cy - cs * 0.7f);
        glVertex2f(cx + cs, cy + cs * 0.7f);
        glVertex2f(cx - cs, cy + cs * 0.7f);
        glEnd();
    }

    // Aurora spectrum arcs
    glLineWidth(3.5f);
    for (int a = 0; a < 5; ++a) {
        float ay = -0.6f + a * 0.28f;
        glColor4f(0.3f + m_spectrum[a*12] * 0.6f, 0.6f + m_spectrum[a*12+6] * 0.5f, 0.9f, 0.35f + intensity * 0.2f);
        glBegin(GL_LINE_STRIP);
        for (int i = 0; i < 48; ++i) {
            float px = -0.95f + (i / 47.0f) * 1.9f;
            float py = ay + sinf(px * 3.5f + m_effectTime + a) * (0.08f + m_spectrum[a*12] * 0.15f);
            glVertex2f(px, py);
        }
        glEnd();
    }
    glLineWidth(1.0f);

    // Floating bright orbs (bass reactive)
    for (int o = 0; o < 6; ++o) {
        float ox = sinf(m_effectTime * 0.15f + o * 1.7f) * 0.7f;
        float oy = cosf(m_effectTime * 0.18f + o) * 0.6f;
        float os = 0.04f + m_spectrum[o * 10] * 0.09f;
        glColor4f(0.95f, 0.85f, 1.0f, 0.5f + intensity * 0.3f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(ox, oy);
        for (int i = 0; i <= 20; ++i) {
            float ang = (i / 20.0f) * 6.283185f;
            glVertex2f(ox + cosf(ang) * os, oy + sinf(ang) * os);
        }
        glEnd();
    }
}

void VibeusVisualizer::drawTunnelMode(float intensity) {
    // Hyperspace tunnel - concentric rings receding
    for (int r = 0; r < 18; ++r) {
        float z = fmodf(m_effectTime * 1.8f + r * 0.11f, 1.4f);
        float radius = 0.15f + z * 1.6f;
        float alpha = 0.9f - z * 0.6f;
        int specIdx = (r * 3) % 64;
        float col = m_spectrum[specIdx] * 0.7f + 0.3f;
        glColor4f(0.2f + col * 0.5f, 0.5f + col * 0.4f, 0.95f - col * 0.3f, alpha);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 32; ++i) {
            float a = (i / 32.0f) * 6.283185f;
            glVertex2f(cosf(a) * radius, sinf(a) * radius);
        }
        glEnd();
    }

    // Radial speed lines (motion blur effect)
    glLineWidth(1.5f);
    glColor4f(0.6f, 0.8f, 1.0f, 0.25f + intensity * 0.15f);
    for (int l = 0; l < 24; ++l) {
        float la = (l / 24.0f) * 6.283185f + m_effectTime * 0.8f;
        glBegin(GL_LINES);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(cosf(la) * 1.8f, sinf(la) * 1.8f);
        glEnd();
    }
    glLineWidth(1.0f);
}

void VibeusVisualizer::drawSymmetryMode(float intensity) {
    // Kaleidoscope: draw core scene 8 times rotated
    for (int s = 0; s < 8; ++s) {
        float rot = (s / 8.0f) * 6.283185f + m_effectTime * 0.3f;
        glPushMatrix();
        glRotatef(rot * 57.2958f, 0.0f, 0.0f, 1.0f); // approx degrees

        // Mini core
        float cs = 0.06f + intensity * 0.08f;
        glColor4f(0.9f, 0.6f + intensity * 0.3f, 1.0f, 0.7f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(0.0f, 0.0f);
        for (int i = 0; i <= 20; ++i) {
            float a = (i / 20.0f) * 6.283185f;
            glVertex2f(cosf(a) * cs, sinf(a) * cs);
        }
        glEnd();

        // Spectrum slices
        float barW = 0.9f / 32.0f;
        glBegin(GL_QUADS);
        for (int b = 0; b < 32; ++b) {
            float h = m_spectrum[b * 2] * 0.7f;
            float x = -0.45f + b * barW;
            glColor4f(0.4f + m_spectrum[b*2] * 0.5f, 0.7f, 0.9f, 0.65f);
            glVertex2f(x, 0.0f);
            glVertex2f(x + barW * 0.9f, 0.0f);
            glVertex2f(x + barW * 0.9f, h);
            glVertex2f(x, h);
        }
        glEnd();

        glPopMatrix();
    }

    // Central radial burst lines
    glLineWidth(2.0f);
    glColor4f(1.0f, 0.9f, 0.6f, 0.4f + intensity * 0.3f);
    for (int l = 0; l < 16; ++l) {
        float la = (l / 16.0f) * 6.283185f + m_effectTime * 2.0f;
        glBegin(GL_LINES);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(cosf(la) * (0.9f + intensity * 0.3f), sinf(la) * (0.9f + intensity * 0.3f));
        glEnd();
    }
    glLineWidth(1.0f);
}

uint32_t VibeusVisualizer::loadPresetsFromDirectory(const std::string& path) {
    uint32_t before = static_cast<uint32_t>(m_presets.size());
    for (int i = 0; i < 25; i++) {
        m_presets.push_back(path + "/VibeusPreset" + std::to_string(i + 1) + ".vbs");
    }
    uint32_t after = static_cast<uint32_t>(m_presets.size());
    fprintf(stderr, "[VibeusVisualizer] Loaded %u placeholder presets from %s\n", after - before, path.c_str());
    return after - before;
}

uint32_t VibeusVisualizer::getPresetCount() const {
    return static_cast<uint32_t>(m_presets.size());
}

void VibeusVisualizer::setPreset(uint32_t index, bool hardCut) {
    if (index < m_presets.size()) {
        m_currentPreset = index;
        if (m_logLevel >= 3) {
            fprintf(stderr, "[VibeusVisualizer] Switched to preset %u (%s)\n", index, hardCut ? "hard cut" : "soft cut");
        }
    }
}

uint32_t VibeusVisualizer::getCurrentPresetIndex() const {
    return m_currentPreset;
}

std::string VibeusVisualizer::getPresetName(uint32_t index) const {
    if (index < m_presets.size()) return m_presets[index];
    return "";
}

void VibeusVisualizer::removePreset(uint32_t index) {
    if (index < m_presets.size()) {
        m_presets.erase(m_presets.begin() + index);
        if (m_currentPreset >= m_presets.size() && !m_presets.empty())
            m_currentPreset = static_cast<uint32_t>(m_presets.size() - 1);
    }
}

void VibeusVisualizer::setBeatSensitivity(float sensitivity) { m_beatSensitivity = sensitivity; }
float VibeusVisualizer::getBeatSensitivity() const { return m_beatSensitivity; }
void VibeusVisualizer::setPresetDuration(double seconds) { m_presetDuration = seconds; }
void VibeusVisualizer::setSoftCutDuration(double seconds) { m_softCutDuration = seconds; }
double VibeusVisualizer::getSoftCutDuration() const { return m_softCutDuration; }
void VibeusVisualizer::setHardCutEnabled(bool enabled) { m_hardCutEnabled = enabled; }
void VibeusVisualizer::setHardCutSensitivity(float sensitivity) { m_hardCutSensitivity = sensitivity; }
void VibeusVisualizer::setHardCutDuration(double seconds) { m_hardCutDuration = seconds; }
void VibeusVisualizer::setPresetLocked(bool locked) { m_presetLocked = locked; }
void VibeusVisualizer::setMeshSize(int width, int height) { m_meshWidth = width; m_meshHeight = height; }
void VibeusVisualizer::setAspectCorrection(bool enabled) { m_aspectCorrection = enabled; }
void VibeusVisualizer::setEasterEgg(float value) { m_easterEgg = value; }
void VibeusVisualizer::setTextureSearchPaths(const char** paths, size_t count) {
    if (m_logLevel >= 3) fprintf(stderr, "[VibeusVisualizer] Set %zu texture paths\n", count);
}
void VibeusVisualizer::touch(float x, float y, int pressure, int type) {
    if (m_logLevel >= 4) fprintf(stderr, "[VibeusVisualizer] Touch (%.2f, %.2f) pressure=%d\n", x, y, pressure);
}
void VibeusVisualizer::touchDrag(float x, float y, int pressure) {
    if (m_logLevel >= 5) fprintf(stderr, "[VibeusVisualizer] TouchDrag (%.2f, %.2f)\n", x, y);
}
std::string VibeusVisualizer::getBackendInfo() const { return "Vibeus Custom Backend v0.5.0 (4 Visual Modes + Shockwaves + Starfield + GLSL Ready)"; }
void VibeusVisualizer::setLogLevel(int level) { m_logLevel = level; }
