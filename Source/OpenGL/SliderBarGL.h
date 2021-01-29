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
class SliderBarGL  : public juce::Component, public juce::OpenGLRenderer, public juce::Timer
{
public:
    SliderBarGL(juce::OpenGLContext* context);
    ~SliderBarGL() override;

    void setData(std::vector<std::vector<double>> _data);
    void newOpenGLContextCreated() override;
    void openGLContextClosing() override;
    void renderOpenGL() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::OpenGLContext *openGLContext;
//    float rotation;
    juce::ScopedPointer<juce::OpenGLShaderProgram> shader;
//    ScopedPointer<Shape> shape;
//    ScopedPointer<Attributes> attributes;
//    ScopedPointer<Uniforms> uniforms;

    std::vector<std::vector<double>> data;
    std::vector<std::vector<double>> currentGrainsData;

    juce::String newVertexShader, newFragmentShader;

//    Draggable3DOrientation draggableOrientation;
    float scale;
//    float rotationSpeed;
    bool force;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SliderBarGL)
};
