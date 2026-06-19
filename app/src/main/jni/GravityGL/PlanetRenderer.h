#pragma once
#include <GLES3/gl3.h>

class PlanetRenderer {
public:
    PlanetRenderer();
    ~PlanetRenderer();

    void init(int width, int height);
    void render(float time);
    
    GLuint getTexture() const { return m_fboTexture; }
    bool isInitialized() const { return m_initialized; }

private:
    void setupShaders();
    void setupQuad();

    bool m_initialized = false;
    int m_width = 0;
    int m_height = 0;

    GLuint m_fbo = 0;
    GLuint m_fboTexture = 0;
    GLuint m_shaderProgram = 0;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
};
