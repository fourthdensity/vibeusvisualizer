#include "transition_compositor.h"

#include <algorithm>
#include <cstdio>

#ifndef APIENTRY
#define APIENTRY
#endif

namespace {

using PFNGLGENFRAMEBUFFERSPROC_ = void (APIENTRY*)(GLsizei, GLuint*);
using PFNGLBINDFRAMEBUFFERPROC_ = void (APIENTRY*)(GLenum, GLuint);
using PFNGLFRAMEBUFFERTEXTURE2DPROC_ = void (APIENTRY*)(GLenum, GLenum, GLenum, GLuint, GLint);
using PFNGLCHECKFRAMEBUFFERSTATUSPROC_ = GLenum (APIENTRY*)(GLenum);
using PFNGLDELETEFRAMEBUFFERSPROC_ = void (APIENTRY*)(GLsizei, const GLuint*);
using PFNGLCREATESHADERPROC_ = GLuint (APIENTRY*)(GLenum);
using PFNGLSHADERSOURCEPROC_ = void (APIENTRY*)(GLuint, GLsizei, const GLchar* const*, const GLint*);
using PFNGLCOMPILESHADERPROC_ = void (APIENTRY*)(GLuint);
using PFNGLGETSHADERIVPROC_ = void (APIENTRY*)(GLuint, GLenum, GLint*);
using PFNGLGETSHADERINFOLOGPROC_ = void (APIENTRY*)(GLuint, GLsizei, GLsizei*, GLchar*);
using PFNGLDELETESHADERPROC_ = void (APIENTRY*)(GLuint);
using PFNGLCREATEPROGRAMPROC_ = GLuint (APIENTRY*)();
using PFNGLATTACHSHADERPROC_ = void (APIENTRY*)(GLuint, GLuint);
using PFNGLLINKPROGRAMPROC_ = void (APIENTRY*)(GLuint);
using PFNGLGETPROGRAMIVPROC_ = void (APIENTRY*)(GLuint, GLenum, GLint*);
using PFNGLGETPROGRAMINFOLOGPROC_ = void (APIENTRY*)(GLuint, GLsizei, GLsizei*, GLchar*);
using PFNGLDELETEPROGRAMPROC_ = void (APIENTRY*)(GLuint);
using PFNGLUSEPROGRAMPROC_ = void (APIENTRY*)(GLuint);
using PFNGLGENBUFFERSPROC_ = void (APIENTRY*)(GLsizei, GLuint*);
using PFNGLBINDBUFFERPROC_ = void (APIENTRY*)(GLenum, GLuint);
using PFNGLBUFFERDATAPROC_ = void (APIENTRY*)(GLenum, GLsizeiptr, const void*, GLenum);
using PFNGLDELETEBUFFERSPROC_ = void (APIENTRY*)(GLsizei, const GLuint*);
using PFNGLGENVERTEXARRAYSPROC_ = void (APIENTRY*)(GLsizei, GLuint*);
using PFNGLBINDVERTEXARRAYPROC_ = void (APIENTRY*)(GLuint);
using PFNGLDELETEVERTEXARRAYSPROC_ = void (APIENTRY*)(GLsizei, const GLuint*);
using PFNGLENABLEVERTEXATTRIBARRAYPROC_ = void (APIENTRY*)(GLuint);
using PFNGLVERTEXATTRIBPOINTERPROC_ = void (APIENTRY*)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*);
using PFNGLGETUNIFORMLOCATIONPROC_ = GLint (APIENTRY*)(GLuint, const GLchar*);
using PFNGLUNIFORM1IPROC_ = void (APIENTRY*)(GLint, GLint);
using PFNGLUNIFORM1FPROC_ = void (APIENTRY*)(GLint, GLfloat);
using PFNGLUNIFORM2FPROC_ = void (APIENTRY*)(GLint, GLfloat, GLfloat);
using PFNGLACTIVETEXTUREPROC_ = void (APIENTRY*)(GLenum);

PFNGLGENFRAMEBUFFERSPROC_ pglGenFramebuffers = nullptr;
PFNGLBINDFRAMEBUFFERPROC_ pglBindFramebuffer = nullptr;
PFNGLFRAMEBUFFERTEXTURE2DPROC_ pglFramebufferTexture2D = nullptr;
PFNGLCHECKFRAMEBUFFERSTATUSPROC_ pglCheckFramebufferStatus = nullptr;
PFNGLDELETEFRAMEBUFFERSPROC_ pglDeleteFramebuffers = nullptr;
PFNGLCREATESHADERPROC_ pglCreateShader = nullptr;
PFNGLSHADERSOURCEPROC_ pglShaderSource = nullptr;
PFNGLCOMPILESHADERPROC_ pglCompileShader = nullptr;
PFNGLGETSHADERIVPROC_ pglGetShaderiv = nullptr;
PFNGLGETSHADERINFOLOGPROC_ pglGetShaderInfoLog = nullptr;
PFNGLDELETESHADERPROC_ pglDeleteShader = nullptr;
PFNGLCREATEPROGRAMPROC_ pglCreateProgram = nullptr;
PFNGLATTACHSHADERPROC_ pglAttachShader = nullptr;
PFNGLLINKPROGRAMPROC_ pglLinkProgram = nullptr;
PFNGLGETPROGRAMIVPROC_ pglGetProgramiv = nullptr;
PFNGLGETPROGRAMINFOLOGPROC_ pglGetProgramInfoLog = nullptr;
PFNGLDELETEPROGRAMPROC_ pglDeleteProgram = nullptr;
PFNGLUSEPROGRAMPROC_ pglUseProgram = nullptr;
PFNGLGENBUFFERSPROC_ pglGenBuffers = nullptr;
PFNGLBINDBUFFERPROC_ pglBindBuffer = nullptr;
PFNGLBUFFERDATAPROC_ pglBufferData = nullptr;
PFNGLDELETEBUFFERSPROC_ pglDeleteBuffers = nullptr;
PFNGLGENVERTEXARRAYSPROC_ pglGenVertexArrays = nullptr;
PFNGLBINDVERTEXARRAYPROC_ pglBindVertexArray = nullptr;
PFNGLDELETEVERTEXARRAYSPROC_ pglDeleteVertexArrays = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC_ pglEnableVertexAttribArray = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC_ pglVertexAttribPointer = nullptr;
PFNGLGETUNIFORMLOCATIONPROC_ pglGetUniformLocation = nullptr;
PFNGLUNIFORM1IPROC_ pglUniform1i = nullptr;
PFNGLUNIFORM1FPROC_ pglUniform1f = nullptr;
PFNGLUNIFORM2FPROC_ pglUniform2f = nullptr;
PFNGLACTIVETEXTUREPROC_ pglActiveTexture = nullptr;

template <typename T>
bool loadProc(T& target, const char* name)
{
    target = reinterpret_cast<T>(SDL_GL_GetProcAddress(name));
    if (!target) {
        fprintf(stderr, "[TransitionCompositor] Missing GL function: %s\n", name);
        return false;
    }
    return true;
}

constexpr int kMaxTransitionStyle = 18;

const char* kVertexShader = R"GLSL(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUv;
out vec2 vUv;
void main()
{
    vUv = aUv;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)GLSL";

const char* kFragmentShader = R"GLSL(
#version 330 core
in vec2 vUv;
out vec4 fragColor;

uniform sampler2D uTexture;
uniform sampler2D uHistoryTexture;
uniform float uProgress;
uniform int uStyle;
uniform vec2 uResolution;
uniform float uReducedMotion;
uniform float uHasHistory;

vec3 sampleRgb(vec2 uv)
{
    return texture(uTexture, clamp(uv, vec2(0.0), vec2(1.0))).rgb;
}

vec3 sampleHistory(vec2 uv)
{
    return texture(uHistoryTexture, clamp(uv, vec2(0.0), vec2(1.0))).rgb;
}

float luminance(vec3 color)
{
    return dot(color, vec3(0.2126, 0.7152, 0.0722));
}

void main()
{
    vec2 uv = vUv;
    vec2 center = vec2(0.5);
    vec2 fromCenter = uv - center;
    float p = clamp(uProgress, 0.0, 1.0);
    float inv = 1.0 - p;
    float amp = mix(1.0, 0.45, uReducedMotion);
    int style = uStyle;
    float eased = smoothstep(0.0, 1.0, p);

    vec2 oldUv = uv;
    vec3 oldColor = mix(sampleRgb(uv), sampleHistory(oldUv), uHasHistory);
    vec3 color;
    float flash = 0.0;
    float reveal = eased;

    if (style == 1) { // Flash Cut
        color = sampleRgb(uv);
        reveal = smoothstep(0.05, 0.28, p);
        flash = smoothstep(1.0, 0.0, p) * 0.65 * amp;
    } else if (style == 2) { // Slow Morph
        oldUv -= fromCenter * sin(p * 3.14159) * 0.05 * amp;
        oldColor = mix(sampleRgb(uv), sampleHistory(oldUv), uHasHistory);
        uv += fromCenter * sin(p * 3.14159) * 0.08 * amp;
        uv.x += sin((uv.y + p) * 18.0) * 0.012 * inv * amp;
        color = sampleRgb(uv);
        reveal = smoothstep(0.08, 0.92, p);
    } else if (style == 3) { // Quick Blend
        float z = 1.0 + inv * 0.18 * amp;
        color = sampleRgb(center + fromCenter / z);
        reveal = smoothstep(0.15, 0.70, p);
    } else if (style == 4) { // Glitch Cut
        float bands = floor(uv.y * 24.0);
        float offset = sin(bands * 19.17 + p * 32.0) * 0.035 * inv * amp;
        oldUv.x -= offset * 0.4;
        oldColor = mix(sampleRgb(uv), sampleHistory(oldUv), uHasHistory);
        uv.x += offset;
        color.r = sampleRgb(uv + vec2(0.012 * inv * amp, 0.0)).r;
        color.g = sampleRgb(uv).g;
        color.b = sampleRgb(uv - vec2(0.014 * inv * amp, 0.0)).b;
        reveal = smoothstep(0.10, 0.45, p);
    } else if (style == 5) { // Zoom Burst
        float z = 1.0 + inv * inv * 0.75 * amp;
        oldColor = mix(sampleRgb(uv), sampleHistory(center + fromCenter * (1.0 + p * 0.22 * amp)), uHasHistory);
        color = sampleRgb(center + fromCenter / z);
        color += sampleRgb(center + fromCenter / (z + 0.12)) * 0.20 * inv * amp;
        reveal = smoothstep(0.12, 0.82, p);
    } else if (style == 6) { // Energy Flash
        vec2 shift = normalize(fromCenter + vec2(0.001)) * 0.018 * inv * amp;
        color.r = sampleRgb(uv + shift).r;
        color.g = sampleRgb(uv).g;
        color.b = sampleRgb(uv - shift).b;
        reveal = smoothstep(0.05, 0.55, p);
        flash = 0.45 * inv * amp;
    } else if (style == 7) { // Snap Fade
        color = sampleRgb(uv);
        color *= smoothstep(0.0, 0.18, p);
        reveal = smoothstep(0.08, 0.24, p);
    } else if (style == 8) { // Long Dissolve
        float dither = fract(sin(dot(floor(uv * uResolution / 4.0), vec2(12.9898,78.233))) * 43758.5453);
        color = sampleRgb(uv);
        reveal = smoothstep(dither * 0.28, 1.0, p + 0.08);
    } else if (style == 9) { // Pulse Blend
        float pulse = sin(p * 3.14159 * 4.0) * inv;
        oldColor = mix(sampleRgb(uv), sampleHistory(uv - fromCenter * pulse * 0.05 * amp), uHasHistory);
        color = sampleRgb(uv + fromCenter * pulse * 0.08 * amp);
        color *= 1.0 + pulse * 0.25 * amp;
        reveal = smoothstep(0.10, 0.78, p);
    } else if (style == 10) { // Bass Slam
        float slam = exp(-p * 6.0);
        vec2 slammed = center + fromCenter * (1.0 - slam * 0.18 * amp);
        oldColor = mix(sampleRgb(uv), sampleHistory(center + fromCenter * (1.0 + slam * 0.18 * amp)), uHasHistory);
        color = sampleRgb(slammed);
        reveal = smoothstep(0.04, 0.34, p);
        flash = slam * 0.35 * amp;
    } else if (style == 11) { // Breathing Fade
        float breathe = sin(p * 3.14159);
        oldColor = mix(sampleRgb(uv), sampleHistory(uv - fromCenter * breathe * 0.03 * amp), uHasHistory);
        uv += fromCenter * breathe * 0.05 * amp;
        color = sampleRgb(uv) * (0.85 + breathe * 0.22 * amp);
        reveal = smoothstep(0.12, 0.88, p);
    } else if (style == 12) { // Clean Slate
        color = sampleRgb(uv);
        color *= smoothstep(0.08, 0.35, p);
        oldColor *= 1.0 - smoothstep(0.0, 0.22, p);
        reveal = smoothstep(0.20, 0.46, p);
    } else if (style == 13) { // Liquid Drift
        oldUv.x -= sin((uv.y * 8.0) + p * 6.0) * 0.020 * p * amp;
        oldColor = mix(sampleRgb(uv), sampleHistory(oldUv), uHasHistory);
        uv.x += sin((uv.y * 10.0) + p * 8.0) * 0.035 * inv * amp;
        uv.y += cos((uv.x * 8.0) - p * 7.0) * 0.025 * inv * amp;
        color = sampleRgb(uv);
        reveal = smoothstep(0.10, 0.88, p);
    } else if (style == 14) { // Ambient Wash
        color = sampleRgb(uv);
        float l = luminance(color);
        color = mix(color, vec3(l) * vec3(0.75, 0.88, 1.10), 0.35 * inv * amp);
        reveal = smoothstep(0.05, 0.95, p);
    } else if (style == 15) { // Spark Jump
        float sparkle = step(0.985, fract(sin(dot(floor(uv * uResolution / 6.0), vec2(43.13, 91.7))) * 927.17 + p * 12.0));
        color = sampleRgb(uv);
        color += sparkle * inv * vec3(0.8, 0.9, 1.0) * amp;
        reveal = smoothstep(0.08, 0.55, p);
    } else if (style == 16) { // Deep Bloom
        color = sampleRgb(uv);
        vec3 bloom = sampleRgb(center + fromCenter * 0.92) + sampleRgb(center + fromCenter * 0.82);
        color += bloom * 0.12 * inv * amp;
        reveal = smoothstep(0.14, 0.86, p);
    } else if (style == 17) { // Afterimage
        color = sampleRgb(uv) * 0.72;
        color += sampleHistory(center + fromCenter * (1.0 + 0.06 * inv * amp)) * 0.22 * uHasHistory;
        color += sampleHistory(center + fromCenter * (1.0 + 0.12 * inv * amp)) * 0.12 * inv * uHasHistory;
        reveal = smoothstep(0.10, 0.80, p);
    } else if (style == 18) { // Drop Smash
        float shake = sin(p * 120.0) * 0.025 * inv * amp;
        oldColor = mix(sampleRgb(uv), sampleHistory(uv - vec2(shake * 0.7, 0.0)), uHasHistory);
        color.r = sampleRgb(uv + vec2(shake, 0.0)).r;
        color.g = sampleRgb(uv - vec2(shake * 0.6, 0.0)).g;
        color.b = sampleRgb(uv + vec2(0.0, shake * 0.7)).b;
        reveal = smoothstep(0.02, 0.25, p);
        flash = 0.55 * inv * amp;
    } else {
        color = sampleRgb(uv);
    }

    // Centralized transition policy:
    // all major visual-world changes use smoothstep opacity blending.
    // Clean Slate is the restrained dip-to-black reset style.
    if (style == 12) {
        float outT = smoothstep(0.0, 1.0, clamp(p / 0.5, 0.0, 1.0));
        float inT = smoothstep(0.0, 1.0, clamp((p - 0.5) / 0.5, 0.0, 1.0));
        vec3 dipped = mix(oldColor, vec3(0.0), outT);
        color = mix(dipped, color, inT);
    } else {
        float glow = 0.10 * sin(eased * 3.14159) * amp;
        color = mix(oldColor, color * (1.0 + glow), eased);
    }
    color = mix(color, vec3(1.0), flash * 0.25 * (1.0 - uReducedMotion));
    fragColor = vec4(color, 1.0);
}
)GLSL";

} // namespace

bool TransitionCompositor::init()
{
    if (m_available)
        return true;

    m_rng.seed(SDL_GetTicks());

    if (!loadFunctions() || !createProgram()) {
        shutdown();
        return false;
    }

    const float vertices[] = {
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
    };

    pglGenVertexArrays(1, &m_vao);
    pglGenBuffers(1, &m_vbo);
    pglBindVertexArray(m_vao);
    pglBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    pglBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    pglEnableVertexAttribArray(0);
    pglVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0));
    pglEnableVertexAttribArray(1);
    pglVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));
    pglBindVertexArray(0);

    m_available = true;
    fprintf(stderr, "[TransitionCompositor] Vibeus post-processing transition layer enabled\n");
    return true;
}

void TransitionCompositor::shutdown()
{
    if (m_fbo) {
        pglDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
    }
    if (m_texture) {
        glDeleteTextures(1, &m_texture);
        m_texture = 0;
    }
    if (m_historyTexture) {
        glDeleteTextures(1, &m_historyTexture);
        m_historyTexture = 0;
    }
    if (m_vbo) {
        pglDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    if (m_vao) {
        pglDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    if (m_program) {
        pglDeleteProgram(m_program);
        m_program = 0;
    }

    m_available = false;
    m_frameActive = false;
    m_hasHistory = false;
}

bool TransitionCompositor::isActive() const
{
    return m_available && m_durationSeconds > 0.0f && progress() < 1.0f;
}

void TransitionCompositor::trigger(int transitionStyle, float durationSeconds, bool reducedMotion)
{
    if (!m_available)
        return;

    if (isActive()) {
        fprintf(stderr, "[TransitionCompositor] Suppressed overlapping trigger style=%d activeProgress=%.2f\n",
                transitionStyle, progress());
        return;
    }

    m_style = transitionStyle;
    if (m_style <= 0) {
        std::uniform_int_distribution<int> dist(1, kMaxTransitionStyle);
        m_effectStyle = dist(m_rng);
    } else {
        m_effectStyle = std::clamp(m_style, 1, kMaxTransitionStyle);
    }

    m_reducedMotion = reducedMotion;
    m_durationSeconds = reducedMotion
        ? std::min(durationSeconds, 0.8f)
        : std::clamp(durationSeconds, 0.50f, 1.20f);
    m_startTicks = SDL_GetTicks();

    fprintf(stderr, "[TransitionCompositor] Trigger style=%d effect=%d duration=%.2fs\n",
            m_style, m_effectStyle, m_durationSeconds);
}

void TransitionCompositor::cancel()
{
    m_durationSeconds = 0.0f;
    m_startTicks = 0;
    m_frameActive = false;
}

bool TransitionCompositor::beginScene(int width, int height)
{
    if (!isActive())
        return false;
    if (!ensureTargets(width, height))
        return false;

    pglBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);
    m_frameActive = true;
    return true;
}

void TransitionCompositor::endScene()
{
    if (!m_frameActive)
        return;
    pglBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_frameActive = false;
}

void TransitionCompositor::renderToScreen(int width, int height)
{
    if (!m_available || !m_texture || !m_program)
        return;

    pglBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    pglUseProgram(m_program);
    pglActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    pglUniform1i(m_uTexture, 0);
    pglActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_historyTexture ? m_historyTexture : m_texture);
    pglUniform1i(m_uHistoryTexture, 1);
    pglUniform1f(m_uProgress, progress());
    pglUniform1i(m_uStyle, m_effectStyle);
    pglUniform2f(m_uResolution, static_cast<float>(width), static_cast<float>(height));
    pglUniform1f(m_uReducedMotion, m_reducedMotion ? 1.0f : 0.0f);
    pglUniform1f(m_uHasHistory, m_hasHistory ? 1.0f : 0.0f);

    pglBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    pglBindVertexArray(0);
    pglActiveTexture(GL_TEXTURE0);
    pglUseProgram(0);
}

void TransitionCompositor::captureHistoryFromScreen(int width, int height)
{
    if (!m_available || isActive())
        return;
    if (!ensureTargets(width, height) || !m_historyTexture)
        return;

    glBindTexture(GL_TEXTURE_2D, m_historyTexture);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);
    m_hasHistory = true;
}

bool TransitionCompositor::loadFunctions()
{
    bool ok = true;
    ok &= loadProc(pglGenFramebuffers, "glGenFramebuffers");
    ok &= loadProc(pglBindFramebuffer, "glBindFramebuffer");
    ok &= loadProc(pglFramebufferTexture2D, "glFramebufferTexture2D");
    ok &= loadProc(pglCheckFramebufferStatus, "glCheckFramebufferStatus");
    ok &= loadProc(pglDeleteFramebuffers, "glDeleteFramebuffers");
    ok &= loadProc(pglCreateShader, "glCreateShader");
    ok &= loadProc(pglShaderSource, "glShaderSource");
    ok &= loadProc(pglCompileShader, "glCompileShader");
    ok &= loadProc(pglGetShaderiv, "glGetShaderiv");
    ok &= loadProc(pglGetShaderInfoLog, "glGetShaderInfoLog");
    ok &= loadProc(pglDeleteShader, "glDeleteShader");
    ok &= loadProc(pglCreateProgram, "glCreateProgram");
    ok &= loadProc(pglAttachShader, "glAttachShader");
    ok &= loadProc(pglLinkProgram, "glLinkProgram");
    ok &= loadProc(pglGetProgramiv, "glGetProgramiv");
    ok &= loadProc(pglGetProgramInfoLog, "glGetProgramInfoLog");
    ok &= loadProc(pglDeleteProgram, "glDeleteProgram");
    ok &= loadProc(pglUseProgram, "glUseProgram");
    ok &= loadProc(pglGenBuffers, "glGenBuffers");
    ok &= loadProc(pglBindBuffer, "glBindBuffer");
    ok &= loadProc(pglBufferData, "glBufferData");
    ok &= loadProc(pglDeleteBuffers, "glDeleteBuffers");
    ok &= loadProc(pglGenVertexArrays, "glGenVertexArrays");
    ok &= loadProc(pglBindVertexArray, "glBindVertexArray");
    ok &= loadProc(pglDeleteVertexArrays, "glDeleteVertexArrays");
    ok &= loadProc(pglEnableVertexAttribArray, "glEnableVertexAttribArray");
    ok &= loadProc(pglVertexAttribPointer, "glVertexAttribPointer");
    ok &= loadProc(pglGetUniformLocation, "glGetUniformLocation");
    ok &= loadProc(pglUniform1i, "glUniform1i");
    ok &= loadProc(pglUniform1f, "glUniform1f");
    ok &= loadProc(pglUniform2f, "glUniform2f");
    ok &= loadProc(pglActiveTexture, "glActiveTexture");
    return ok;
}

bool TransitionCompositor::createProgram()
{
    GLuint vs = compileShader(GL_VERTEX_SHADER, kVertexShader);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, kFragmentShader);
    if (!vs || !fs)
        return false;

    m_program = pglCreateProgram();
    pglAttachShader(m_program, vs);
    pglAttachShader(m_program, fs);
    pglLinkProgram(m_program);

    GLint linked = 0;
    pglGetProgramiv(m_program, GL_LINK_STATUS, &linked);
    if (!linked) {
        char log[2048] = {};
        pglGetProgramInfoLog(m_program, sizeof(log), nullptr, log);
        fprintf(stderr, "[TransitionCompositor] Program link failed: %s\n", log);
        pglDeleteShader(vs);
        pglDeleteShader(fs);
        return false;
    }

    pglDeleteShader(vs);
    pglDeleteShader(fs);

    m_uTexture = pglGetUniformLocation(m_program, "uTexture");
    m_uHistoryTexture = pglGetUniformLocation(m_program, "uHistoryTexture");
    m_uProgress = pglGetUniformLocation(m_program, "uProgress");
    m_uStyle = pglGetUniformLocation(m_program, "uStyle");
    m_uResolution = pglGetUniformLocation(m_program, "uResolution");
    m_uReducedMotion = pglGetUniformLocation(m_program, "uReducedMotion");
    m_uHasHistory = pglGetUniformLocation(m_program, "uHasHistory");
    return true;
}

bool TransitionCompositor::ensureTargets(int width, int height)
{
    if (width <= 0 || height <= 0)
        return false;
    if (m_fbo && m_texture && m_historyTexture && width == m_width && height == m_height)
        return true;

    if (!m_fbo)
        pglGenFramebuffers(1, &m_fbo);
    if (!m_texture)
        glGenTextures(1, &m_texture);
    if (!m_historyTexture)
        glGenTextures(1, &m_historyTexture);

    m_width = width;
    m_height = height;

    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, m_historyTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    m_hasHistory = false;

    pglBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    pglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);

    GLenum status = pglCheckFramebufferStatus(GL_FRAMEBUFFER);
    pglBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "[TransitionCompositor] Framebuffer incomplete: 0x%x\n", status);
        return false;
    }
    return true;
}

GLuint TransitionCompositor::compileShader(GLenum type, const char* source)
{
    GLuint shader = pglCreateShader(type);
    pglShaderSource(shader, 1, &source, nullptr);
    pglCompileShader(shader);

    GLint compiled = 0;
    pglGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        char log[2048] = {};
        pglGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        fprintf(stderr, "[TransitionCompositor] Shader compile failed: %s\n", log);
        pglDeleteShader(shader);
        return 0;
    }
    return shader;
}

float TransitionCompositor::progress() const
{
    if (m_durationSeconds <= 0.0f)
        return 1.0f;
    Uint32 elapsed = SDL_GetTicks() - m_startTicks;
    return std::clamp((elapsed / 1000.0f) / m_durationSeconds, 0.0f, 1.0f);
}
