#pragma once
#include <GLES3/gl3.h>

class HexagonBackground {
public:
    HexagonBackground();
    ~HexagonBackground();

    void init(int width, int height);
    void render(float time, float touchX, float touchY);
    GLuint getTexture() const { return m_fboTexture; }
    bool isInitialized() const { return m_initialized; }

private:
    bool m_initialized = false;
    int m_width = 0;
    int m_height = 0;
    
    GLuint m_fbo = 0;
    GLuint m_fboTexture = 0;
    
    GLuint m_shaderProgram = 0;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    
    void setupShaders();
    void setupQuad();
};
