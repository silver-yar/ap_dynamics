/*
  ==============================================================================

    CustomSlider.cpp
    Created: 20 Jan 2021 4:38:04pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#include "APSlider.h"

#include "../Helpers/APDefines.h"

APSlider::APSlider(Ap_dynamicsAudioProcessor &p, SliderType sliderType) : audioProcessor(p), sliderType_(sliderType)
{
  lookAndFeel_ = std::make_unique<MainSliderLookAndFeel>();

  switch (sliderType)
  {
    case Invert: sliderBarGl_ = std::make_unique<SliderBarGL>("liquidmetal.shader"); break;
    case Normal: sliderBarGl_ = std::make_unique<SliderBarGL>("basic.shader");
    default: break;
  }

  sliderBarGl_->start();
  addAndMakeVisible(sliderBarGl_.get());
  slider.setLookAndFeel(lookAndFeel_.get());
  addAndMakeVisible(slider);

  startTimerHz(30);
}

APSlider::~APSlider()
{
  sliderBarGl_->stop();
  slider.setLookAndFeel(nullptr);
  stopTimer();
}

void APSlider::resized()
{
  constexpr auto offset = 72;
  sliderBarGl_->setBounds(Rectangle<int>(1, 10, getWidth() - offset, getHeight() - 20));
  slider.setBounds(getLocalBounds());
}

void APSlider::timerCallback()
{
  const auto gain_dB =
      juce::Decibels::gainToDecibels(audioProcessor.meterLocalMaxVal.load(), APConstants::Math::MINUS_INF_DB);
  constexpr auto target_range_min = 0.0f;
  constexpr auto target_range_max = 1.0f;
  constexpr auto source_range_max = 0.0f;

  switch (sliderType_)
  {
    case SliderType::Normal:
    {
      const float sliderPos   = slider.getPositionOfValue(slider.getValue());
      const float sliderValue = juce::jmap(sliderPos, static_cast<float>(slider.getHeight()), source_range_max,
                                           target_range_min, target_range_max);
      sliderBarGl_->setSliderValue(sliderValue);
    }
    break;
    case SliderType::Invert:
      sliderBarGl_->setSliderValue(juce::jmap(static_cast<float>(slider.getValue()), static_cast<float>(slider.getMinimum()),
                                              static_cast<float>(slider.getMaximum()), target_range_min, target_range_max));
      sliderBarGl_->setMeterValue(
          juce::jmap(gain_dB, APConstants::Math::MINUS_INF_DB, source_range_max, target_range_min, target_range_max));
      break;
    default: break;
  }
  resized();
}


