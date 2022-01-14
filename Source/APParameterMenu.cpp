//
// Created by Johnathan Handy on 12/20/21.
//

#include "APParameterMenu.h"

#include "BinaryData.h"

ParameterGrid::ParameterGrid(juce::AudioProcessor& p, juce::AudioProcessorValueTreeState& s) : audioProcessor_(p), apvts_(s)
{
  initializeAssets();
}
ParameterGrid::~ParameterGrid() { }
void ParameterGrid::paint(juce::Graphics& g)
{
  g.setColour(juce::Colours::limegreen);
  g.fillAll();
}
void ParameterGrid::resized()
{
  using Fr    = juce::Grid::Fr;
  using Track = juce::Grid::TrackInfo;
  juce::Grid grid;
  grid.templateColumns = { Track(Fr(1)), Track(Fr(1)), Track(Fr(1)) };
  grid.templateRows    = { Track(Fr(1)), Track(Fr(1)), Track(Fr(1)) };
  grid.items.resize(static_cast<int>(sliders_.size()));
  std::transform(sliders_.begin(), sliders_.end(), grid.items.begin(),
                 [](auto& slider) { return juce::GridItem(*slider.first); });
  grid.performLayout(getLocalBounds());
}
void ParameterGrid::initializeAssets()
{
  auto all_parameters = audioProcessor_.getParameters();

  std::for_each(all_parameters.begin() + 2, all_parameters.end(),
                [this](auto parameter)
                {
                  const auto* cast_param = dynamic_cast<const juce::RangedAudioParameter*>(parameter);
                  auto slider =
                      std::make_unique<juce::Slider>(juce::Slider::SliderStyle::LinearBar, juce::Slider::TextBoxAbove);
                  auto attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                      apvts_, cast_param->getParameterID(), *slider);
                  addAndMakeVisible(slider.get());
                  sliders_.emplace_back(std::move(slider), std::move(attachment));
                });
}

APParameterMenu::APParameterMenu(juce::AudioProcessor& p, juce::AudioProcessorValueTreeState& s)
    : audioProcessor_(p), apvts_(s)
{
  initializeAssets();
  closeButton_->onClick = [this]() { setVisible(false); };
  closeButton_->setBounds(5, 5, 20, 20);
  addAndMakeVisible(closeButton_.get());

  addAndMakeVisible(parameterGrid_.get());
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
  parameterGrid_ = std::make_unique<ParameterGrid>(audioProcessor_, apvts_);
}

void APParameterMenu::resized() { parameterGrid_->setBounds(getLocalBounds().reduced(10)); }
