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
SliderBarGL::SliderBarGL(juce::OpenGLContext* context) : scale (0.5f), openGLContext (context), force (false)
{
    setOpaque (true);
    openGLContext->setRenderer (this);
    openGLContext->attachTo (*this);
    openGLContext->setContinuousRepainting (true);
}

SliderBarGL::~SliderBarGL()
{
    openGLContext->detach();
}

void SliderBarGL::paint (juce::Graphics& g)
{
}

void SliderBarGL::resized()
{
}
