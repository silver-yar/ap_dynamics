/*
  ==============================================================================

    SliderBarGL.h
    Created: 28 Jan 2021 6:42:26pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class SliderBarGL  : public juce::Component, public juce::OpenGLRenderer
{
public:
    SliderBarGL(const std::string&);
    ~SliderBarGL() override;

    // Context Control Functions
    void start() { openGLContext_.setContinuousRepainting (true); }
    void stop() { openGLContext_.setContinuousRepainting (false); }

    // OpenGL Callbacks
    void newOpenGLContextCreated() override;
    void openGLContextClosing() override;
    void renderOpenGL() override;

    // JUCE Callbacks
    void paint(juce::Graphics&) override;
    void resized() override;

    void setSliderValue(float value) {
        if (uniforms_->sliderVal != nullptr)
        {
            value_ = value;
        }
    };

    void setMeterValue(float value) {
        if (uniforms_->sliderVal != nullptr)
        {
            vmValue_ = value;
        }
    };

private:

    void createShaders();
    // Struct to manage uniforms for the fragment shader
    struct Uniforms
    {
        Uniforms (juce::OpenGLContext& openGLContext, juce::OpenGLShaderProgram& shaderProgram)
        {
            //projectionMatrix = createUniform (openGLContext, shaderProgram, "projectionMatrix");
            //viewMatrix       = createUniform (openGLContext, shaderProgram, "viewMatrix");

            resolution.reset (createUniform (openGLContext, shaderProgram, "resolution"));
            sliderVal.reset (createUniform (openGLContext, shaderProgram, "sliderValue"));
            vmVal.reset (createUniform (openGLContext, shaderProgram, "vmValue"));
            diffTexture.reset (createUniform (openGLContext, shaderProgram, "diffTexture"));
            specTexture.reset (createUniform (openGLContext, shaderProgram, "specTexture"));
            runTime.reset (createUniform (openGLContext, shaderProgram, "runTime"));
        }

        //ScopedPointer<OpenGLShaderProgram::Uniform> projectionMatrix, viewMatrix;
        std::unique_ptr<juce::OpenGLShaderProgram::Uniform> resolution, sliderVal, vmVal,
                                                                diffTexture, runTime, specTexture;

    private:
        static juce::OpenGLShaderProgram::Uniform* createUniform (juce::OpenGLContext& openGLContext,
                                                            juce::OpenGLShaderProgram& shaderProgram,
                                                            const char* uniformName)
        {
            if (openGLContext.extensions.glGetUniformLocation (shaderProgram.getProgramID(), uniformName) < 0)
                return nullptr;

            return new juce::OpenGLShaderProgram::Uniform (shaderProgram, uniformName);
        }
    };

    // OpenGL Member Variables
    juce::OpenGLContext openGLContext_;
    GLuint VBO_, VAO_, EBO_;

    juce::Time time_;
    juce::OpenGLTexture diffTexture_, specTexture_;
    juce::Image cantaloupeImg_ {juce::ImageCache::getFromMemory(BinaryData::cantaloupe_png,
                                                                BinaryData::cantaloupe_pngSize)};
    std::unique_ptr<juce::OpenGLShaderProgram> shader_;
    std::unique_ptr<Uniforms> uniforms_;

    std::string filename_;
    float value_ = 0;
    float vmValue_ = 0.5f;
    float index_ = 0;
    juce::String statusText_;

    const char* vertexShader_;
    const char* fragmentShader_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SliderBarGL)
};
