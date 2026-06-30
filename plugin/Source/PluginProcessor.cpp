#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
constexpr float baseNote = 261.63f;
constexpr double twoPi = juce::MathConstants<double>::twoPi;

const std::array<std::vector<int>, 4> scales {{
  { 0, 2, 4, 7, 9, 12, 14, 16, 19, 21, 24, 26, 28 },
  { 0, 2, 4, 5, 7, 9, 11, 12, 14, 16, 17, 19, 21 },
  { 0, 2, 3, 5, 7, 8, 10, 12, 14, 15, 17, 19, 20 },
  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 }
}};
}

LaserHarpAudioProcessor::LaserHarpAudioProcessor()
  : AudioProcessor(BusesProperties()
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
  for (auto& flag : midiHeld) flag.store(false);
  for (auto& flag : uiHeld) flag.store(false);
  modulation.store(false);
}

juce::AudioProcessorValueTreeState::ParameterLayout LaserHarpAudioProcessor::createParameterLayout()
{
  std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
  params.push_back(std::make_unique<juce::AudioParameterChoice>("waveform", "Waveform",
    juce::StringArray { "Sine", "Triangle", "Saw", "Square" }, 2));
  params.push_back(std::make_unique<juce::AudioParameterChoice>("scale", "Scale",
    juce::StringArray { "Pentatonic", "Major", "Minor", "Chromatic" }, 0));
  params.push_back(std::make_unique<juce::AudioParameterChoice>("key", "Key",
    juce::StringArray { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, 0));
  params.push_back(std::make_unique<juce::AudioParameterFloat>("filter", "Filter Cutoff",
    juce::NormalisableRange<float>(200.0f, 18000.0f), 8000.0f));
  params.push_back(std::make_unique<juce::AudioParameterFloat>("reverb", "Reverb",
    juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
  params.push_back(std::make_unique<juce::AudioParameterFloat>("delay", "Delay",
    juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
  params.push_back(std::make_unique<juce::AudioParameterFloat>("gain", "Output Gain",
    juce::NormalisableRange<float>(0.0f, 1.0f), 0.7f));
  return { params.begin(), params.end() };
}

bool LaserHarpAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
  const auto& out = layouts.getMainOutputChannelSet();
  return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void LaserHarpAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
  currentSampleRate = sampleRate;

  juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32>(samplesPerBlock), static_cast<juce::uint32>(getTotalNumOutputChannels()) };
  reverb.prepare(spec);
  reverb.reset();

  delayLine.prepare(spec);
  delayLine.setMaximumDelayInSamples(static_cast<int>(sampleRate * 1.0));
  delayLine.setDelay(static_cast<float>(sampleRate * 0.35));
  delayLine.reset();

  lowpass.prepare(spec);
  lowpass.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 8000.0, 0.707f);

  for (auto& voice : voices)
  {
    voice.phase = 0;
    voice.phase2 = 0;
    voice.gain = 0;
    voice.targetGain = 0;
    voice.frequency = baseNote;
    voice.waveform = 2;
    voice.active = false;
  }
}

float LaserHarpAudioProcessor::getStringFrequency(int stringIndex, int scale, int key) const
{
  if (scale < 0 || scale >= static_cast<int>(scales.size()))
    return baseNote;
  const auto semitones = key + scales[static_cast<size_t>(scale)][static_cast<size_t>(stringIndex)];
  return baseNote * std::pow(2.0f, semitones / 12.0f);
}

float LaserHarpAudioProcessor::renderWaveform(int waveform, double phase) const
{
  const auto normalized = static_cast<float>(phase / twoPi);
  switch (waveform)
  {
    case 0: return std::sin(static_cast<float>(phase));
    case 1: return 2.0f * std::abs(2.0f * (normalized - std::floor(normalized + 0.5f))) - 1.0f;
    case 2: return 2.0f * (normalized - std::floor(normalized + 0.5f));
    case 3: return normalized < 0.5f ? 1.0f : -1.0f;
  }
  return 0.0f;
}

void LaserHarpAudioProcessor::triggerVoice(int index, float frequency, int waveform)
{
  auto& voice = voices[static_cast<size_t>(index)];
  voice.active = true;
  voice.phase = 0;
  voice.phase2 = 0;
  voice.gain = 0.0f;
  voice.targetGain = 0.6f;
  voice.frequency = frequency;
  voice.waveform = waveform;
}

void LaserHarpAudioProcessor::releaseVoice(int index)
{
  auto& voice = voices[static_cast<size_t>(index)];
  voice.targetGain = 0.0f;
  voice.active = false;
}

void LaserHarpAudioProcessor::setStringGates(const std::array<bool, numStrings>& held, bool mod)
{
  for (int i = 0; i < numStrings; ++i)
    uiHeld[static_cast<size_t>(i)].store(held[static_cast<size_t>(i)]);
  modulation.store(mod);
}

void LaserHarpAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
  juce::ScopedNoDenormals noDenormals;
  buffer.clear();

  const auto waveform = static_cast<int>(parameters.getRawParameterValue("waveform")->load());
  const auto scale = static_cast<int>(parameters.getRawParameterValue("scale")->load());
  const auto key = static_cast<int>(parameters.getRawParameterValue("key")->load());
  const auto filterCutoff = parameters.getRawParameterValue("filter")->load();
  const auto reverbMix = parameters.getRawParameterValue("reverb")->load();
  const auto delayMix = parameters.getRawParameterValue("delay")->load();
  const auto outputGain = parameters.getRawParameterValue("gain")->load();
  const auto modActive = modulation.load();

  if (filterCutoff != lowpass.coefficients->getMagnitudeAtFrequency(filterCutoff, 0.0f))
  {
    *lowpass.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass(currentSampleRate, filterCutoff, 0.707f);
  }

  for (const auto metadata : midi)
  {
    const auto message = metadata.getMessage();
    if (message.isNoteOn()) { midiHeld[message.getNoteNumber() % numStrings].store(true); }
    if (message.isNoteOff()) { midiHeld[message.getNoteNumber() % numStrings].store(false); }
  }

  for (int i = 0; i < numStrings; ++i)
  {
    const bool heldNow = midiHeld[static_cast<size_t>(i)].load() || uiHeld[static_cast<size_t>(i)].load();
    auto& voice = voices[static_cast<size_t>(i)];
    const auto targetFreq = getStringFrequency(i, scale, key);
    voice.frequency = targetFreq;
    voice.waveform = waveform;

    if (heldNow && !voice.active)
      triggerVoice(i, targetFreq, waveform);
    if (!heldNow && voice.active)
      releaseVoice(i);
  }

  const auto delayFeedback = modActive ? 0.55f : 0.4f;
  const auto tremolo = modActive ? (0.82f + 0.14f * static_cast<float>(std::sin(twoPi * 0.3))) : 1.0f;
  const auto wet = std::min(1.0f, reverbMix + (modActive ? 0.18f : 0.0f));
  juce::dsp::Reverb::Parameters reverbParameters;
  reverbParameters.roomSize = 0.55f;
  reverbParameters.damping = 0.4f;
  reverbParameters.wetLevel = wet * 0.45f;
  reverbParameters.dryLevel = 1.0f - reverbParameters.wetLevel * 0.5f;
  reverb.setParameters(reverbParameters);

  auto* left = buffer.getWritePointer(0);
  auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;

  for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
  {
    float sampleValue = 0.0f;
    for (int i = 0; i < numStrings; ++i)
    {
      auto& voice = voices[static_cast<size_t>(i)];
      voice.gain += (voice.targetGain - voice.gain) * 0.005f;
      if (voice.gain < 0.0005f && !voice.active) { voice.gain = 0.0f; continue; }
      const auto detune = modActive ? std::sin(static_cast<float>(sample) * 0.0006f) * 22.0f : 0.0f;
      const auto frequency = voice.frequency * std::pow(2.0f, detune / 12.0f);
      voice.phase += twoPi * frequency / currentSampleRate;
      if (voice.phase >= twoPi) voice.phase -= twoPi;
      voice.phase2 += twoPi * (frequency * std::pow(2.0f, 7.0f / 12.0f)) / currentSampleRate;
      if (voice.phase2 >= twoPi) voice.phase2 -= twoPi;
      const auto primary = renderWaveform(voice.waveform, voice.phase);
      const auto secondary = renderWaveform(voice.waveform, voice.phase2) * 0.25f;
      sampleValue += (primary + secondary) * voice.gain * 0.35f;
    }

    auto filtered = lowpass.processSample(sampleValue);
    const auto delayed = delayLine.popSample(0);
    delayLine.pushSample(0, filtered + delayed * delayFeedback);
    const auto mixed = filtered * (1.0f - delayMix) + delayed * delayMix;
    left[sample] = mixed * outputGain * tremolo;
    if (right != nullptr) right[sample] = mixed * outputGain * tremolo;
  }

  juce::dsp::AudioBlock<float> block(buffer);
  reverb.process(juce::dsp::ProcessContextReplacing<float>(block));
}

void LaserHarpAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
  if (auto xml = parameters.copyState().createXml())
    copyXmlToBinary(*xml, destData);
}

void LaserHarpAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
  if (auto xml = getXmlFromBinary(data, sizeInBytes))
    parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorEditor* LaserHarpAudioProcessor::createEditor()
{
  return new LaserHarpAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
  return new LaserHarpAudioProcessor();
}
