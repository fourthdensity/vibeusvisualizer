#pragma once

#include <SDL.h>
#include <SDL_opengl.h>

#include <random>

class TransitionCompositor {
public:
    bool init();
    void shutdown();

    bool isAvailable() const { return m_available; }
    bool isActive() const;

    void trigger(int transitionStyle, float durationSeconds, bool reducedMotion);
    void cancel();

    bool beginScene(int width, int height);
    void endScene();
    void renderToScreen(int width, int height);
    void captureHistoryFromScreen(int width, int height);

private:
    bool loadFunctions();
    bool createProgram();
    bool ensureTargets(int width, int height);
    GLuint compileShader(GLenum type, const char* source);
    float progress() const;

    bool m_available = false;
    bool m_frameActive = false;
    bool m_reducedMotion = false;

    int m_width = 0;
    int m_height = 0;
    int m_style = 0;
    int m_effectStyle = 1;

    Uint32 m_startTicks = 0;
    float m_durationSeconds = 0.0f;

    GLuint m_fbo = 0;
    GLuint m_texture = 0;
    GLuint m_historyTexture = 0;
    GLuint m_program = 0;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;

    GLint m_uTexture = -1;
    GLint m_uHistoryTexture = -1;
    GLint m_uProgress = -1;
    GLint m_uStyle = -1;
    GLint m_uResolution = -1;
    GLint m_uReducedMotion = -1;
    GLint m_uHasHistory = -1;

    bool m_hasHistory = false;

    std::mt19937 m_rng;
};
