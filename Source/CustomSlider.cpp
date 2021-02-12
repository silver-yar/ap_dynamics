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
    sliderBarBk_.start();
    addAndMakeVisible (sliderBarBk_);
    sliderBarFr_.start();
    addAndMakeVisible (sliderBarFr_);
    addAndMakeVisible (slider);

    startTimerHz(30);
}

CustomSlider_::~CustomSlider_()
{
    stopTimer();
}

void CustomSlider_::resized()
{
    switch (sliderType_) {
        case Normal:
            boundsBk_ = Rectangle<int> (0, sliderVal_,
                                        getWidth() - 70.0f, getHeight() - sliderVal_ - 10);
            sliderBarBk_.setBounds(boundsBk_);
            slider.setBounds(getLocalBounds());
            break;
        case Invert:
            boundsBk_ = Rectangle<int> (0, 10,
                                        getWidth() - 70.0f, getHeight() - 20);
            boundsFr_ = Rectangle<int> (0, sliderVal_,
                                        getWidth() - 70.0f, getHeight() - sliderVal_ - 10);
            sliderBarBk_.setBounds(boundsBk_);
            sliderBarFr_.setBounds(boundsFr_);
            slider.setBounds(getLocalBounds());
            break;
        default:
            break;
    }
}

void CustomSlider_::timerCallback()
{
    switch (sliderType_) {
        case SliderType::Normal:
            sliderVal_ = juce::jmap(slider.getValue(), 1.0,
                                    100.0, getHeight() - 10.0,
                                    10.0);
            break;
        case SliderType::Invert:
            sliderBarFr_.setSliderValue(juce::jmap((float) slider.getValue(), (float) slider.getMinimum(),
                                                   (float) slider.getMaximum(), 0.0f,
                                                   1.0f));
            //sliderBarBk_.setMeterValue(0.75);
            //                sliderVal_ = juce::jmap(slider.getValue(), slider.getMinimum(),
            //                                        slider.getMaximum(), getHeight() - 10.0,
            //                                        10.0);
        break;
            default:
        break;
    }
    resized();
}

MyLookAndFeel::MyLookAndFeel(Ap_dynamicsAudioProcessor& p) : audioProcessor (p)
{
}

void MyLookAndFeel::drawLinearSlider(juce::Graphics &g, int x, int y, int width, int height, float sliderPos,
                                           float minSliderPos, float maxSliderPos, const juce::Slider::SliderStyle style,
                                           juce::Slider &slider) {
    bubStart_ = height;
    lastSliderPos_ = sliderPos;
    // Background
    g.setGradientFill (
            juce::ColourGradient (juce::Colours::grey, width / 2, height / 2 - 100,juce::Colours::darkgrey,0,0,true)
    );
    g.fillRoundedRectangle (x, y, width - labelMargin_, height, 10);

    // Volume Meter
    if (slider.isTextBoxEditable()) {
        auto hasClipped = Decibels::gainToDecibels(audioProcessor.meterGlobalMaxVal.load()) >= 0.0f;
        auto dbVal = Decibels::gainToDecibels(audioProcessor.meterLocalMaxVal.load(), -100.0f);
        dbVal = jlimit(-100.0f, 0.0f, dbVal);

        auto meter = Rectangle<int>(x, 10, width - labelMargin_, height - 20);

        g.setColour(Colours::black.withAlpha(0.1f));
        g.fillRect(meter);

        meter.removeFromTop(meter.getHeight() * -dbVal / 100.0f);
        auto gradient = hasClipped ? juce::ColourGradient (juce::Colour(0xFFFF2600), width / 2, height / 2,
                                                           juce::Colour(0xFFA9A9A9),width / 2, meter.getY(),false) :
                        juce::ColourGradient (juce::Colour(0xFF00FA92), width / 2, height / 2,
                                              juce::Colour(0xFFD783FF),width / 2,meter.getY(),false);
        g.setGradientFill (
                gradient
        );

        g.fillRect(meter);
    }

    // Bar
    if (slider.isBar())
    {
//        g.drawImage(bar1_, Rectangle<float> ((float) x, sliderPos > 20 ? sliderPos - 10 : 10,
//                                      (float) width - labelMargin_,
//                                      sliderPos > 20 ? (float) y + ((float) height - sliderPos) : height - 20),
//                                      juce::RectanglePlacement::fillDestination);
        Path p;
        if (slider.isTextBoxEditable()) {
            g.setGradientFill(
                    juce::ColourGradient(slider.findColour(slider.trackColourId),
                                         width / 2, height / 2,
                                         slider.findColour(slider.trackColourId).darker(0.5f),
                                         -200, -200, true)
            );
        } else {
            g.setGradientFill(
                    juce::ColourGradient(juce::Colour(0xFF00FA92),
                                         width / 2, height,
                                         juce::Colour(0xFFD783FF),
                                         width / 2, sliderPos, false)
            );
        }
        auto cornerSize = 10;
        sliderWidth_ = width - labelMargin_ + 1.0f;
        p.startNewSubPath((float) x, height - cornerSize);
        p.lineTo(sliderWidth_, height - cornerSize);
        p.quadraticTo(sliderWidth_, height + 1.0f, sliderWidth_ - cornerSize, height + 1.0f);
        p.lineTo((float) x + cornerSize, height + 1.0f);
        p.quadraticTo((float) x, height, (float) x, height - cornerSize);
        p.closeSubPath();
        g.fillPath(p);

        g.fillRect (Rectangle<float> ((float) x, sliderPos > 20 ? sliderPos - 10 : 10,
                                      (float) width - labelMargin_,
                                      sliderPos > 20 ? (float) y + ((float) height - sliderPos) : height - 20));

        // Bubbles
        if (!slider.isTextBoxEditable())
        {
            g.setColour(juce::Colours::white);
            auto bubbleBounds = Rectangle<float> (bubblePosX1_, bubblePosY1_, 20, 20);
            auto bb2 = Rectangle<float> (bubblePosX2_, bubblePosY2_, 40, 40);
            auto bb3 = Rectangle<float> (bubblePosX3_, bubblePosY3_, 10, 10);
//            auto bb4 = Rectangle<float> (50, bubblePosY1_, 20, 20);

            g.setGradientFill(
                    juce::ColourGradient(juce::Colour(0xFF00FA92).withAlpha(0.3f),
                                         bubbleBounds.getCentreX(), bubbleBounds.getCentreY(),
                                         juce::Colour(0xFFD783FF).withAlpha(0.5f),
                                         bubbleBounds.getX(), bubbleBounds.getY(), true)
            );
            g.fillEllipse (bubbleBounds);
            g.setGradientFill(
                    juce::ColourGradient(juce::Colour(0xFF00FA92).withAlpha(0.3f),
                                         bb2.getCentreX(), bb2.getCentreY(),
                                         juce::Colour(0xFFD783FF).withAlpha(0.5f),
                                         bb2.getX(), bb2.getY(), true)
            );
            g.fillEllipse (bb2);
            g.setGradientFill(
                    juce::ColourGradient(juce::Colour(0xFF00FA92).withAlpha(0.3f),
                                         bb3.getCentreX(), bb3.getCentreY(),
                                         juce::Colour(0xFFD783FF).withAlpha(0.5f),
                                         bb3.getX(), bb3.getY(), true)
            );
            g.fillEllipse (bb3);

            if (bubblePosY1_ > sliderPos)
            {
                bubblePosY1_ -= 0.8f;
            } else {
                bubblePosY1_ = bubStart_;
                bubblePosX1_ = rand() % (int) (width - labelMargin_ - 20.0f);
            }
            if (bubblePosY2_ > sliderPos)
            {
                bubblePosY2_ -= 0.5f;
            } else {
                bubblePosY2_ = bubStart_;
                bubblePosX2_ = rand() % (int) (width - labelMargin_ - 40.0f);
            }
            if (bubblePosY3_ > sliderPos)
            {
                bubblePosY3_ -= 1.0f;
            } else {
                bubblePosY3_ = bubStart_;
                bubblePosX3_ = rand() % (int) (width - labelMargin_ - 40.0f);
            }
        }
    }
}

void MyLookAndFeel::drawLinearSliderBackground(Graphics &g, int x, int y, int width, int height, float sliderPos,
                                                     float minSliderPos, float maxSliderPos,
                                                     const juce::Slider::SliderStyle style, Slider &slider) {
    g.setColour(Colours::indianred);
    g.fillRect(x, y, width, height);
}

juce::Label* MyLookAndFeel::createSliderTextBox(Slider &slider) {
    auto* l = LookAndFeel_V2::createSliderTextBox (slider);

    l->setColour (Label::textColourId, Colours::snow);
    l->setFont(labelFont_);
    l->setFont (16);
    l->setBounds(slider.getLocalBounds().removeFromTop(100));

    return l;
}

void MyLookAndFeel::drawLabel(Graphics &g, Label &label) {
    g.setColour(juce::Colours::snow);

    auto labelBounds = Rectangle<int> (sliderWidth_,
                                       lastSliderPos_ > 20 ? lastSliderPos_ - 20 : lastSliderPos_,
                                       labelMargin_,
                                       20);
    g.drawFittedText(label.getText().substring(0, 9), labelBounds,
                     label.getJustificationType(), 1);
}

