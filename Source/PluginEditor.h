/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class LookAndFeel : public juce::LookAndFeel_V4
{
  void drawRotarySlider(juce::Graphics &g,
                        int x, int y, int width, int height,
                        float sliderPosProportional,
                        float rotaryStartAngle,
                        float rotaryEndAngle,
                        juce::Slider &slider) override;
};

class RotarySliderWithLabels : public juce::Slider
{
public:
  RotarySliderWithLabels(juce::RangedAudioParameter &rap, const juce::String &unitSuffix)
      : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                     juce::Slider::TextEntryBoxPosition::NoTextBox),
        param(&rap),
        suffix(unitSuffix)
  {
    setLookAndFeel(&lnf);
  }

  ~RotarySliderWithLabels()
  {
    setLookAndFeel(nullptr);
  }

  void paint(juce::Graphics &g) override;

  juce::Rectangle<int> getSliderBounds() const;

  int getTextHeight() const { return 14; }

  juce::String getDisplayString() const;

private:
  LookAndFeel lnf;

  juce::RangedAudioParameter *param;
  juce::String suffix;
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

  RotarySliderWithLabels peakFreqSlider,
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
