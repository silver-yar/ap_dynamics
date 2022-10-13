//
// Created by Johnathan Handy on 2/22/22.
//

#include "APLookAndFeel.h"

juce::Label* MenuLookAndFeel::createSliderTextBox(juce::Slider& slider)
{
  auto *l = juce::LookAndFeel_V4::createSliderTextBox(slider);
  l->setFont(APConstants::Gui::SYS_FONT);

  return l;
}

MainSliderLookAndFeel::MainSliderLookAndFeel() { initializeAssets(); }

void MainSliderLookAndFeel::drawLinearSlider(juce::Graphics &g, int x, int y, int width, int height, float sliderPos,
                                             float minSliderPos, float maxSliderPos, const juce::Slider::SliderStyle style,
                                             juce::Slider &slider)
{
  // Unused Parameters
  juce::ignoreUnused(minSliderPos);
  juce::ignoreUnused(maxSliderPos);
  juce::ignoreUnused(style);
  juce::ignoreUnused(slider);

  lastSliderPos_ = static_cast<int>(sliderPos);
  sliderWidth_   = width - APConstants::Gui::SLIDER_LABEL_MARGIN + 1;
  // Background
  g.setColour(APConstants::Colors::DARK_GREY);
  g.fillRoundedRectangle(static_cast<float>(x), static_cast<float>(y),
                         static_cast<float>(width - APConstants::Gui::SLIDER_LABEL_MARGIN), static_cast<float>(height),
                         APConstants::Gui::CORNER_SIZE);
}

juce::Label *MainSliderLookAndFeel::createSliderTextBox(juce::Slider &slider)
{
  auto *l = juce::LookAndFeel_V4::createSliderTextBox(slider);

  return l;
}

void MainSliderLookAndFeel::drawLabel(juce::Graphics &g, juce::Label &label)
{
  const auto justification = juce::Justification::centred;
  const auto font = APConstants::Gui::SYS_FONT.withHeight(APConstants::Gui::LABEL_FONT_HEIGHT);
  const auto transparent = juce::Colours::transparentBlack;
  const auto name         = label.getText();
  const auto slider_value = getLabelText == nullptr ? name : getLabelText();

  const auto labelBounds = juce::Rectangle<int>(sliderWidth_, lastSliderPos_ > 20 ? lastSliderPos_ - 20 : lastSliderPos_,
                                          APConstants::Gui::SLIDER_LABEL_MARGIN, 20);

  const auto shadowBounds = labelBounds.withX(labelBounds.getX() + 11).withY(labelBounds.getY() - 10).toFloat();

  kernel_->createGaussianBlur(APConstants::Gui::BLUR_RADIUS_LABEL);

  if (shadow_ == nullptr)
  {
    shadow_ = std::make_unique<juce::Image>(juce::Image::PixelFormat::ARGB, label.getWidth(),
                                            (int)APConstants::Gui::SHADOW_FONT_HEIGHT, true);

    juce::Graphics graphics(*shadow_);
    graphics.setColour(APConstants::Colors::SHADOW_COLOR);
    graphics.setFont(APConstants::Gui::SYS_FONT.withHeight(APConstants::Gui::LABEL_SHADOW_FONT_HEIGHT));
    graphics.drawText(name, 0, 0, shadow_->getWidth(), shadow_->getHeight(), juce::Justification::centred, false);
    kernel_->applyToImage(*shadow_, *shadow_, shadow_->getBounds());
  }
  g.drawImage(*shadow_, shadowBounds, juce::RectanglePlacement::fillDestination);

  auto* editor = label.getCurrentTextEditor();
  g.setColour(editor == nullptr ? APConstants::Colors::DARK_GREY : transparent);
  g.setFont(APConstants::Gui::SYS_FONT.withHeight(APConstants::Gui::LABEL_FONT_HEIGHT));
  g.drawFittedText(slider_value, labelBounds, justification, 1);

  if (editor != nullptr) {
    editor->setBounds(labelBounds);
    editor->setBorder(juce::BorderSize<int> {0,1,0,0});
    editor->applyFontToAllText(font);
    editor->setJustification(justification);
    editor->setColour(juce::TextEditor::backgroundColourId, transparent);
    editor->setColour(juce::TextEditor::focusedOutlineColourId, transparent);
    editor->setColour(juce::TextEditor::textColourId, juce::Colours::cadetblue);
    editor->setColour(juce::TextEditor::highlightColourId, juce::Colours::cadetblue);
    editor->setInputRestrictions(6, "-.1234567890");
  }

}
void MainSliderLookAndFeel::initializeAssets()
{
  constexpr auto kernelSize = 16;
  kernel_                   = std::make_unique<juce::ImageConvolutionKernel>(kernelSize);
}
