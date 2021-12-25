//
// Created by Johnathan Handy on 12/20/21.
//

#pragma once
#include "juce_gui_basics/juce_gui_basics.h"

class APParameterMenu : public juce::Component
{
 public:
  APParameterMenu();
  ~APParameterMenu();

  void paint(juce::Graphics& g) override;
  void initializeAssets();
 private:
  std::unique_ptr<juce::DrawableImage> closeIcon_;
  std::unique_ptr<juce::DrawableImage> closeIconOver_;
  std::unique_ptr<juce::DrawableButton> closeButton_;
};

