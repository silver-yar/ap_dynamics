//
// Created by Johnathan Handy on 6/18/22.
//

#pragma once
#include "juce_dsp/juce_dsp.h"

class APConvolution
{
 public:
  APConvolution();
  ~APConvolution();

  void prepare (const juce::dsp::ProcessSpec& spec)
  {
    cabConvolution_->prepare(spec);
  }

  void process (const juce::dsp::ProcessContextReplacing<float>& context)
  {
    cabConvolution_->process(context);
  }

  void reset() { cabConvolution_->reset(); }

 private:
  std::unique_ptr<juce::dsp::Convolution> cabConvolution_;
};

