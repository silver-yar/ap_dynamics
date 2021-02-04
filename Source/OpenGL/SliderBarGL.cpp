/*
  ==============================================================================

    SliderBarGL.cpp
    Created: 28 Jan 2021 6:42:26pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#include <JuceHeader.h>
#include "SliderBarGL.h"

//==============================================================================
SliderBarGL::SliderBarGL()
{
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

    // Setup Buffer Objects
    openGLContext_.extensions.glGenBuffers (1, &VBO_); // Vertex Buffer Object
    openGLContext_.extensions.glGenBuffers (1, &EBO_); // Element Buffer Object
}

void SliderBarGL::openGLContextClosing()
{
    shader_.release();
    uniforms_.release();
}

void SliderBarGL::renderOpenGL()
{
    jassert (juce::OpenGLHelpers::isContextActive());
    
    // Setup Viewport
    const float renderingScale = (float) openGLContext_.getRenderingScale();
    glViewport (0, 0, juce::roundToInt (renderingScale * getWidth()), juce::roundToInt (renderingScale * getHeight()));
    
    // Set background Color
    juce::OpenGLHelpers::clear (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
    // Enable Alpha Blending
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Use Shader Program that's been defined
    shader_->use();
    
    // Setup the Uniforms for use in the Shader

    if (uniforms_->resolution != nullptr)
        uniforms_->resolution->set ((GLfloat) renderingScale * getWidth(), (GLfloat) renderingScale * getHeight());

    // Read in samples from ring buffer
    if (uniforms_->audioSampleData != nullptr)
    {

    }
    
    // Define Vertices for a Square (the view plane)
    GLfloat vertices[] = {
            1.0f,   1.0f,  0.0f,  // Top Right
            1.0f,  -1.0f,  0.0f,  // Bottom Right
            -1.0f, -1.0f,  0.0f,  // Bottom Left
            -1.0f,  1.0f,  0.0f   // Top Left
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
    // GL_DYNAMIC_DRAW or GL_STREAM_DRAW
    // Don't we want GL_DYNAMIC_DRAW since this
    // vertex data will be changing alot??
    // test this
    
    // EBO (Element Buffer Object) - Bind and Write to Buffer
    openGLContext_.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, EBO_);
    openGLContext_.extensions.glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STREAM_DRAW);
    // GL_DYNAMIC_DRAW or GL_STREAM_DRAW
    // Don't we want GL_DYNAMIC_DRAW since this
    // vertex data will be changing alot??
    // test this
    
    // Setup Vertex Attributes
    openGLContext_.extensions.glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    openGLContext_.extensions.glEnableVertexAttribArray (0);
    
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

void SliderBarGL::createShaders()
{
    vertexShader_ =
            "attribute vec3 position;\n"
            "\n"
            "void main()\n"
            "{\n"
            "    gl_Position = vec4(position, 1.0);\n"
            "}\n";

    fragmentShader_ =
            "uniform vec2  resolution;\n"
            "uniform float audioSampleData[256];\n"
            "\n"
            "void getAmplitudeForXPos (in float xPos, out float audioAmplitude)\n"
            "{\n"
            // Buffer size - 1
            "   float perfectSamplePosition = 255.0 * xPos / resolution.x;\n"
            "   int leftSampleIndex = int (floor (perfectSamplePosition));\n"
            "   int rightSampleIndex = int (ceil (perfectSamplePosition));\n"
            "   audioAmplitude = mix (audioSampleData[leftSampleIndex], audioSampleData[rightSampleIndex], fract (perfectSamplePosition));\n"
            "}\n"
            "\n"
            "#define THICKNESS 0.02\n"
            "void main()\n"
            "{\n"
            "    float y = gl_FragCoord.y / resolution.y;\n"
            "    float amplitude = 0.0;\n"
            "    getAmplitudeForXPos (gl_FragCoord.x, amplitude);\n"
            "\n"
            // Centers & Reduces Wave Amplitude
            "    amplitude = 0.5 - amplitude / 2.5;\n"
            "    float r = abs (THICKNESS / (amplitude-y));\n"
            "\n"
            "gl_FragColor = vec4 (r - abs (r * 0.2), r - abs (r * 0.2), r - abs (r * 0.2), 1.0);\n"
            "}\n";

    std::unique_ptr<juce::OpenGLShaderProgram> shaderProgramAttempt = std::make_unique<juce::OpenGLShaderProgram> (openGLContext_);

    // Sets up pipeline of shaders and compiles the program
    if (shaderProgramAttempt->addVertexShader (juce::OpenGLHelpers::translateVertexShaderToV3 (vertexShader_))
        && shaderProgramAttempt->addFragmentShader (juce::OpenGLHelpers::translateFragmentShaderToV3 (fragmentShader_))
        && shaderProgramAttempt->link())
    {
        uniforms_.release();
        shader_ = std::move (shaderProgramAttempt);
        uniforms_.reset (new Uniforms (openGLContext_, *shader_));

        statusText_ = "GLSL: v" + juce::String (juce::OpenGLShaderProgram::getLanguageVersion(), 2);
    }
    else
    {
        statusText_ = shaderProgramAttempt->getLastError();
    }

    //triggerAsyncUpdate();
}
