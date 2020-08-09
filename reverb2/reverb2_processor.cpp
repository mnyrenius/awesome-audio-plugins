#include "reverb2_processor.h"

#include "reverb2_editor.h"

//==============================================================================
Reverb2AudioProcessor::Reverb2AudioProcessor()
    : AudioProcessor(
          BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
      ) {
  parameters_.resize(ReverbParameters::End);
  addParameter(parameters_[ReverbParameters::Mix] =
                   new ReverbParam("Mix", 0.3f));
  addParameter(parameters_[ReverbParameters::Size] =
                   new ReverbParam("Size", 0.5f));
  addParameter(parameters_[ReverbParameters::PreDelay] =
                   new ReverbParam("Predelay", 0.01f));
  addParameter(parameters_[ReverbParameters::Decay] =
                   new ReverbParam("Feedback", 0.3f));
  addParameter(parameters_[ReverbParameters::Speed] =
                   new ReverbParam("ModRate", 0.1f));
  addParameter(parameters_[ReverbParameters::Depth] =
                   new ReverbParam("ModDepth", 0.0f));
  addParameter(parameters_[ReverbParameters::Damping] =
                   new ReverbParam("Damping", 0.05f));
}

Reverb2AudioProcessor::~Reverb2AudioProcessor() {}

//==============================================================================
const juce::String Reverb2AudioProcessor::getName() const {
  return JucePlugin_Name;
}

bool Reverb2AudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool Reverb2AudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool Reverb2AudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double Reverb2AudioProcessor::getTailLengthSeconds() const { return 0.0; }

int Reverb2AudioProcessor::getNumPrograms() {
  return 1;  // NB: some hosts don't cope very well if you tell them there are 0
             // programs, so this should be at least 1, even if you're not
             // really implementing programs.
}

int Reverb2AudioProcessor::getCurrentProgram() { return 0; }

void Reverb2AudioProcessor::setCurrentProgram(int index) {
  juce::ignoreUnused(index);
}

const juce::String Reverb2AudioProcessor::getProgramName(int index) {
  juce::ignoreUnused(index);
  return {};
}

void Reverb2AudioProcessor::changeProgramName(int index,
                                              const juce::String& newName) {
  juce::ignoreUnused(index, newName);
}

//==============================================================================
void Reverb2AudioProcessor::prepareToPlay(double sampleRate,
                                          int samplesPerBlock) {
  // Use this method as the place to do any pre-playback
  // initialisation that you need..
  juce::ignoreUnused(samplesPerBlock);

  reverbTank_.setSampleRate(sampleRate);
  std::cout << "Sample rate: " << sampleRate << std::endl;
  sizeCurrent_ = parameters_[ReverbParameters::Size]->getValue();
}

void Reverb2AudioProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

bool Reverb2AudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const {
#if JucePlugin_IsMidiEffect
  juce::ignoreUnused(layouts);
  return true;
#else
  // This is the place where you check if the layout is supported.
  // In this template code we only support mono or stereo.
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

    // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;
#endif

  return true;
#endif
}

// Plate-class reverb from J. Dattorro, Effect Design Part 1: Reverberator and Other Filters
void Reverb2AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                         juce::MidiBuffer& midiMessages) {
  juce::ignoreUnused(midiMessages);

  juce::ScopedNoDenormals noDenormals;

  auto num_samples = buffer.getNumSamples();
  auto inL = buffer.getReadPointer(0);
  auto inR = buffer.getReadPointer(1);
  auto outL = buffer.getWritePointer(0);
  auto outR = buffer.getWritePointer(1);

  auto mix = parameters_[ReverbParameters::Mix]->getValue();
  auto predelay = 20000 * parameters_[ReverbParameters::PreDelay]->getValue();
  auto damping = parameters_[ReverbParameters::Damping]->getValue();
  auto decay = parameters_[ReverbParameters::Decay]->getValue();
  auto speed = parameters_[ReverbParameters::Speed]->getValue();
  auto depth = parameters_[ReverbParameters::Depth]->getValue();
  auto size = parameters_[ReverbParameters::Size]->getValue();

  while (num_samples--) {
    if (sizeCurrent_ < size) {
      sizeCurrent_ += 0.000005f;
    } else if (sizeCurrent_ > size) {
      sizeCurrent_ -= 0.000005f;
    }

    const auto left = *inL++;
    const auto right = *inR++;

    const auto in = 0.5 * (left + right);

    // Predelay + low pass filter
    predelay_.write(in);
    auto predelayed = predelay_.read(predelay);
    predelayed = predelayFilter_.process(predelayed, 0.9995, 1 - 0.9995);

    // Input Diffusers
    auto diffused = predelayed;
    for (auto& ap : inputDiffusionAps_) {
      diffused = ap.process(diffused, sizeCurrent_ * ap.size() - 1);
    }

    // Tank
    const auto wet = reverbTank_.process(diffused, sizeCurrent_, decay, damping,
                                         speed, depth);

    *outL++ = std::get<0>(wet) * mix + left * (1 - mix);
    *outR++ = std::get<1>(wet) * mix + right * (1 - mix);
  }
}

//==============================================================================
bool Reverb2AudioProcessor::hasEditor() const {
  return true;  // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Reverb2AudioProcessor::createEditor() {
  return new Reverb2AudioProcessorEditor(*this);
}

//==============================================================================
void Reverb2AudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
  // You should use this method to store your parameters in the memory block.
  // You could do that either as raw data, or use the XML or ValueTree classes
  // as intermediaries to make it easy to save and load complex data.
  juce::ignoreUnused(destData);
}

void Reverb2AudioProcessor::setStateInformation(const void* data,
                                                int sizeInBytes) {
  // You should use this method to restore your parameters from this memory
  // block, whose contents will have been created by the getStateInformation()
  // call.
  juce::ignoreUnused(data, sizeInBytes);
}

std::vector<AudioProcessorParameter*> Reverb2AudioProcessor::getParameters() {
  return parameters_;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new Reverb2AudioProcessor();
}
