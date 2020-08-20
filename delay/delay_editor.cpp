#include "delay_editor.h"

#include "delay_processor.h"

//==============================================================================
DelayAudioProcessorEditor::DelayAudioProcessorEditor(
    DelayAudioProcessor& p)
    : AudioProcessorEditor(&p), processor_(p) {
  knobs_.resize(DelayParameters::End);
  knobs_[DelayParameters::Mix] = std::make_shared<Knob>("Mix");
  knobs_[DelayParameters::Time] = std::make_shared<Knob>("Time");
  knobs_[DelayParameters::Feedback] = std::make_shared<Knob>("Feedback");

  addAndMakeVisible(titleLabel_);
  titleLabel_.setColour(Label::textColourId, Colours::silver);
  titleLabel_.setFont(Font(32.0f, Font::bold));
  titleLabel_.setJustificationType(Justification::horizontallyCentred);
  titleLabel_.setText("D E L A Y", dontSendNotification);

  for (auto i = 0U; i < DelayParameters::End; ++i) {
    addAndMakeVisible(*knobs_[i]);
    knobs_[i]->getSlider().addListener(this);
    knobs_[i]->getSlider().setValue(processor_.getParameters()[i]->getValue());
  }

  setSize(800, 200);
}

DelayAudioProcessorEditor::~DelayAudioProcessorEditor() {}

//==============================================================================
void DelayAudioProcessorEditor::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);
}

void DelayAudioProcessorEditor::resized() {
  auto b = getLocalBounds();
  titleLabel_.setBounds(b.removeFromTop(30));

  FlexBox fb;
  fb.flexDirection = FlexBox::Direction::row;
  fb.justifyContent = juce::FlexBox::JustifyContent::center;

  for (auto& knob : knobs_) {
    fb.items.add(FlexItem(getWidth() / 10, getHeight() / 5, *knob).withMargin(30.0f));
  }

  fb.performLayout(b);
}

void DelayAudioProcessorEditor::sliderValueChanged(Slider* slider) {
  if (slider == &knobs_[DelayParameters::Mix]->getSlider()) {
    processor_.getParameters()[DelayParameters::Mix]->setValueNotifyingHost(
        slider->getValue());
  } else if (slider == &knobs_[DelayParameters::Time]->getSlider()) {
    processor_.getParameters()[DelayParameters::Time]->setValueNotifyingHost(
        slider->getValue());
  } else if (slider == &knobs_[DelayParameters::Feedback]->getSlider()) {
    processor_.getParameters()[DelayParameters::Feedback]->setValueNotifyingHost(
        slider->getValue());
  }
}
