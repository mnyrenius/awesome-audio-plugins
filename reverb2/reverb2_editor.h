#pragma once

#include "reverb2_processor.h"

using namespace juce;

constexpr float sliderHeight = 0.8f;
constexpr float labelHeight = 1.f - sliderHeight;

class KnobLookAndFeel : public LookAndFeel_V4 {
 public:
  KnobLookAndFeel() { setColour(Slider::thumbColourId, Colours::white); }

  void drawRotarySlider(Graphics& g, int x, int y, int width, int height,
                        float sliderPos, const float rotaryStartAngle,
                        const float rotaryEndAngle, Slider&) override {
    auto radius = (float)jmin(width / 2, height / 2) - 4.0f;
    auto centreX = (float)x + (float)width * 0.5f;
    auto centreY = (float)y + (float)height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle =
        rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    g.setColour(Colours::lightgrey);
    g.fillEllipse(rx, ry, rw, rw);

    g.setColour(Colours::white);
    g.drawEllipse(rx, ry, rw, rw, 5.0f);

    Path p;
    auto pointerLength = radius * 0.33f;
    auto pointerThickness = 5.0f;
    p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness,
                   pointerLength);
    p.applyTransform(
        AffineTransform::rotation(angle).translated(centreX, centreY));

    g.setColour(Colours::black);
    g.fillPath(p);
  }
};

class Knob : public Component {
 public:
  using Ptr = std::shared_ptr<Knob>;

  explicit Knob(const String& labelText) {
    slider_.setLookAndFeel(&lf_);
    slider_.setRange(0.01f, 1.0f, 0.01f);
    addAndMakeVisible(slider_);
    addAndMakeVisible(label_);

    label_.setFont(16.0f);
    label_.setText(labelText, dontSendNotification);
    label_.setJustificationType(Justification::centred);
  }

  void resized() override {
    label_.setBoundsRelative(0.f, 0.f, 1.f, labelHeight);
    slider_.setBoundsRelative(0.f, labelHeight, 1.f, sliderHeight);
  }

  Slider& getSlider() { return slider_; }
  Label& getLabel() { return label_; }

 private:
  Slider slider_{Slider::RotaryHorizontalVerticalDrag, Slider::TextBoxBelow};
  Label label_;
  KnobLookAndFeel lf_;
};

class Reverb2AudioProcessorEditor : public AudioProcessorEditor,
                                    public Slider::Listener {
 public:
  explicit Reverb2AudioProcessorEditor(Reverb2AudioProcessor&);
  ~Reverb2AudioProcessorEditor() override;

  void paint(Graphics&) override;
  void resized() override;

  void sliderValueChanged(Slider* slider) override;

 private:
  Reverb2AudioProcessor& processor_;
  Label titleLabel_;

  std::vector<Knob::Ptr> knobs_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Reverb2AudioProcessorEditor)
};
