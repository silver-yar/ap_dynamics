//
// Created by Johnathan Handy on 6/18/22.
//

#include "APConvolution.h"

#include "BinaryData.h"

APConvolution::APConvolution()
{
  using Conv = juce::dsp::Convolution;

  cabConvolution_ = std::make_unique<juce::dsp::Convolution>();
  cabConvolution_->loadImpulseResponse(BinaryData::mdcone_wav, BinaryData::mdcone_wavSize, Conv ::Stereo::yes,
                                       Conv::Trim::no, 0, juce::dsp::Convolution::Normalise::yes);
}

APConvolution::~APConvolution() { }
