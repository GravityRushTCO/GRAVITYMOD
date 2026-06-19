#pragma once
#include <GLES3/gl3.h>

class ModelRenderer {
public:
    enum ModelType {
        MODEL_NONE = 0,
        MODEL_WEAPON = 1,
        MODEL_VEHICLE = 2,
        MODEL_SKIN = 3
    };

    ModelRenderer();
    ~ModelRenderer();

    // Initialize FBO and shaders
    void init(int width, int height);
    
    // Render the current model to the FBO
    void render(float time, int modelType, int selectionIndex);
    
    // Get the texture ID for ImGui::Image
    GLuint getTexture() const { return m_fboTexture; }
    
    // Check if initialized
    bool isInitialized() const { return m_initialized; }

private:
    bool m_initialized = false;
    int m_width = 0;
    int m_height = 0;
    
    GLuint m_fbo = 0;
    GLuint m_fboTexture = 0;
    GLuint m_rbo = 0; // Renderbuffer for depth
    
    GLuint m_shaderProgram = 0;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    
    void setupShaders();
    void setupBuffers();
};
