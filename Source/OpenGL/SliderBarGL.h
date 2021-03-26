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
    explicit SliderBarGL(const std::string&);
    ~SliderBarGL() override;

    // Context Control Functions
    void start() { openGLContext_.setContinuousRepainting (true); }
    void stop() { openGLContext_.setContinuousRepainting (false); }

    // OpenGL Callbacks
    void newOpenGLContextCreated() override;
    void openGLContextClosing() override;
    void renderOpenGL() override;

    void loadCubeMap(std::vector<juce::Image>& texture_images);

    // JUCE Callbacks
    void paint(juce::Graphics&) override;
    void resized() override;

    void setSliderValue(float value) {
        if (uniforms_ != nullptr && uniforms_->sliderVal != nullptr)
        {
            value_ = value;
        }
    };

    void setMeterValue(float value) {
        if (uniforms_ != nullptr && uniforms_->vmVal != nullptr)
        {
            vmValue_ = value;
        }
    };

    int64_t startTime = 0;
private:

    void createShaders();
    // Struct to manage uniforms for the fragment shader
    struct Uniforms
    {
        Uniforms (juce::OpenGLContext& openGLContext, juce::OpenGLShaderProgram& shaderProgram)
        {
            resolution = (createUniform (openGLContext, shaderProgram, "resolution"));
            sliderVal = (createUniform (openGLContext, shaderProgram, "sliderValue"));
            vmVal = (createUniform (openGLContext, shaderProgram, "vomValue"));
            diffTexture = (createUniform (openGLContext, shaderProgram, "diffTexture"));
            specTexture = (createUniform (openGLContext, shaderProgram, "specTexture"));
            runTime = (createUniform (openGLContext, shaderProgram, "runTime"));
        }

        //ScopedPointer<OpenGLShaderProgram::Uniform> projectionMatrix, viewMatrix;
        std::unique_ptr<juce::OpenGLShaderProgram::Uniform> resolution, sliderVal, vmVal,
                                                                diffTexture, runTime, specTexture;

    private:
        static std::unique_ptr<juce::OpenGLShaderProgram::Uniform> createUniform (juce::OpenGLContext& openGLContext,
                                                            juce::OpenGLShaderProgram& shaderProgram,
                                                            const char* uniformName)
        {
            if (openGLContext.extensions.glGetUniformLocation (shaderProgram.getProgramID(), uniformName) < 0)
            {
//                jassertfalse;
//                DBG("Uniform: " << uniformName << ", Location: "
//                                << openGLContext.extensions.glGetUniformLocation (shaderProgram.getProgramID(), uniformName));
                return nullptr;
            }

//            DBG("Uniform: " << uniformName << ", Location: "
//                            << openGLContext.extensions.glGetUniformLocation (shaderProgram.getProgramID(), uniformName));

            return std::make_unique<juce::OpenGLShaderProgram::Uniform> (shaderProgram, uniformName);
        }
    };

    // OpenGL Member Variables
    juce::OpenGLContext openGLContext_;
    GLuint VBO_, VAO_, EBO_;

    juce::Time time_;

    // Texture Objects
    juce::OpenGLTexture diffTexture_, specTexture_;
//    juce::Array<juce::OpenGLTexture> cubeMapTextures_ {
//        &rightTex_, &leftTex_,
//        &topTex_, &bottomTex_,
//        &backTex_, &frontTex_,
//    };

    // Texture Imagesjuce::OpenGLTexture()
    juce::Image diffImage_ {juce::ImageCache::getFromMemory(BinaryData::blue_noise_png,
                                                            BinaryData::blue_noise_pngSize)};

    std::vector<juce::Image> textureFaces_ { // right, left, top, bottom, back, front
            juce::Image {
                juce::ImageCache::getFromMemory(BinaryData::blurCube1_png,
                                                BinaryData::blurCube1_pngSize)
            },
            juce::Image {
                juce::ImageCache::getFromMemory(BinaryData::blurCube2_png,
                                                BinaryData::blurCube2_pngSize)
            },
            juce::Image {
                juce::ImageCache::getFromMemory(BinaryData::blurCube3_png,
                                                BinaryData::blurCube3_pngSize)
            },
            juce::Image {
                juce::ImageCache::getFromMemory(BinaryData::blurCube4_png,
                                                BinaryData::blurCube4_pngSize)
            },
            juce::Image {
                juce::ImageCache::getFromMemory(BinaryData::blurCube5_png,
                                                BinaryData::blurCube5_pngSize)
            },
            juce::Image {
                juce::ImageCache::getFromMemory(BinaryData::blurCube6_png,
                                                BinaryData::blurCube6_pngSize)
            },
    };

    juce::OpenGLShaderProgram shader_;
    std::unique_ptr<Uniforms> uniforms_;

    std::string filename_;
    float value_;
    float vmValue_;
    float index_ = 0.0f;
    juce::String statusText_;

    const char* vertexShader_;
    const char* fragmentShader_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SliderBarGL)
};
