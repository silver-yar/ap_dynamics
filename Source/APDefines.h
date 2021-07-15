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

// Breakout into two files for graphics and logic constants
namespace AP
{
  namespace Colors
  {
    extern const juce::Colour DarkGrey;
  }

  namespace Constants
  {
    constexpr float minusInfinityDb = -96.0f;
  }
}  // namespace AP
