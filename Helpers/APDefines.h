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

#include "BinaryData.h"

namespace APConstants
{
  namespace Colors
  {
    //    extern const juce::Colour DARK_GREY;
    static const juce::Colour DARK_GREY         = juce::Colours::darkgrey;
    static const juce::Colour SHADOW_COLOR      = juce::Colours::black.withAlpha(0.2f);
    static const juce::Colour INNER_GRADIENT_BG = juce::Colour(0xFFFFDC93);
    static const juce::Colour OUTER_GRADIENT_BG = juce::Colour(0xFFFFC446);
  }  // namespace Colors

  namespace Gui
  {
    inline constexpr int M_HEIGHT                   = 500;
    inline constexpr int M_WIDTH                    = 700;
    inline constexpr int MENU_WIDTH                 = 630;
    inline constexpr int SLIDER_Y                   = 290;
    inline constexpr int SLIDER_WIDTH               = 200;
    inline constexpr int MENU_BUTTON_SIZE = 50;
    inline constexpr float FONT_HEIGHT              = 24.0f;
    inline constexpr float SHADOW_FONT_HEIGHT       = 28.0f;
    inline constexpr float LABEL_FONT_HEIGHT        = 16.0f;
    inline constexpr float LABEL_SHADOW_FONT_HEIGHT = 20.0f;
    inline constexpr int SLIDER_LABEL_MARGIN        = 70;
    inline constexpr float CORNER_SIZE              = 10.0f;
    inline constexpr float BLUR_RADIUS_LABEL        = 3.2f;
    inline constexpr float BLUR_RADIUS_LOGO         = 5.6f;
    inline const juce::Font SYS_FONT =
        juce::Font(juce::Typeface::createSystemTypefaceFor(BinaryData::VarelaRound_ttf, BinaryData::VarelaRound_ttfSize));
  }  // namespace Gui

  namespace Math
  {
    inline constexpr float MINUS_INF_DB = -96.0f;
  }
}  // namespace APConstants

namespace APParameters
{
  inline constexpr auto THRESHOLD_ID       = "THR";
  inline constexpr auto THRESHOLD_NAME     = "Threshold";
  inline constexpr auto THRESHOLD_SUFFIX   = "dBFS";
  inline constexpr auto THRESHOLD_START    = -96.0f;
  inline constexpr auto THRESHOLD_END      = 0.0f;
  inline constexpr auto THRESHOLD_INTERVAL = 0.1f;
  inline constexpr auto THRESHOLD_DEFAULT  = 0.0f;

  inline constexpr auto RATIO_ID       = "RAT";
  inline constexpr auto RATIO_NAME     = "Ratio";
  inline constexpr auto RATIO_SUFFIX   = ": 1";
  inline constexpr auto RATIO_START    = 1.0f;
  inline constexpr auto RATIO_END      = 100.0f;
  inline constexpr auto RATIO_INTERVAL = 0.1f;
  inline constexpr auto RATIO_SKEW     = 0.3f;
  inline constexpr auto RATIO_DEFAULT  = 1.0f;

  inline constexpr auto MIX_ID       = "MIX";
  inline constexpr auto MIX_NAME     = "Global Mix";
  inline constexpr auto MIX_SUFFIX   = "";
  inline constexpr auto MIX_START    = 0.0f;
  inline constexpr auto MIX_END      = 1.0f;
  inline constexpr auto MIX_INTERVAL = 0.01f;
  inline constexpr auto MIX_DEFAULT  = 0.0f;

  inline constexpr auto DISTQ_ID       = "DSQ";
  inline constexpr auto DISTQ_NAME     = "Distortion Q";
  inline constexpr auto DISTQ_SUFFIX   = "";
  inline constexpr auto DISTQ_START    = -1.0f;
  inline constexpr auto DISTQ_END      = 1.0f;
  inline constexpr auto DISTQ_INTERVAL = 0.1f;
  inline constexpr auto DISTQ_DEFAULT  = 0.0f;

  inline constexpr auto MAKEUP_ID       = "MUP";
  inline constexpr auto MAKEUP_NAME     = "Makeup";
  inline constexpr auto MAKEUP_SUFFIX   = "dB";
  inline constexpr auto MAKEUP_START    = -30.0f;
  inline constexpr auto MAKEUP_END      = 30.0f;
  inline constexpr auto MAKEUP_INTERVAL = 0.1f;
  inline constexpr auto MAKEUP_DEFAULT  = 0.0f;
}  // namespace APParameters
