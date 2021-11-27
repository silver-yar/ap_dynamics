/*
  ==============================================================================

    MixerButton.cpp
    Created: 21 Jan 2021 7:10:13pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#include "MixerButton.h"

#include <JuceHeader.h>

#include "APDefines.h"

namespace
{
  constexpr auto kHandleHeight = 28.0f;
}

//==============================================================================
MixerButton::MixerButton(Ap_dynamicsAudioProcessor& p) : audioProcessor_(p) { }

MixerButton::~MixerButton() = default;

void MixerButton::paint(juce::Graphics& g)
{
  constexpr auto cornerRadius = 10;
  constexpr auto sliderMargin = 70;
  constexpr auto lineThickness = 1;
  constexpr auto maxNumLines = 1;
  const auto halfWidth      = static_cast<float>(getWidth()) * 0.5f;
  const auto halfHeight     = static_cast<float>(getHeight()) * 0.5f;
  const auto thirdHeight    = static_cast<float>(getHeight()) * 0.333333f;
  const auto sliderWidth    = static_cast<float>(getWidth()) - sliderMargin;
  auto bounds                = juce::Rectangle<int>(60, 0, static_cast<int>(sliderWidth), getHeight());
  // Background
  g.setGradientFill(juce::ColourGradient(juce::Colours::grey, halfWidth, halfHeight, juce::Colours::darkgrey, 0, 0, true));
  g.fillRoundedRectangle(bounds.toFloat(), cornerRadius);

  // Labels
  g.setColour(juce::Colours::snow);
  g.setFont(APConstants::Gui::SYS_FONT.withHeight(APConstants::Gui::FONT_HEIGHT));
  g.drawFittedText("dirtier", bounds.removeFromTop(static_cast<int>(thirdHeight)), juce::Justification::centred, maxNumLines);

  g.drawLine(sliderMargin, thirdHeight, static_cast<float>(getWidth() - 20), static_cast<float>(getHeight()) / 3.0f, lineThickness);
  g.drawFittedText("dirty", bounds.removeFromTop(static_cast<int>(thirdHeight)), juce::Justification::centred, maxNumLines);
  g.drawLine(sliderMargin, thirdHeight * 2, static_cast<float>(getWidth() - 20), thirdHeight * 2, lineThickness);
  g.drawFittedText("clean", bounds, juce::Justification::centred, maxNumLines);

  // Selector Bar
  constexpr auto alphaOne = 0.3f;
  constexpr auto alphaTwo = 0.7f;
  constexpr auto halfHandleHeight = kHandleHeight / 2;
  const auto paramRange     = audioProcessor_.apvts.getParameterRange("MIX");
  const auto param          = audioProcessor_.apvts.getParameter("MIX");
  const auto mappedParamVal = juce::jmap(param->getValue(), paramRange.start, paramRange.end,
                                         static_cast<float>(getHeight()) - halfHandleHeight, halfHandleHeight);
  const auto barBounds      = juce::Rectangle<float>(sliderWidth, kHandleHeight)
                             .withCentre(bounds.getCentre().withY(static_cast<int>(mappedParamVal)).toFloat());
  g.setGradientFill(juce::ColourGradient(juce::Colours::grey.withAlpha(alphaOne), barBounds.getCentreX(), barBounds.getCentreY(),
                                         juce::Colours::white.withAlpha(alphaTwo), barBounds.getX() - (sliderWidth * 0.3f),
                                         barBounds.getY() + 2, true));
  g.fillRoundedRectangle(barBounds, cornerRadius);
  g.setColour(juce::Colours::white.withAlpha(alphaOne));
  g.drawRoundedRectangle(barBounds, cornerRadius, lineThickness);
}

void MixerButton::resized() { }

void MixerButton::mouseDown(const juce::MouseEvent& event)
{
  const auto bounds = getLocalBounds();

  if (bounds.contains(event.getMouseDownPosition()))
  {
    mapMouseToValue(event.getMouseDownPosition());
  }
}

void MixerButton::mouseDrag(const juce::MouseEvent& event)
{
  const auto bounds = getLocalBounds();

  if (bounds.contains(event.getMouseDownPosition()))
  {
    mapMouseToValue(event.getPosition());
  }
}

void MixerButton::mouseUp(const juce::MouseEvent& event)
{
  const auto bounds = getLocalBounds();

  if (bounds.contains(event.getMouseDownPosition()))
  {
    mapMouseToValue(event.getPosition());
  }
}

void MixerButton::mapMouseToValue(const juce::Point<int>& mPoint)
{
  constexpr auto yMin = kHandleHeight / 2.0f;
  const auto yMax     = static_cast<float>(getHeight()) - (kHandleHeight / 2.0f);

  const auto mappedVal  = juce::jmap(static_cast<float>(mPoint.getY()), yMin, yMax, 1.0f, 0.0f);
  const auto limitedVal = juce::jlimit(0.0f, 1.0f, mappedVal);
  audioProcessor_.apvts.getParameterAsValue("MIX").setValue(limitedVal);
}
