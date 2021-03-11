/*
  ==============================================================================

    CustomSlider.cpp
    Created: 20 Jan 2021 4:38:04pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#include "CustomSlider.h"

using namespace juce;

CustomSlider::CustomSlider()
{
}

CustomSlider::~CustomSlider()
{

}


CustomSlider_::CustomSlider_(Ap_dynamicsAudioProcessor& p, SliderType sliderType) : audioProcessor (p), sliderType_ (sliderType)
{
    ratioSliderBar_.start();
    addAndMakeVisible (ratioSliderBar_);
    threshSliderBar_.start();
    addAndMakeVisible (threshSliderBar_);
    addAndMakeVisible (slider);

    startTimerHz(30);
}

CustomSlider_::~CustomSlider_()
{
    ratioSliderBar_.stop();
    threshSliderBar_.stop();
    stopTimer();
}

void CustomSlider_::resized()
{
    auto offset = 72.0f;
    switch (sliderType_) {
        case Normal:
            ratioBounds_ = Rectangle<int> (1, 10,
                                        getWidth() - offset, getHeight() - 20);
            ratioSliderBar_.setBounds(ratioBounds_);
            slider.setBounds(getLocalBounds());
            break;
        case Invert:
//            ratioBounds_ = Rectangle<int> (1, 10,
//                                        getWidth() - offset, getHeight() - 20);
            threshBounds_ = Rectangle<int> (1, 10,
                                        getWidth() - offset, getHeight() - 20);
//            ratioSliderBar_.setBounds(ratioBounds_);
            threshSliderBar_.setBounds(threshBounds_);
            slider.setBounds(getLocalBounds());
            break;
        default:
            break;
    }
}

void CustomSlider_::timerCallback()
{
    auto gaindB = juce::Decibels::gainToDecibels(audioProcessor.meterLocalMaxVal.load(), -96.0f);

    switch (sliderType_) {
        case SliderType::Normal: {
            const float sliderPos = slider.getPositionOfValue(slider.getValue());
            const float sliderValue = juce::jmap(sliderPos,
                                                 (float) slider.getHeight(),
                                                 0.0f,
                                                 0.0f,
                                                 1.0f);
            ratioSliderBar_.setSliderValue(sliderValue);
        }
            break;
        case SliderType::Invert:
            threshSliderBar_.setSliderValue(juce::jmap((float) slider.getValue(), (float) slider.getMinimum(),
                                                       (float) slider.getMaximum(), 0.0f,
                                                       1.0f));
            threshSliderBar_.setMeterValue(juce::jmap(gaindB, -96.0f, 0.0f, 0.0f, 1.0f));
            break;
        default:
            break;
    }
    resized();
}

MyLookAndFeel::MyLookAndFeel(Ap_dynamicsAudioProcessor& p, SliderType sliderType) : audioProcessor (p), sliderType_ (sliderType)
{
}

void MyLookAndFeel::drawLinearSlider(juce::Graphics &g, int x, int y, int width, int height, float sliderPos,
                                           float minSliderPos, float maxSliderPos, const juce::Slider::SliderStyle style,
                                           juce::Slider &slider) {
    lastSliderPos_ = sliderPos;
    sliderWidth_ = width - labelMargin_ + 1.0f;
    // Background
    g.setColour (juce::Colours::darkgrey);
    g.fillRoundedRectangle (x, y, width - labelMargin_, height, CustomSlider_::cornerSize);
}

void MyLookAndFeel::drawLinearSliderBackground(Graphics &g, int x, int y, int width, int height, float sliderPos,
                                                     float minSliderPos, float maxSliderPos,
                                                     const juce::Slider::SliderStyle style, Slider &slider) {
    g.setColour(Colours::indianred);
    g.fillRect(x, y, width, height);
}

juce::Label* MyLookAndFeel::createSliderTextBox(Slider &slider) {
    auto* l = LookAndFeel_V2::createSliderTextBox (slider);

//    l->setColour (Label::textColourId, Colours::snow);
//    l->setFont(labelFont_);
//    l->setFont (16);
//    l->setBounds(slider.getLocalBounds().removeFromTop(100));

    return l;
}

void MyLookAndFeel::drawLabel(Graphics &g, Label &label) {
    g.setColour(juce::Colours::snow);
    g.setFont(labelFont_.withHeight(16.0f));

    Rectangle<int> labelBounds;

    switch (sliderType_) {
        case Normal:
            //DBG("lastSliderPos: " << lastSliderPos_);
            labelBounds = Rectangle<int> (sliderWidth_,
                                               lastSliderPos_,
                                               labelMargin_,
                                               20);
            break;
        case Invert:
            labelBounds = Rectangle<int> (sliderWidth_,
                                               lastSliderPos_ > 20 ? lastSliderPos_ - 20 : lastSliderPos_,
                                               labelMargin_,
                                               20);
            break;
        default:
            break;
    }
    g.drawFittedText(label.getText().substring(0, 9), labelBounds,
                     label.getJustificationType(), 1);
}

