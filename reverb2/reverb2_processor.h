#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

using namespace juce;

enum ReverbParameters {
  Mix,
  PreDelay,
  Size,
  Decay,
  Speed,
  Depth,
  Damping,
  End
};

class Allpass {
 public:
  Allpass(std::uint32_t size, float fbGain, float ffGain)
      : size_(size), fbGain_(fbGain), ffGain_(ffGain) {
    buffer_.resize(size);
    std::fill(buffer_.begin(), buffer_.end(), 0);
  }

  inline float process(float in, float delay) {
    auto m = index_ - delay;
    if (m < 0) m += size_;

    const auto y = buffer_[m] + ffGain_ * in;
    buffer_[index_++] = in + fbGain_ * y;
    if (index_ >= size_) index_ = 0;

    return y;
  }

  inline float tap(std::uint32_t index) const {
    std::int32_t m = index_ - index;
    if (m < 0) m += size_;
    return buffer_[m];
  }

  inline std::uint32_t size() const { return size_; }

 private:
  std::vector<float> buffer_{};
  std::uint32_t index_{};
  std::uint32_t size_{};
  float fbGain_{};
  float ffGain_{};
};

class LPFilter {
 public:
  inline float process(float input, float gain, float fbGain) {
    return x1_ = gain * input + fbGain * x1_;
  }

 private:
  float x1_{};
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

class ReverbTank {
 public:
  ReverbTank() {}

  inline void setSampleRate(float fs) { fs_ = fs; }

  inline std::tuple<float, float> process(float input, float size, float decay,
                                          float damping, float modRate,
                                          float modDepth) {
    float out_l = 0.0f;
    float out_r = 0.0f;

    const auto mod =
        std::sin(2.0f * 3.141592f * modPhase_ / fs_) * 128.0f * modDepth;
    modPhase_ += 3.0f * modRate;

    auto tank1 = decayDiffusion1Left_.process(
                     input, size * Diffusion1BaseDelayLeft - 1.0f + mod) +
                 decay * delay2Right_.read(size * delay2Right_.size() - 1.0f);
    delay1Left_.write(tank1);
    tank1 = delay1Left_.read(size * delay1Left_.size() - 1.0f);
    tank1 = dampingLeft_.process(tank1, 1.0f - damping, damping);
    tank1 = decayDiffusion2Left_.process(
        tank1 * decay, size * decayDiffusion2Left_.size() - 1.0f);
    delay2Left_.write(tank1);

    auto tank2 = decayDiffusion1Right_.process(
                     input, size * Diffusion1BaseDelayRight - 1.0f + mod) +
                 decay * delay2Left_.read(size * delay2Left_.size() - 1.0f);
    delay1Right_.write(tank2);
    tank2 = delay1Right_.read(size * delay1Right_.size() - 1.0f);
    tank2 = dampingRight_.process(tank2, 1.0f - damping, damping);
    tank2 = decayDiffusion2Right_.process(
        tank2 * decay, size * decayDiffusion2Right_.size() - 1.0f);
    delay2Right_.write(tank2);

    const float ratio = fs_ / 29761.0f;
    out_l += 0.6f * delay1Right_.read(2 * size * 266.0f * ratio);
    out_l += 0.6f * delay1Right_.read(2 * size * 2974.0f * ratio);
    out_l -= 0.6f * decayDiffusion2Right_.tap(2 * size * 1913.0f * ratio);
    out_l += 0.6f * delay2Right_.read(2 * size * 1996.0f * ratio);
    out_l -= 0.6f * delay1Left_.read(2 * size * 1990.0f * ratio);
    out_l -= 0.6f * decayDiffusion2Left_.tap(2 * size * 187.0f * ratio);
    out_l -= 0.6f * delay2Left_.read(2 * size * 1066.0f * ratio);

    out_r += 0.6f * delay1Left_.read(2 * size * 353.0f * ratio);
    out_r += 0.6f * delay1Left_.read(2 * size * 3627.0f * ratio);
    out_r -= 0.6f * decayDiffusion2Left_.tap(2 * size * 1228.0f * ratio);
    out_r += 0.6f * delay2Left_.read(2 * size * 2673.0f * ratio);
    out_r -= 0.6f * delay2Right_.read(2 * size * 2111.0f * ratio);
    out_r -= 0.6f * decayDiffusion2Right_.tap(2 * size * 335.0f * ratio);
    out_r -= 0.6f * delay2Right_.read(2 * size * 121.0f * ratio);

    return {out_l, out_r};
  }

 private:
  // left side of tank
  const std::uint32_t Diffusion1BaseDelayLeft = 2 * 995;
  Allpass decayDiffusion1Left_{Diffusion1BaseDelayLeft + 128, 0.7, -0.7};
  Allpass decayDiffusion2Left_{2 * 2667, -0.5, 0.5};
  Delay delay1Left_{2 * 6598};
  LPFilter dampingLeft_{};
  Delay delay2Left_{2 * 5512};

  // right side of tank
  const std::uint32_t Diffusion1BaseDelayRight = 2 * 1345;
  Allpass decayDiffusion1Right_{Diffusion1BaseDelayRight + 128, 0.7f, -0.7f};
  Allpass decayDiffusion2Right_{2 * 3935, -0.5f, 0.5f};
  Delay delay1Right_{2 * 6248};
  LPFilter dampingRight_{};
  Delay delay2Right_{2 * 4687};

  float fs_;
  float modPhase_{};
};

class ReverbParam : public AudioProcessorParameter {
 public:
  ReverbParam(const String& name, float defaultValue)
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
class Reverb2AudioProcessor : public juce::AudioProcessor {
 public:
  //==============================================================================
  Reverb2AudioProcessor();
  ~Reverb2AudioProcessor() override;

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
  float sizeCurrent_{};
  Delay predelay_{20001};
  LPFilter predelayFilter_{};
  std::vector<Allpass> inputDiffusionAps_{{2 * 210, -0.75, 0.75},
                                          {2 * 148, -0.75, 0.75},
                                          {2 * 561, -0.625, 0.625},
                                          {2 * 410, -0.625, 0.625}};
  ReverbTank reverbTank_{};

  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Reverb2AudioProcessor)
};
