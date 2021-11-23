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
class SliderBarGL : public juce::Component, public juce::OpenGLRenderer
{
 public:
  explicit SliderBarGL(std::string );
  ~SliderBarGL() override;

  // Context Control Functions
  void start() { openGLContext_.setContinuousRepainting(true); }
  void stop() { openGLContext_.setContinuousRepainting(false); }

  // OpenGL Callbacks
  void newOpenGLContextCreated() override;
  void openGLContextClosing() override;
  void renderOpenGL() override;

  // JUCE Callbacks
  void paint(juce::Graphics&) override;
  void resized() override;

  void setSliderValue(float value)
  {
    if (uniforms_ != nullptr && uniforms_->sliderVal != nullptr)
    {
      value_ = value;
    }
  }

  void setMeterValue(float value)
  {
    if (uniforms_ != nullptr && uniforms_->vmVal != nullptr)
    {
      vmValue_ = value;
    }
  }

  int64_t startTime = 0;

 private:
  void createShaders();
  // Struct to manage uniforms for the fragment shader
  struct Uniforms
  {
    Uniforms(juce::OpenGLContext& openGLContext, juce::OpenGLShaderProgram& shaderProgram)
    {
      resolution  = (createUniform(openGLContext, shaderProgram, "resolution"));
      sliderVal   = (createUniform(openGLContext, shaderProgram, "sliderValue"));
      vmVal       = (createUniform(openGLContext, shaderProgram, "vomValue"));
      diffTexture = (createUniform(openGLContext, shaderProgram, "diffTexture"));
      specTexture = (createUniform(openGLContext, shaderProgram, "specTexture"));
      runTime     = (createUniform(openGLContext, shaderProgram, "runTime"));
    }

    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> resolution, sliderVal, vmVal, diffTexture, runTime, specTexture;

   private:
    static std::unique_ptr<juce::OpenGLShaderProgram::Uniform> createUniform(juce::OpenGLContext& openGLContext,
                                                                             juce::OpenGLShaderProgram& shaderProgram,
                                                                             const char* uniformName)
    {
      if (juce::OpenGLExtensionFunctions::glGetUniformLocation(shaderProgram.getProgramID(), uniformName) < 0)
      {
        return nullptr;
      }

      return std::make_unique<juce::OpenGLShaderProgram::Uniform>(shaderProgram, uniformName);
    }
  };

  // OpenGL Member Variables
  juce::OpenGLContext openGLContext_;
  GLuint VBO_, VAO_, EBO_;

  juce::Time time_;

  // Texture Objects
  juce::OpenGLTexture diffTexture_, specTexture_;

  // Texture Imagesjuce::OpenGLTexture()
  juce::Image diffImage_{ juce::ImageCache::getFromMemory(BinaryData::blue_noise_png, BinaryData::blue_noise_pngSize) };

  juce::OpenGLShaderProgram shader_;
  std::unique_ptr<Uniforms> uniforms_;

  std::string filename_;
  float value_;
  float vmValue_;
  juce::String statusText_;

  const char* vertexShader_;
  const char* fragmentShader_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SliderBarGL)
};
