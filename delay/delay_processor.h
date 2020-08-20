#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

using namespace juce;

enum DelayParameters {
  Mix,
  Time,
  Feedback,
  End
};

class Delay {
 public:
  Delay(std::uint32_t size) : size_(size) {
    buffer_.resize(size);
    std::fill(buffer_.begin(), buffer_.end(), 0);
  }

  inline float read(float delay) const {
    auto m = index_ - delay;
    if (m < 0) m += size_;

    return buffer_[m];
  }

  inline void write(float in) {
    buffer_[index_++] = in;
    if (index_ >= size_) index_ = 0;
  }

  inline std::uint32_t size() const { return size_; }

 private:
  std::vector<float> buffer_;
  std::uint32_t size_{};
  std::uint32_t index_{};
};

class DelayParam : public AudioProcessorParameter {
 public:
  DelayParam(const String& name, float defaultValue)
      : name_(name), defaultValue_(defaultValue) {
    value_.store(defaultValue);
  }

  float getValue() const override { return value_.load(); }

  void setValue(float v) override { value_.store(v); }

  float getDefaultValue() const override { return 0.5f; }

  String getName(int maximumStringLength) const override { return name_; }

  String getLabel() const override { return ""; }

  float getValueForText(const String& text) const override { return 0.0f; }

  String name_;
  float defaultValue_;
  std::atomic<float> value_;
};

//==============================================================================
class DelayAudioProcessor : public juce::AudioProcessor {
 public:
  //==============================================================================
  DelayAudioProcessor();
  ~DelayAudioProcessor() override;

  //==============================================================================
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

  bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

  //==============================================================================
  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override;

  //==============================================================================
  const juce::String getName() const override;

  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;

  //==============================================================================
  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String& newName) override;

  //==============================================================================
  void getStateInformation(juce::MemoryBlock& destData) override;
  void setStateInformation(const void* data, int sizeInBytes) override;

  std::vector<AudioProcessorParameter*> getParameters();

 private:
  std::vector<AudioProcessorParameter*> parameters_{};
  float currentTime_{};
  Delay delayLeft_{1024 * 100};
  Delay delayRight_{1024 * 100};

  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DelayAudioProcessor)
};
