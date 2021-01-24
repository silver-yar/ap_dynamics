/*
  ==============================================================================

    APToggleButton.h
    Created: 27 Dec 2020 11:55:27pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include <utility>
#include "PluginProcessor.h"

//==============================================================================
/*
*/
class ButtonChoice : public juce::Component
{
    ButtonChoice();
    ~ButtonChoice() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent& e) override;
};
class APToggleButton  : public juce::Component
{
public:
    APToggleButton();
    ~APToggleButton() override;

    enum Type {
        Boolean = 1,
        Multi = 2
    };

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent& e) override;
    void setButtonType (Type type) { buttonType_ = type; }
    void setMultiText (juce::Array<juce::String> options) { multiText_ = options; }
    void setBoolText (juce::String text) { boolText_ = std::move(text); }

    std::function<void()> onButtonClicked = nullptr;
    bool boolToggle = false;
    int intToggle = 0;
private:
    Type buttonType_ = Type::Boolean;

    juce::Font myFont_ {juce::Typeface::createSystemTypefaceFor(BinaryData::Antipasto_Med_ttf,
                                                                BinaryData::Antipasto_Med_ttfSize)};

    juce::Array<juce::String> multiText_;
    juce::String boolText_ {"Text" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (APToggleButton)
};
