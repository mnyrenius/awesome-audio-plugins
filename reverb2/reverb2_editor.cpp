#include "reverb2_editor.h"

#include "reverb2_processor.h"

//==============================================================================
Reverb2AudioProcessorEditor::Reverb2AudioProcessorEditor(
    Reverb2AudioProcessor& p)
    : AudioProcessorEditor(&p), processor_(p) {
  knobs_.resize(ReverbParameters::End);
  knobs_[ReverbParameters::Mix] = std::make_shared<Knob>("Mix");
  knobs_[ReverbParameters::Size] = std::make_shared<Knob>("Size");
  knobs_[ReverbParameters::PreDelay] = std::make_shared<Knob>("Pre-Delay");
  knobs_[ReverbParameters::Decay] = std::make_shared<Knob>("Decay");
  knobs_[ReverbParameters::Speed] = std::make_shared<Knob>("Speed");
  knobs_[ReverbParameters::Depth] = std::make_shared<Knob>("Depth");
  knobs_[ReverbParameters::Damping] = std::make_shared<Knob>("Damping");

  addAndMakeVisible(titleLabel_);
  titleLabel_.setColour(Label::textColourId, Colours::lightgrey);
  titleLabel_.setFont(Font(32.0f, Font::bold));
  titleLabel_.setJustificationType(Justification::horizontallyCentred);
  titleLabel_.setText("R E V E R B | 2", dontSendNotification);

  for (auto i = 0U; i < ReverbParameters::End; ++i) {
    addAndMakeVisible(*knobs_[i]);
    knobs_[i]->getSlider().addListener(this);
    knobs_[i]->getSlider().setValue(processor_.getParameters()[i]->getValue());
  }

  setSize(800, 200);
}

Reverb2AudioProcessorEditor::~Reverb2AudioProcessorEditor() {}

//==============================================================================
void Reverb2AudioProcessorEditor::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);
}

void Reverb2AudioProcessorEditor::resized() {
  auto b = getLocalBounds();
  titleLabel_.setBounds(b.removeFromTop(30));

  FlexBox fb;
  fb.flexDirection = FlexBox::Direction::row;

  for (auto& knob : knobs_) {
    fb.items.add(FlexItem(getWidth() / 7, getHeight() / 5, *knob));
  }

  fb.performLayout(b);
}

void Reverb2AudioProcessorEditor::sliderValueChanged(Slider* slider) {
  if (slider == &knobs_[ReverbParameters::Mix]->getSlider()) {
    processor_.getParameters()[ReverbParameters::Mix]->setValueNotifyingHost(
        slider->getValue());
  } else if (slider == &knobs_[ReverbParameters::Size]->getSlider()) {
    processor_.getParameters()[ReverbParameters::Size]->setValueNotifyingHost(
        slider->getValue());
  } else if (slider == &knobs_[ReverbParameters::PreDelay]->getSlider()) {
    processor_.getParameters()[ReverbParameters::PreDelay]->setValueNotifyingHost(
        slider->getValue());
  } else if (slider == &knobs_[ReverbParameters::Decay]->getSlider()) {
    processor_.getParameters()[ReverbParameters::Decay]->setValueNotifyingHost(
        slider->getValue());
  } else if (slider == &knobs_[ReverbParameters::Speed]->getSlider()) {
    processor_.getParameters()[ReverbParameters::Speed]->setValueNotifyingHost(
        slider->getValue());
  } else if (slider == &knobs_[ReverbParameters::Depth]->getSlider()) {
    processor_.getParameters()[ReverbParameters::Depth]->setValueNotifyingHost(
        slider->getValue());
  } else if (slider == &knobs_[ReverbParameters::Damping]->getSlider()) {
    processor_.getParameters()[ReverbParameters::Damping]->setValueNotifyingHost(
        slider->getValue());
  }
}
