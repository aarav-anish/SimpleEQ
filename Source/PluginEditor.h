/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class CustomRotarySlider : public juce::Slider
{
public:
  CustomRotarySlider() : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                                      juce::Slider::TextEntryBoxPosition::NoTextBox)
  {
  }
};

//==============================================================================
/**
 */

class ResponseCurveComponent : public juce::Component,
                               public juce::AudioProcessorParameter::Listener,
                               public juce::Timer
{
public:
  ResponseCurveComponent(SimpleEQAudioProcessor &);
  ~ResponseCurveComponent();

  void parameterValueChanged(int parameterIndex, float newValue) override;
  void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override {}
  void timerCallback() override;

  void paint(juce::Graphics &) override;

private:
  SimpleEQAudioProcessor &audioProcessor;
  MonoChain monoChain;
  juce::Atomic<bool> parametersChanged{false};

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ResponseCurveComponent)
};

class SimpleEQAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
  SimpleEQAudioProcessorEditor(SimpleEQAudioProcessor &);
  ~SimpleEQAudioProcessorEditor() override;

  //==============================================================================
  void paint(juce::Graphics &) override;
  void resized() override;

private:
  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  SimpleEQAudioProcessor &audioProcessor;

  CustomRotarySlider peakFreqSlider,
      peakGainSlider,
      peakQualitySlider,
      lowCutFreqSlider,
      highCutFreqSlider,
      lowCutSlopeSlider,
      highCutSlopeSlider;

  ResponseCurveComponent responseCurveComponent;

  using APVTS = juce::AudioProcessorValueTreeState;
  using SliderAttachment = APVTS::SliderAttachment;

  SliderAttachment peakFreqSliderAttachment,
      peakGainSliderAttachment,
      peakQualitySliderAttachment,
      lowCutFreqSliderAttachment,
      highCutFreqSliderAttachment,
      lowCutSlopeSliderAttachment,
      highCutSlopeSliderAttachment;

  std::vector<juce::Component *> getComps();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleEQAudioProcessorEditor)
};
