/*
  ==============================================================================

    SliderBarGL.cpp
    Created: 28 Jan 2021 6:42:26pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#include <JuceHeader.h>
#include <fstream>
#include <string>
#include "SliderBarGL.h"

struct ShaderProgramSource
{
    std::string VertexSource;
    std::string FragmentSource;
};

static ShaderProgramSource ParseShader(const std::string& filepath)
{
    std::ifstream stream (filepath);

    enum class ShaderType
    {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;

    while (std::getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
            {
                type = ShaderType::VERTEX;
            }
            else if (line.find("fragment") != std::string::npos)
            {
                type = ShaderType::FRAGMENT;
            }
        }
        else
        {
            ss[(int)type] << line << '\n';
        }
    }

    return { ss[0].str(), ss[1].str() };
}

//==============================================================================
SliderBarGL::SliderBarGL(const std::string& filenameNoPath) : filename_ (filenameNoPath),
                                                              value_ (0.0f),
                                                              vmValue_ (0.5f)
{
    auto now = std::chrono::high_resolution_clock::now();
    startTime = std::chrono::duration_cast<std::chrono::nanoseconds>(
            now.time_since_epoch()).count();
//    time_ = juce::Time::getCurrentTime();
    // Sets OpenGL version to 3.2
    openGLContext_.setOpenGLVersionRequired (
            juce::OpenGLContext::OpenGLVersion::openGL3_2);
    // Attach the OpenGL context to SliderBarGL OpenGLRenderer
    openGLContext_.setRenderer (this);
    openGLContext_.attachTo (*this);
}

SliderBarGL::~SliderBarGL()
{
    // Stop & detach OpenGL
    openGLContext_.setContinuousRepainting (false);
    openGLContext_.detach();
}

void SliderBarGL::newOpenGLContextCreated()
{
    // Setup Shaders
    createShaders();

    // Load image for texture
    diffTexture_.loadImage (diffImage_);

    // Load images for cubemap texture
//    rightTex_.loadImage(textureFaces_[0]);
//    leftTex_.loadImage(textureFaces_[1]);
//    topTex_.loadImage(textureFaces_[2]);
//    bottomTex_.loadImage(textureFaces_[3]);
//    backTex_.loadImage(textureFaces_[4]);
//    frontTex_.loadImage(textureFaces_[5]);

//    specTexture_.loadImage (diffImage_);
    openGLContext_.setTextureMagnificationFilter(juce::OpenGLContext::TextureMagnificationFilter::linear);

    // Setup Buffer Objects
    openGLContext_.extensions.glGenBuffers (1, &VBO_); // Vertex Buffer Object
    openGLContext_.extensions.glGenBuffers (1, &EBO_); // Element Buffer Object
}

void SliderBarGL::openGLContextClosing()
{
    uniforms_.release();
    diffTexture_.release();
//    rightTex_.release();
//    leftTex_.release();
//    topTex_.release();
//    bottomTex_.release();
//    backTex_.release();
//    frontTex_.release();
//    for (auto &texture : cubeMapTextures_)
//    {
//        texture.release();
//    }
//    specTexture_.release();
    shader_.release();
}

void SliderBarGL::renderOpenGL()
{
    jassert (juce::OpenGLHelpers::isContextActive());
    
    // Setup Viewport
    const float renderingScale = (float) openGLContext_.getRenderingScale();
    glViewport (0, 0, juce::roundToInt (renderingScale * getWidth()), juce::roundToInt (renderingScale * getHeight()));
    
    // Set background Color
    juce::OpenGLHelpers::clear (juce::Colours::grey);
    
    // Enable Alpha Blending
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    openGLContext_.extensions.glActiveTexture (GL_TEXTURE0);
    glEnable (GL_TEXTURE_2D);

    if (uniforms_ == nullptr || shader_ == nullptr) {
        return;
    }

    if (uniforms_->diffTexture != nullptr)
    {
        diffTexture_.bind();
    }


//    if (uniforms_->specTexture != nullptr)
//    {
//        specTexture_.bind();
//    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    // Use Shader Program that's been defined
    shader_->use();
    
    // Setup the Uniforms for use in the Shader

    if (uniforms_->resolution != nullptr) // resolution
    {
        uniforms_->resolution->set((GLfloat) renderingScale * getWidth(),
                                   (GLfloat) renderingScale * getHeight());
    }

    if (uniforms_->sliderVal != nullptr) // sliderVal
    {
        uniforms_->sliderVal->set((GLfloat) value_);
    }

    if (uniforms_->vmVal != nullptr) // vmVal
    {
        uniforms_->vmVal->set((GLfloat) vmValue_);
        if (vmValue_ < 1)
            vmValue_ += 0.01f;
        else
            vmValue_ = 0.5f;
    }

    if (uniforms_->diffTexture != nullptr)
    {
        uniforms_->diffTexture->set((GLint) 0);
    }

    if (uniforms_->specTexture != nullptr)
    {
        uniforms_->specTexture->set((GLint) loadCubeMap());
    }

    if (uniforms_->runTime != nullptr) {
        auto now = std::chrono::high_resolution_clock::now();
        auto sysTime = std::chrono::duration_cast<std::chrono::nanoseconds>(
                now.time_since_epoch()).count();
        auto elapsed = sysTime - startTime;
        float seconds = elapsed / 1000000000.0;
        uniforms_->runTime->set((GLfloat) seconds );
    }
    
    // Define Vertices for a Square (the view plane)
    GLfloat vertices[] = {
            1.0f,   1.0f, 0.0f, 0.0f,  // Top Right + Tex Coord.
            1.0f,  -1.0f, 1.0f, 0.0f, // Bottom Right + Tex Coord.
            -1.0f, -1.0f, 1.0f, 1.0f, // Bottom Left + Tex Coord.
            -1.0f,  1.0f, 0.0f, 1.0f,  // Top Left + Tex Coord.
    };
    // Define Which Vertex Indexes Make the Square
    GLuint indices[] = {  // Note that we start from 0!
            0, 1, 3,   // First Triangle
            1, 2, 3    // Second Triangle
    };
    
    // Vertex Array Object stuff for later
    //openGLContext_.extensions.glGenVertexArrays(1, &VAO);
    //openGLContext_.extensions.glBindVertexArray(VAO);
    
    // VBO (Vertex Buffer Object) - Bind and Write to Buffer
    openGLContext_.extensions.glBindBuffer (GL_ARRAY_BUFFER, VBO_);
    openGLContext_.extensions.glBufferData (GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
    
    // EBO (Element Buffer Object) - Bind and Write to Buffer
    openGLContext_.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, EBO_);
    openGLContext_.extensions.glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STREAM_DRAW);
    
    // Setup Vertex Attributes
    openGLContext_.extensions.glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    openGLContext_.extensions.glEnableVertexAttribArray (0);

    // Setup Texture Coordinate Attributes
    openGLContext_.extensions.glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
                                                     (GLvoid*) (2 * sizeof(GLfloat)));
    openGLContext_.extensions.glEnableVertexAttribArray (1);
    
    // Draw Vertices
    //glDrawArrays (GL_TRIANGLES, 0, 6); // For just VBO's (Vertex Buffer Objects)
    glDrawElements (GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // For EBO's (Element Buffer Objects) (Indices)
    
    
    
    // Reset the element buffers so child Components draw correctly
    openGLContext_.extensions.glBindBuffer (GL_ARRAY_BUFFER, 0);
    openGLContext_.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
    //openGLContext_.extensions.glBindVertexArray(0);
}

void SliderBarGL::paint (juce::Graphics& g)
{
}

void SliderBarGL::resized()
{
}

unsigned int SliderBarGL::loadCubeMap()
{
    unsigned int textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

    for (auto i = 0; i < textureFaces_.size(); i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB,
                     textureFaces_[i].getWidth(),
                     textureFaces_[i].getHeight(),
                     0,
                     GL_RGB,
                     GL_UNSIGNED_BYTE,
                     textureFaces_[i].getPixelData()
        );
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureId;
}

void SliderBarGL::createShaders()
{
//    std::unique_ptr<juce::OpenGLShaderProgram> shaderProgramAttempt = std::make_unique<juce::OpenGLShaderProgram> (openGLContext_);
    shader_ = std::make_unique<juce::OpenGLShaderProgram> (openGLContext_);

    // Retrieve shader from file
    ShaderProgramSource source = ParseShader("../../../../../../Resources/shaders/" + filename_);
    DBG("Shader Selected: " + filename_ + '\n');

    // Sets up pipeline of shaders and compiles the program
    if (shader_->addShader (source.VertexSource, GL_VERTEX_SHADER)
        && shader_->addShader (source.FragmentSource, GL_FRAGMENT_SHADER)
        && shader_->link())
    {
//        shader_ = std::move (shader_);
        uniforms_ = std::make_unique<Uniforms> (openGLContext_, *shader_);

        statusText_ = "GLSL: v" + juce::String (juce::OpenGLShaderProgram::getLanguageVersion(), 2);
    }
    else
    {
        statusText_ = shader_->getLastError();
        shader_ = nullptr;
    }

    //triggerAsyncUpdate();
}
