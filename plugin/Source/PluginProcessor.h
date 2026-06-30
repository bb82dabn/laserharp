#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

class LaserHarpAudioProcessor final : public juce::AudioProcessor
{
public:
  static constexpr int numStrings = 13;

  LaserHarpAudioProcessor();
  ~LaserHarpAudioProcessor() override = default;

  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override {}
  bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override { return true; }

  const juce::String getName() const override { return "LaserHarp"; }
  bool acceptsMidi() const override { return true; }
  bool producesMidi() const override { return false; }
  bool isMidiEffect() const override { return false; }
  double getTailLengthSeconds() const override { return 2.0; }

  int getNumPrograms() override { return 1; }
  int getCurrentProgram() override { return 0; }
  void setCurrentProgram(int) override {}
  const juce::String getProgramName(int) override { return {}; }
  void changeProgramName(int, const juce::String&) override {}

  void getStateInformation(juce::MemoryBlock& destData) override;
  void setStateInformation(const void* data, int sizeInBytes) override;

  void setStringGates(const std::array<bool, numStrings>& held, bool mod);

  juce::AudioProcessorValueTreeState parameters;

private:
  struct Voice
  {
    double phase = 0.0;
    double phase2 = 0.0;
    float gain = 0.0f;
    float targetGain = 0.0f;
    float frequency = 261.63f;
    int waveform = 2;
    bool active = false;
  };

  static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
  float getStringFrequency(int stringIndex, int scale, int key) const;
  float renderWaveform(int waveform, double phase) const;
  void releaseVoice(int index);
  void triggerVoice(int index, float frequency, int waveform);

  std::array<Voice, numStrings> voices;
  std::array<std::atomic<bool>, numStrings> midiHeld;
  std::array<std::atomic<bool>, numStrings> uiHeld;
  std::atomic<bool> modulation;

  juce::dsp::Reverb reverb;
  juce::dsp::DelayLine<float> delayLine { 48000 * 2 };
  juce::dsp::IIR::Filter<float> lowpass;
  double currentSampleRate = 44100.0;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LaserHarpAudioProcessor)
};
