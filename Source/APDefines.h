/*
  ==============================================================================

    APDefines.h
    Created: 11 Mar 2021 8:51:36pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#pragma once
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>

namespace APConstants
{
  namespace Colors
  {
    //    extern const juce::Colour DarkGrey;
    static const juce::Colour DarkGrey          = juce::Colours::darkgrey;
    static const juce::Colour SHADOW_COLOR      = juce::Colours::black.withAlpha(0.2f);
    static const juce::Colour INNER_GRADIENT_BG = juce::Colour(0xFFFFDC93);
    static const juce::Colour OUTER_GRADIENT_BG = juce::Colour(0xFFFFC446);
  }  // namespace Colors

  namespace Gui
  {
    inline constexpr int M_HEIGHT             = 500;
    inline constexpr int M_WIDTH              = 700;
    inline constexpr int SLIDER_Y             = 290;
    inline constexpr int SLIDER_WIDTH         = 200;
    inline constexpr float FONT_HEIGHT        = 24.0f;
    inline constexpr float SHADOW_FONT_HEIGHT = 28.0f;
  }  // namespace Gui

  namespace Math
  {
    inline constexpr float minusInfinityDb = -96.0f;
  }
}  // namespace APConstants
