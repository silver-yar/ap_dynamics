//
// Created by Johnathan Handy on 12/20/21.
//

#include "APParameterMenu.h"

#include "BinaryData.h"

APParameterMenu::APParameterMenu()
{
  initializeAssets();
  closeButton_->onClick = [this]() { setVisible(false); };
  closeButton_->setBounds(5, 5, 20, 20);
  addAndMakeVisible(closeButton_.get());
}

APParameterMenu::~APParameterMenu() { }

void APParameterMenu::paint(juce::Graphics& g)
{
  g.setColour(juce::Colours::black);
  g.fillRoundedRectangle(getLocalBounds().toFloat(), 10.0f);
}
void APParameterMenu::initializeAssets()
{
  closeIcon_ = std::make_unique<juce::DrawableImage>(
      juce::ImageCache::getFromMemory(BinaryData::close_white_png, BinaryData::close_white_pngSize));
  closeIconOver_ = std::make_unique<juce::DrawableImage>(
      juce::ImageCache::getFromMemory(BinaryData::close_white_png, BinaryData::close_white_pngSize));
  closeIconOver_->setOverlayColour(juce::Colours::red);
  closeButton_ = std::make_unique<juce::DrawableButton>("Close", juce::DrawableButton::ButtonStyle::ImageFitted);
  closeButton_->setImages(closeIcon_.get(), closeIconOver_.get());
}
