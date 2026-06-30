#include "PluginEditor.h"

namespace
{
const juce::Colour backgroundColour { 0xff05080f };
const juce::Colour primaryColour { 0xff00f0ff };
const juce::Colour secondaryColour { 0xffff2d75 };
const juce::Colour textDimColour { 0xff7a8d99 };

const juce::StringArray laserPalette { "#ff0044", "#ff2200", "#ff6600", "#ff9900", "#ffcc00",
                                       "#aaff00", "#00ff66", "#00ffcc", "#00ccff", "#0066ff",
                                       "#6600ff", "#aa00ff", "#ff00cc" };
}

LaserHarpAudioProcessorEditor::LaserHarpAudioProcessorEditor(LaserHarpAudioProcessor& p)
  : AudioProcessorEditor(&p), processor(p)
{
  setSize(820, 520);

  configureCombo(waveformBox, { "Sine", "Triangle", "Saw", "Square" });
  configureCombo(scaleBox, { "Pentatonic", "Major", "Minor", "Chromatic" });
  configureCombo(keyBox, { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" });
  configureSlider(filterSlider, filterLabel, "Filter");
  configureSlider(reverbSlider, reverbLabel, "Reverb");
  configureSlider(delaySlider, delayLabel, "Delay");
  configureSlider(gainSlider, gainLabel, "Gain");

  waveformAttachment = std::make_unique<ComboAttachment>(processor.parameters, "waveform", waveformBox);
  scaleAttachment = std::make_unique<ComboAttachment>(processor.parameters, "scale", scaleBox);
  keyAttachment = std::make_unique<ComboAttachment>(processor.parameters, "key", keyBox);
  filterAttachment = std::make_unique<SliderAttachment>(processor.parameters, "filter", filterSlider);
  reverbAttachment = std::make_unique<SliderAttachment>(processor.parameters, "reverb", reverbSlider);
  delayAttachment = std::make_unique<SliderAttachment>(processor.parameters, "delay", delaySlider);
  gainAttachment = std::make_unique<SliderAttachment>(processor.parameters, "gain", gainSlider);
}

void LaserHarpAudioProcessorEditor::configureCombo(juce::ComboBox& box, const juce::StringArray& items)
{
  for (int i = 0; i < items.size(); ++i)
    box.addItem(items[i], i + 1);
  box.setColour(juce::ComboBox::backgroundColourId, juce::Colours::black.withAlpha(0.35f));
  box.setColour(juce::ComboBox::outlineColourId, primaryColour.withAlpha(0.35f));
  box.setColour(juce::ComboBox::textColourId, primaryColour);
  addAndMakeVisible(box);
}

void LaserHarpAudioProcessorEditor::configureSlider(juce::Slider& slider, juce::Label& label, const juce::String& text)
{
  slider.setSliderStyle(juce::Slider::LinearHorizontal);
  slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 54, 18);
  slider.setColour(juce::Slider::trackColourId, primaryColour.withAlpha(0.25f));
  slider.setColour(juce::Slider::thumbColourId, primaryColour);
  slider.setColour(juce::Slider::textBoxTextColourId, primaryColour);
  slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
  label.setText(text, juce::dontSendNotification);
  label.setJustificationType(juce::Justification::centred);
  label.setColour(juce::Label::textColourId, textDimColour);
  addAndMakeVisible(slider);
  addAndMakeVisible(label);
}

int LaserHarpAudioProcessorEditor::findNearestString(juce::Point<int> position) const
{
  if (playArea.isEmpty()) return -1;
  const auto x = static_cast<float>(position.x - playArea.getX()) / static_cast<float>(playArea.getWidth());
  const auto index = juce::jlimit(0, LaserHarpAudioProcessor::numStrings - 1, static_cast<int>(std::round(x * (LaserHarpAudioProcessor::numStrings - 1))));
  return index;
}

void LaserHarpAudioProcessorEditor::refreshGates()
{
  std::array<bool, LaserHarpAudioProcessor::numStrings> gates {};
  for (size_t i = 0; i < gates.size(); ++i) gates[i] = held[i];
  processor.setStringGates(gates, rightMod);
  repaint();
}

void LaserHarpAudioProcessorEditor::paint(juce::Graphics& g)
{
  g.fillAll(backgroundColour);
  auto bounds = getLocalBounds().toFloat();
  g.setGradientFill(juce::ColourGradient(primaryColour.withAlpha(0.12f), 0.0f, 0.0f,
                                         secondaryColour.withAlpha(0.08f), bounds.getBottomRight().x, bounds.getBottomRight().y, false));
  g.fillRect(bounds);

  g.setColour(juce::Colours::black.withAlpha(0.6f));
  g.fillRoundedRectangle(playArea.toFloat(), 10.0f);
  g.setColour(primaryColour.withAlpha(0.22f));
  g.drawRoundedRectangle(playArea.toFloat(), 10.0f, 1.0f);

  if (playArea.getWidth() > 0)
  {
    for (int i = 0; i < LaserHarpAudioProcessor::numStrings; ++i)
    {
      const auto t = static_cast<float>(i) / static_cast<float>(LaserHarpAudioProcessor::numStrings - 1);
      const auto x = static_cast<float>(playArea.getX()) + t * static_cast<float>(playArea.getWidth());
      juce::Colour colour = juce::Colour::fromString(laserPalette[static_cast<int>(i)]);
      if (held[static_cast<size_t>(i)])
        colour = colour.brighter(0.4f);
      g.setColour(colour.withAlpha(held[static_cast<size_t>(i)] ? 0.95f : 0.55f));
      g.drawLine(x, static_cast<float>(playArea.getY()) + 12.0f, x, static_cast<float>(playArea.getBottom()) - 12.0f, held[static_cast<size_t>(i)] ? 4.0f : 2.0f);
    }
  }

  g.setFont(juce::FontOptions(28.0f, juce::Font::bold));
  g.setColour(primaryColour);
  g.drawText("LASER HARP", getLocalBounds().removeFromTop(58), juce::Justification::centred);
}

void LaserHarpAudioProcessorEditor::resized()
{
  auto bounds = getLocalBounds().reduced(16);
  bounds.removeFromTop(52);
  auto controls = bounds.removeFromBottom(112);
  playArea = bounds.reduced(0, 4);

  auto topControls = controls.removeFromTop(46);
  waveformBox.setBounds(topControls.removeFromLeft(150).reduced(5));
  scaleBox.setBounds(topControls.removeFromLeft(170).reduced(5));
  keyBox.setBounds(topControls.removeFromLeft(92).reduced(5));

  auto layoutSlider = [] (juce::Rectangle<int> area, juce::Label& label, juce::Slider& slider)
  {
    label.setBounds(area.removeFromTop(20));
    slider.setBounds(area);
  };

  auto sliderArea = controls;
  layoutSlider(sliderArea.removeFromLeft(190).reduced(6), filterLabel, filterSlider);
  layoutSlider(sliderArea.removeFromLeft(190).reduced(6), reverbLabel, reverbSlider);
  layoutSlider(sliderArea.removeFromLeft(190).reduced(6), delayLabel, delaySlider);
  layoutSlider(sliderArea.removeFromLeft(160).reduced(6), gainLabel, gainSlider);
}

void LaserHarpAudioProcessorEditor::mouseDown(const juce::MouseEvent& event)
{
  if (event.mods.isLeftButtonDown())
  {
    leftDown = true;
    rightMod = event.mods.isRightButtonDown();
    const auto stringIndex = findNearestString(event.getPosition());
    if (stringIndex >= 0) held[static_cast<size_t>(stringIndex)] = true;
  }
  refreshGates();
}

void LaserHarpAudioProcessorEditor::mouseDrag(const juce::MouseEvent& event)
{
  if (leftDown)
  {
    rightMod = event.mods.isRightButtonDown();
    std::fill(held.begin(), held.end(), false);
    const auto stringIndex = findNearestString(event.getPosition());
    if (stringIndex >= 0) held[static_cast<size_t>(stringIndex)] = true;
    refreshGates();
  }
}

void LaserHarpAudioProcessorEditor::mouseUp(const juce::MouseEvent& event)
{
  if (event.mods.isLeftButtonDown()) return;
  leftDown = false;
  rightMod = false;
  std::fill(held.begin(), held.end(), false);
  refreshGates();
}
