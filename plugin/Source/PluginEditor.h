#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class LaserHarpAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
  explicit LaserHarpAudioProcessorEditor(LaserHarpAudioProcessor&);
  ~LaserHarpAudioProcessorEditor() override = default;

  void paint(juce::Graphics&) override;
  void resized() override;
  void mouseDown(const juce::MouseEvent&) override;
  void mouseDrag(const juce::MouseEvent&) override;
  void mouseUp(const juce::MouseEvent&) override;

private:
  using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
  using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

  void configureCombo(juce::ComboBox&, const juce::StringArray&);
  void configureSlider(juce::Slider&, juce::Label&, const juce::String&);
  int findNearestString(juce::Point<int>) const;
  void refreshGates();

  LaserHarpAudioProcessor& processor;
  juce::Rectangle<int> playArea;
  std::array<bool, LaserHarpAudioProcessor::numStrings> held {};
  bool leftDown = false;
  bool rightMod = false;

  juce::ComboBox waveformBox;
  juce::ComboBox scaleBox;
  juce::ComboBox keyBox;
  juce::Slider filterSlider;
  juce::Slider reverbSlider;
  juce::Slider delaySlider;
  juce::Slider gainSlider;
  juce::Label filterLabel;
  juce::Label reverbLabel;
  juce::Label delayLabel;
  juce::Label gainLabel;

  std::unique_ptr<ComboAttachment> waveformAttachment;
  std::unique_ptr<ComboAttachment> scaleAttachment;
  std::unique_ptr<ComboAttachment> keyAttachment;
  std::unique_ptr<SliderAttachment> filterAttachment;
  std::unique_ptr<SliderAttachment> reverbAttachment;
  std::unique_ptr<SliderAttachment> delayAttachment;
  std::unique_ptr<SliderAttachment> gainAttachment;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LaserHarpAudioProcessorEditor)
};
