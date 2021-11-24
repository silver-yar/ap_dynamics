/*
  ==============================================================================

    CustomSlider.cpp
    Created: 20 Jan 2021 4:38:04pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#include "APSlider.h"
#include "APDefines.h"

APSlider::APSlider(Ap_dynamicsAudioProcessor &p, SliderType sliderType) : audioProcessor(p), sliderType_(sliderType)
{
  lookAndFeel_ = std::make_unique<MyLookAndFeel>(sliderType_);

//  auto offset = 72;

  switch (sliderType)
  {
    case Invert:
      sliderBarGl_ = std::make_unique<SliderBarGL>("liquidmetal.shader");
//      openGlBounds_ = Rectangle<int>(1, 10, getWidth() - offset, getHeight() - 20);
      break;
    case Normal:
      sliderBarGl_ = std::make_unique<SliderBarGL>("basic.shader");
//      openGlBounds_ = Rectangle<int>(1, 10, getWidth() - offset, getHeight() - 20);
    default: break;
  }

  //  ratioSliderBar_  = std::make_unique<SliderBarGL>("basic.shader");
  sliderBarGl_->start();
  addAndMakeVisible(sliderBarGl_.get());
//  sliderBarGl_->setBounds(openGlBounds_);
  //  ratioSliderBar_->start();
  //  addAndMakeVisible(ratioSliderBar_.get());
  slider.setLookAndFeel(lookAndFeel_.get());
  addAndMakeVisible(slider);

  startTimerHz(30);
}

APSlider::~APSlider()
{
//  ratioSliderBar_->stop();
  sliderBarGl_->stop();
  slider.setLookAndFeel(nullptr);
  stopTimer();
}

void APSlider::resized()
{
  auto offset = 72;
  openGlBounds_ = Rectangle<int>(1, 10, getWidth() - offset, getHeight() - 20);
  sliderBarGl_->setBounds(openGlBounds_);
  slider.setBounds(getLocalBounds());
}

void APSlider::timerCallback()
{
  auto gain_dB = juce::Decibels::gainToDecibels(audioProcessor.meterLocalMaxVal.load(), APConstants::Math::minusInfinityDb);
  auto target_range_min = 0.0f;
  auto target_range_max = 1.0f;
  auto source_range_max = 0.0f;

  switch (sliderType_)
  {
    case SliderType::Normal:
    {
      const float sliderPos   = slider.getPositionOfValue(slider.getValue());
      const float sliderValue = juce::jmap(sliderPos, (float)slider.getHeight(), source_range_max, target_range_min, target_range_max);
      sliderBarGl_->setSliderValue(sliderValue);
    }
    break;
    case SliderType::Invert:
      sliderBarGl_->setSliderValue(
          juce::jmap((float)slider.getValue(), (float)slider.getMinimum(), (float)slider.getMaximum(), target_range_min, target_range_max));
      sliderBarGl_->setMeterValue(juce::jmap(gain_dB, APConstants::Math::minusInfinityDb, source_range_max, target_range_min, target_range_max));
      break;
    default: break;
  }
  resized();
}

MyLookAndFeel::MyLookAndFeel(SliderType sliderType)
    : sliderType_(sliderType)
{
}

void MyLookAndFeel::drawLinearSlider(juce::Graphics &g, int x, int y, int width, int height, float sliderPos,
                                     float minSliderPos, float maxSliderPos, const juce::Slider::SliderStyle style,
                                     juce::Slider &slider)
{
  // Unused Parameters
  ignoreUnused(minSliderPos);
  ignoreUnused(maxSliderPos);
  ignoreUnused(style);
  ignoreUnused(slider);

  lastSliderPos_ = static_cast<int>(sliderPos);
  sliderWidth_   = width - labelMargin_ + 1;
  // Background
  g.setColour(APConstants::Colors::DarkGrey);
  g.fillRoundedRectangle(static_cast<float>(x), static_cast<float>(y), static_cast<float>(width - labelMargin_),
                         static_cast<float>(height), APSlider::cornerSize);
}

void MyLookAndFeel::drawLinearSliderBackground(Graphics &g, int x, int y, int width, int height, float sliderPos,
                                               float minSliderPos, float maxSliderPos, const juce::Slider::SliderStyle style,
                                               Slider &slider)
{
  // Unused Parameters
  ignoreUnused(sliderPos);
  ignoreUnused(minSliderPos);
  ignoreUnused(maxSliderPos);
  ignoreUnused(style);
  ignoreUnused(slider);

  g.setColour(Colours::indianred);
  g.fillRect(x, y, width, height);
}

juce::Label *MyLookAndFeel::createSliderTextBox(Slider &slider)
{
  auto *l = juce::LookAndFeel_V4::createSliderTextBox(slider);

  return l;
}

void MyLookAndFeel::drawLabel(Graphics &g, Label &label)
{
  Rectangle<int> labelBounds;

  switch (sliderType_)
  {
    case Normal: labelBounds = Rectangle<int>(sliderWidth_, lastSliderPos_, labelMargin_, 20); break;
    case Invert:
      labelBounds =
          Rectangle<int>(sliderWidth_, lastSliderPos_ > 20 ? lastSliderPos_ - 20 : lastSliderPos_, labelMargin_, 20);
      break;
    default: break;
  }

  const auto name   = label.getText();
  auto shadowBounds = labelBounds.withX(labelBounds.getX() + 11).withY(labelBounds.getY() - 10).toFloat();

  kernel_.createGaussianBlur(3.2f);

  shadow_ = juce::Image(juce::Image::PixelFormat::ARGB, label.getWidth(), (int)APConstants::Gui::SHADOW_FONT_HEIGHT, true);
  juce::Graphics graphics(shadow_);
  graphics.setColour(APConstants::Colors::SHADOW_COLOR);
  graphics.setFont(labelFont_.withHeight(20.0f));
  graphics.drawText(name, 0, 0, shadow_.getWidth(), shadow_.getHeight(), juce::Justification::centred, false);
  kernel_.applyToImage(shadow_, shadow_, shadow_.getBounds());
  g.drawImage(shadow_, shadowBounds, juce::RectanglePlacement::fillDestination);

  g.setColour(APConstants::Colors::DarkGrey);
  g.setFont(labelFont_.withHeight(16.0f));
  g.drawFittedText(label.getText().substring(0, 9), labelBounds, juce::Justification::centredRight, 1);
}
