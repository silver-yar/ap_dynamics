/*
  ==============================================================================

    MixerButton.cpp
    Created: 21 Jan 2021 7:10:13pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#include "MixerButton.h"

#include <JuceHeader.h>

namespace
{
  constexpr float kHandleHeight = 28.0f;
}

//==============================================================================
MixerButton::MixerButton(Ap_dynamicsAudioProcessor& p) : audioProcessor(p) { }

MixerButton::~MixerButton() { }

void MixerButton::paint(juce::Graphics& g)
{
  constexpr int cornerRadius = 10;
  constexpr int sliderMargin = 70;
  auto sliderWidth           = getWidth() - sliderMargin;
  auto bounds                = juce::Rectangle<int>(60, 0, sliderWidth, getHeight());
  // Background
  g.setGradientFill(
      juce::ColourGradient(juce::Colours::grey, getWidth() / 2, getHeight() / 2, juce::Colours::darkgrey, 0, 0, true));
  g.fillRoundedRectangle(bounds.toFloat(), cornerRadius);

  // Labels
  g.setColour(juce::Colours::snow);
  g.setFont(labelFont_.withHeight(24.0f));
  g.drawFittedText("dirtier", bounds.removeFromTop(getHeight() / 3), juce::Justification::centred, 1);

  // dB
  //    if (showdB) {
  //        g.setFont(8.0f);
  //        g.drawFittedText("-20 dB", juce::Rectangle<int>(70, bounds.getY() + (getHeight() / 6) - 4,
  //                                                        20, 10),
  //                         juce::Justification::centred, 1);
  //        g.drawFittedText("+20 dB", juce::Rectangle<int>(bounds.getRight() - 30, bounds.getY() + (getHeight() / 6) - 4,
  //                                                        20, 10),
  //                         juce::Justification::centred, 1);
  //    }

  g.setFont(24.0f);
  g.drawLine(70, getHeight() / 3, getWidth() - 20, getHeight() / 3, 1);
  g.drawFittedText("dirty", bounds.removeFromTop(getHeight() / 3), juce::Justification::centred, 1);
  g.drawLine(70, getHeight() / 3 * 2, getWidth() - 20, getHeight() / 3 * 2, 1);
  g.drawFittedText("clean", bounds, juce::Justification::centred, 1);

  //    auto bubbleBounds = juce::Rectangle<float> (pointerPos_.getX() - 10, pointerPos_.getY() - 10, 20, 20);

  // Selector Shadow
  //    g.setColour(juce::Colours::black.withAlpha(0.1f));
  //    g.fillEllipse(bubbleBounds.withPosition(bubbleBounds.getX(), bubbleBounds.getY() + 10));

  // Selector
  //    g.setGradientFill(juce::ColourGradient(juce::Colours::grey.withAlpha(0.3f),
  //                         bubbleBounds.getCentreX(),
  //                         bubbleBounds.getCentreY(),
  //                         juce::Colours::white,
  //                         bubbleBounds.getX(), bubbleBounds.getY(), true)
  //                         );
  //    g.fillEllipse (bubbleBounds);
  auto barBounds =
      juce::Rectangle<float>(sliderWidth, kHandleHeight).withCentre(bounds.getCentre().withY(pointerPos_.getY()).toFloat());
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
    showdB = true;
    mapMouseToValue(event.getMouseDownPosition());
  }
}

void MixerButton::mouseDrag(const juce::MouseEvent& event)
{
  auto bounds = getLocalBounds();

  if (bounds.contains(event.getMouseDownPosition()))
  {
    showdB = true;
    mapMouseToValue(event.getPosition());
  }
}

void MixerButton::mouseUp(const juce::MouseEvent& event)
{
  auto bounds = getLocalBounds();

  if (bounds.contains(event.getMouseDownPosition()))
  {
    showdB = false;
  }
}

void MixerButton::mapMouseToValue(const juce::Point<int>& mPoint)
{
  auto x_min = 70.0f;
  auto x_max = getWidth() - 20.0f;
  auto y_min = kHandleHeight / 2.0f;
  auto y_max = getHeight() - (kHandleHeight / 2.0f);

  auto x = juce::jlimit(x_min, x_max, (float)mPoint.getX());
  auto y = juce::jlimit(y_min, y_max, (float)mPoint.getY());

  pointerPos_ = juce::Point<int>(x, y);
  //    mixValue_ = juce::jmap((float) pointerPos_.getY(), y_min, y_max,
  //                           0.0f, 100.0f);
  //    gainOutdB_ = juce::jmap((float) pointerPos_.getX(), x_min, x_max, -40.0f, 40.0f);
  audioProcessor.setMixValue(juce::jmap((float)pointerPos_.getY(), y_min, y_max, 1.0f, 0.0f));
  //    audioProcessor.setOutputGain (juce::jmap((float) pointerPos_.getX(), x_min, x_max,
  //                                             -20.0f, 20.0f));
}
