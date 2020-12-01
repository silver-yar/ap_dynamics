/*
  ==============================================================================

    APPlot.cpp
    Created: 30 Nov 2020 7:31:17pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#include <JuceHeader.h>
#include "APPlot.h"

//==============================================================================
APPlot::APPlot()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

APPlot::~APPlot()
{
}

void APPlot::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colour(0xff003232));
    auto margin = 20;
    auto graphWidth = getLocalBounds().getHeight() - (margin * 2);
    auto graphBounds = juce::Rectangle<int> (margin, margin, graphWidth, graphWidth)
            .withCentre(getLocalBounds().getCentre());
    g.fillRect(graphBounds);
    g.setColour(juce::Colour(0xff00a9a9));

    drawGrid(g, graphBounds);
    drawXAxis(g, graphBounds);
    drawYAxis(g, graphBounds);
}

void APPlot::resized()
{

}

void APPlot::drawGrid(juce::Graphics& g, juce::Rectangle<int> graphBounds)
{
    // TODO: Fix to work with resizable window
    auto bounds = graphBounds.withCentre(getLocalBounds().getCentre());
    auto width = bounds.getWidth();
    g.setColour(juce::Colour(0xff00a9a9).withAlpha(0.5f));
    g.drawRect(bounds);
    auto gridSize = 0.05f;
    auto margin = 20;
    for (auto i = 1; i < (1 / gridSize); ++i)
    {
        auto vline = juce::Rectangle<float> (width * (i * gridSize) + margin, margin,
                                            1, bounds.getHeight());
        g.fillRect(vline);
        auto hline = juce::Rectangle<float> (margin, bounds.getHeight() * (i * gridSize) + margin,
                                             bounds.getWidth(), 1);
        g.fillRect(hline);
//        g.drawLine(width * (i * gridSize) + margin, margin,
//                   width * (i * gridSize) + margin, bounds.getHeight() + margin);
//        g.drawLine(margin, bounds.getHeight() * (i * gridSize) + margin,
//                   bounds.getWidth() + margin, bounds.getHeight() * (i * gridSize) + margin);
    }
}

void APPlot::drawXAxis(juce::Graphics& g, juce::Rectangle<int> axisBounds)
{
    g.setColour(juce::Colours::white);
    g.setFont(8.0f);
    auto gridSize = 0.05f;
    auto margin = 20;
    auto xvalue = 20;
    for (auto i = 0; i < (1 / gridSize + 1); ++i)
    {
        auto xbounds = juce::Rectangle<int> (axisBounds.getWidth() * (i * gridSize) + (margin / 2),
                                             getLocalBounds().getHeight() - margin, 15, 10);
        g.drawText(i != xvalue ? "-"+juce::String(xvalue - i) : juce::String(0),
                   xbounds, juce::Justification::centred, false);
    }
}

void APPlot::drawYAxis(juce::Graphics& g, juce::Rectangle<int> axisBounds)
{
    g.setColour(juce::Colours::white);
    g.setFont(8.0f);
    auto gridSize = 0.05f;
    auto margin = 20;
//    auto xvalue = 20;
    for (auto i = 0; i < (1 / gridSize - 1); ++i)
    {
        auto ybounds = juce::Rectangle<int> (5,
                                             axisBounds.getHeight() * (i * gridSize) + (margin * 0.75),
                                             15, 10);
        g.drawText(i != 0 ? "-"+juce::String(i) : juce::String(0),
                   ybounds, juce::Justification::centred, false);
    }
}

void APPlot::plotLine(juce::Graphics& g, juce::Rectangle<int> plotBounds)
{
    g.setColour(juce::Colours::white);
    juce::Path p;
}
