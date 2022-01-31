//
// Created by Johnathan Handy on 12/20/21.
//

#include "APParameterMenu.h"

#include "../Helpers/APDefines.h"
#include "BinaryData.h"

void APParameterMenu::ParameterGrid::paint(juce::Graphics& g) { juce::ignoreUnused(g); }
void APParameterMenu::ParameterGrid::resized()
{
  using Fr    = juce::Grid::Fr;
  using Track = juce::Grid::TrackInfo;
  juce::Grid grid;



  grid.templateColumns = { Track(Fr(1)), Track(Fr(2)) };
  //  grid.templateRows    = { Track(Fr(1)), Track(Fr(1)), Track(Fr(1)) };
  grid.autoRows = Track(Fr(1));
  for (const auto& slider : sliders_)
  {
    grid.items.add(GridItem(*slider.label));
    grid.items.add(GridItem(*slider.slider));
  }

  grid.performLayout(getLocalBounds());
}
void APParameterMenu::ParameterGrid::initializeAssets()
{
  const auto predicate = [](juce::AudioParameterFloat* param) -> bool
  {
    const std::vector<juce::String> idsToKeep{ "gain", "distortion", /* bass */ };

    const auto* cast_param = dynamic_cast<const juce::RangedAudioParameter*>(param);
    if (cast_param != nullptr)
    {
      const auto id = cast_param->getParameterID();

      return std::any_of(idsToKeep.begin(), idsToKeep.end(), [&id](const auto& idToKeep) { return id == idToKeep; });
    }

    return false;
  };
  auto all_parameters = audioProcessor_.getParameters();
  std::for_each(all_parameters.begin(), all_parameters.end(),
                [this](auto parameter)
                {
                  if (parameterFilter(parameter))
                  {

                  }
                  SliderObject new_slider;
                  const auto* cast_param = dynamic_cast<const juce::RangedAudioParameter*>(parameter);
                  new_slider.slider =
                      std::make_unique<juce::Slider>(juce::Slider::SliderStyle::LinearBar, juce::Slider::TextBoxAbove);
                  new_slider.sliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                      apvts_, cast_param->getParameterID(), *new_slider.slider);
                  new_slider.label = std::make_unique<juce::Label>(cast_param->name, cast_param->name);
                  new_slider.label->setFont(APConstants::Gui::SYS_FONT.withHeight(APConstants::Gui::FONT_HEIGHT));
                  addAndMakeVisible(new_slider.label.get());
                  addAndMakeVisible(new_slider.slider.get());

                  sliders_.emplace_back(std::move(new_slider));
                });
}

APParameterMenu::APParameterMenu(juce::AudioProcessor& p, juce::AudioProcessorValueTreeState& s)
    : audioProcessor_(p), apvts_(s)
{
  initializeAssets();
  closeButton_->onClick = [this]() { setVisible(false); };
  closeButton_->setBounds(5, 5, 20, 20);

  setViewedComponent(parameterGrid_.get());
  setViewPositionProportionately(1.0, 1.0);
  addAndMakeVisible(closeButton_.get());
}

APParameterMenu::~APParameterMenu() { }

void APParameterMenu::paint(juce::Graphics& g)
{
  if (backgroundImage_)
  {
    g.drawImage(*backgroundImage_, getLocalBounds().toFloat(), juce::RectanglePlacement::fillDestination);
  }
  else
  {
    constexpr auto bg_alpha = 0.6f;
    g.setColour(juce::Colours::black.withAlpha(bg_alpha));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 10.0f);
  }
}

void APParameterMenu::setBackgroundImage(const Image& backgroundImage)
{
  backgroundImage_ = std::make_unique<juce::Image>(backgroundImage);
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

void APParameterMenu::resized() {
  jassert(parameterGrid_->parameterFilter != nullptr);
  auto all_parameters = audioProcessor_.getParameters();
  constexpr float num_vis_sliders      = 3.5f;
  constexpr float slider_height_scalar = 1.0f / num_vis_sliders;
  auto slider_height                   = static_cast<int>(static_cast<float>(getLocalBounds().getHeight()) * slider_height_scalar);

  const int sum = static_cast<int>(std::count_if(all_parameters.begin(), all_parameters.end(),parameterGrid_->parameterFilter));

  parameterGrid_->setBounds(getLocalBounds().withHeight(sum * slider_height));

}
