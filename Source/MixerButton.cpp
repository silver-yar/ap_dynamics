/*
  ==============================================================================

    MixerButton.cpp
    Created: 21 Jan 2021 7:10:13pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#include "MixerButton.h"
#include "APDefines.h"

#include <JuceHeader.h>

namespace
{
  constexpr float kHandleHeight = 28.0f;
}

//==============================================================================
MixerButton::MixerButton(Ap_dynamicsAudioProcessor& p) : audioProcessor_(p) { }

MixerButton::~MixerButton() { }

void MixerButton::paint(juce::Graphics& g)
{
  constexpr int cornerRadius = 10;
  constexpr int sliderMargin = 70;
  const float halfWidth      = static_cast<float>(getWidth()) * 0.5f;
  const float halfHeight     = static_cast<float>(getHeight()) * 0.5f;
  const float thirdHeight    = static_cast<float>(getHeight()) * 0.333333f;
  float sliderWidth          = static_cast<float>(getWidth()) - sliderMargin;
  auto bounds                = juce::Rectangle<int>(60, 0, static_cast<int>(sliderWidth), getHeight());
  // Background
  g.setGradientFill(juce::ColourGradient(juce::Colours::grey, halfWidth, halfHeight, juce::Colours::darkgrey, 0, 0, true));
  g.fillRoundedRectangle(bounds.toFloat(), cornerRadius);

  // Labels
  g.setColour(juce::Colours::snow);
  g.setFont(APConstants::Gui::SYS_FONT.withHeight(24.0f));
  g.drawFittedText("dirtier", bounds.removeFromTop(static_cast<int>(thirdHeight)), juce::Justification::centred, 1);

  g.setFont(24.0f);
  g.drawLine(70, thirdHeight, static_cast<float>(getWidth() - 20), static_cast<float>(getHeight()) / 3.0f, 1);
  g.drawFittedText("dirty", bounds.removeFromTop(static_cast<int>(thirdHeight)), juce::Justification::centred, 1);
  g.drawLine(70, thirdHeight * 2, static_cast<float>(getWidth() - 20), thirdHeight * 2, 1);
  g.drawFittedText("clean", bounds, juce::Justification::centred, 1);

  // Selector Bar
  auto paramRange = audioProcessor_.apvts.getParameterRange("MIX");
  auto param = audioProcessor_.apvts.getParameter("MIX");
  auto mappedParamVal = juce::jmap(param->getValue(), paramRange.start, paramRange.end, getHeight() - kHandleHeight / 2, kHandleHeight / 2);
  auto barBounds =
      juce::Rectangle<float>(sliderWidth, kHandleHeight).withCentre(bounds.getCentre().withY(mappedParamVal).toFloat());
  g.setGradientFill(juce::ColourGradient(juce::Colours::grey.withAlpha(0.3f), barBounds.getCentreX(), barBounds.getCentreY(),
                                         juce::Colours::white.withAlpha(0.7f), barBounds.getX() - (sliderWidth * 0.3f),
                                         barBounds.getY() + 2, true));
  g.fillRoundedRectangle(barBounds, cornerRadius);
  g.setColour(juce::Colours::white.withAlpha(0.3f));
  g.drawRoundedRectangle(barBounds, cornerRadius, 1.0f);
}

void MixerButton::resized() { }

void MixerButton::mouseDown(const juce::MouseEvent& event)
{
  auto bounds = getLocalBounds();

  if (bounds.contains(event.getMouseDownPosition()))
  {
    mapMouseToValue(event.getMouseDownPosition());
  }
}

void MixerButton::mouseDrag(const juce::MouseEvent& event)
{
  auto bounds = getLocalBounds();

  if (bounds.contains(event.getMouseDownPosition()))
  {
    mapMouseToValue(event.getPosition());
  }
}

void MixerButton::mouseUp(const juce::MouseEvent& event)
{
  auto bounds = getLocalBounds();

  if (bounds.contains(event.getMouseDownPosition()))
  {
    mapMouseToValue(event.getPosition());
  }
}

void MixerButton::mapMouseToValue(const juce::Point<int>& mPoint)
{
  constexpr auto yMin  = kHandleHeight / 2.0f;
  const auto yMax      = static_cast<float>(getHeight()) - (kHandleHeight / 2.0f);

  auto mappedVal = juce::jmap(static_cast<float> (mPoint.getY()), yMin, yMax, 1.0f, 0.0f);
  auto limitedVal = juce::jlimit(0.0f, 1.0f, mappedVal);
  audioProcessor_.apvts.getParameterAsValue("MIX").setValue(limitedVal);
//  audioProcessor_.setMixValue(juce::jmap((float)pointerPos_.getY(), yMin, yMax, 1.0f, 0.0f));
}
