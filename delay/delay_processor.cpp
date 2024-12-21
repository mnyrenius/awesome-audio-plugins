#include "delay_processor.h"

#include "delay_editor.h"

//==============================================================================
DelayAudioProcessor::DelayAudioProcessor()
    : AudioProcessor(
          BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
      ) {
  parameters_.resize(DelayParameters::End);
  addParameter(parameters_[DelayParameters::Mix] = new DelayParam("Mix", 0.3f));
  addParameter(parameters_[DelayParameters::Time] =
                   new DelayParam("Time", 0.5f));
  addParameter(parameters_[DelayParameters::Feedback] =
                   new DelayParam("Feedback", 0.3f));
}

DelayAudioProcessor::~DelayAudioProcessor() {}

//==============================================================================
const juce::String DelayAudioProcessor::getName() const {
  return JucePlugin_Name;
}

bool DelayAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool DelayAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool DelayAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double DelayAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int DelayAudioProcessor::getNumPrograms() {
  return 1;  // NB: some hosts don't cope very well if you tell them there are 0
             // programs, so this should be at least 1, even if you're not
             // really implementing programs.
}

int DelayAudioProcessor::getCurrentProgram() { return 0; }

void DelayAudioProcessor::setCurrentProgram(int index) {
  juce::ignoreUnused(index);
}

const juce::String DelayAudioProcessor::getProgramName(int index) {
  juce::ignoreUnused(index);
  return {};
}

void DelayAudioProcessor::changeProgramName(int index,
                                            const juce::String& newName) {
  juce::ignoreUnused(index, newName);
}

//==============================================================================
void DelayAudioProcessor::prepareToPlay(double sampleRate,
                                        int samplesPerBlock) {
  // Use this method as the place to do any pre-playback
  // initialisation that you need..
  juce::ignoreUnused(samplesPerBlock);

  std::cout << "Sample rate: " << sampleRate << std::endl;
  currentTime_ = parameters_[DelayParameters::Time]->getValue();
}

void DelayAudioProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

bool DelayAudioProcessor::isBusesLayoutSupported(
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

void DelayAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                       juce::MidiBuffer& midiMessages) {
  juce::ignoreUnused(midiMessages);

  juce::ScopedNoDenormals noDenormals;

  auto mix = parameters_[DelayParameters::Mix]->getValue();
  auto time = parameters_[DelayParameters::Time]->getValue();
  auto feedback = parameters_[DelayParameters::Feedback]->getValue();

  auto num_samples = buffer.getNumSamples();
  auto inL = buffer.getReadPointer(0);
  auto inR = buffer.getReadPointer(1);
  auto outL = buffer.getWritePointer(0);
  auto outR = buffer.getWritePointer(1);

  while (num_samples--) {
    if (currentTime_ < time) {
      currentTime_ += 0.000005f;
    } else if (currentTime_ > time) {
      currentTime_ -= 0.000005f;
    }

    auto left = *inL++;
    auto right = *inR++;

    auto delayedL = delayLeft_.read(delayLeft_.size() * currentTime_ - 1);
    auto delayedR = delayRight_.read(delayRight_.size() * currentTime_ - 1);

    delayLeft_.write(left + delayedR * feedback);
    delayRight_.write(right + delayedL * feedback);

    *outL++ = delayedL * mix + left * (1 - mix);
    *outR++ = delayedR * mix + right * (1 - mix);
  }
}

//==============================================================================
bool DelayAudioProcessor::hasEditor() const {
  return true;  // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DelayAudioProcessor::createEditor() {
  return new DelayAudioProcessorEditor(*this);
}

//==============================================================================
void DelayAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
  std::unique_ptr<juce::XmlElement> xml(new juce::XmlElement("Delay"));
  xml->setAttribute("Mix", parameters_[DelayParameters::Mix]->getValue());
  xml->setAttribute("Time", parameters_[DelayParameters::Time]->getValue());
  xml->setAttribute("Feedback", parameters_[DelayParameters::Feedback]->getValue());
  copyXmlToBinary(*xml, destData);
}

void DelayAudioProcessor::setStateInformation(const void* data,
                                              int sizeInBytes) {
  std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

  if (xmlState.get() != nullptr)
    if (xmlState->hasTagName("Delay"))
    {
      parameters_[DelayParameters::Mix]->setValue(xmlState->getDoubleAttribute("Mix", 0.3));
      parameters_[DelayParameters::Time]->setValue(xmlState->getDoubleAttribute("Time", 0.5));
      parameters_[DelayParameters::Feedback]->setValue(xmlState->getDoubleAttribute("Feedback", 0.3));
    }
}

std::vector<AudioProcessorParameter*> DelayAudioProcessor::getParameters() {
  return parameters_;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new DelayAudioProcessor();
}
