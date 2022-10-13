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
  explicit SliderBarGL(std::string);
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

  void setSliderValue(const float value)
  {
    if (uniforms_ != nullptr && uniforms_->sliderVal != nullptr)
    {
      value_ = value;
    }
  }

  void setMeterValue(const float value)
  {
    if (uniforms_ != nullptr && uniforms_->vmVal != nullptr)
    {
      vmValue_ = value;
    }
  }

 private:
  void createShaders();
  // Struct to manage uniforms for the fragment shader
  struct Uniforms
  {
    explicit Uniforms(juce::OpenGLShaderProgram& shaderProgram)
    {
      resolution  = (createUniform(shaderProgram, "resolution"));
      sliderVal   = (createUniform(shaderProgram, "sliderValue"));
      vmVal       = (createUniform(shaderProgram, "vomValue"));
      diffTexture = (createUniform(shaderProgram, "diffTexture"));
      specTexture = (createUniform(shaderProgram, "specTexture"));
      runTime     = (createUniform(shaderProgram, "runTime"));
    }

    std::unique_ptr<juce::OpenGLShaderProgram::Uniform> resolution, sliderVal, vmVal, diffTexture, runTime, specTexture;

   private:
    static std::unique_ptr<juce::OpenGLShaderProgram::Uniform> createUniform(juce::OpenGLShaderProgram& shaderProgram,
                                                                             const char* uniformName)
    {
      if (juce::OpenGLExtensionFunctions::glGetUniformLocation(shaderProgram.getProgramID(), uniformName) < 0)
      {
        return nullptr;
      }

      return std::make_unique<juce::OpenGLShaderProgram::Uniform>(shaderProgram, uniformName);
    }
  };

  int64_t startTime_ = 0;

  // OpenGL Member Variables
  juce::OpenGLContext openGLContext_;
  GLuint VBO_ = 0;
  GLuint EBO_ = 0;

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

  const char* vertexShader_ = nullptr;
  const char* fragmentShader_ = nullptr;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SliderBarGL)
};
