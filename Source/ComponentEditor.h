/*
  ==============================================================================

    ShowHideContainer.h
    Created: 9 Mar 2021 12:03:16am
    Author:  Johnathan Handy

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
 */
// JUCE_DECLARE_SINGLETON - maybe add?
class ComponentEditor : public juce::Component
{
 public:
  using Listener = std::function<void(void)>;

  ComponentEditor();

  ~ComponentEditor() override;

  void addListener (Listener);

  float getValue(const juce::String&);

 private:
  juce::Array<Listener> listeners_;
  juce::Array<std::pair<std::unique_ptr<juce::TextEditor>, std::unique_ptr<juce::TextEditor>>> fieldPairs_;

  juce::HashMap<juce::String, float> fieldData_;

  void addField();
  void updateValue (const juce::String& key, float value);
  void notifyListeners();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShowHideContainer)
};
