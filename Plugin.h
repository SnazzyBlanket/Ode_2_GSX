#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "Engine.h"
class Processor final : public juce::AudioProcessor {
 gsx::Engine engine;
public:
 gsx::Controls controls;
 Processor():AudioProcessor(BusesProperties().withOutput("Output",juce::AudioChannelSet::stereo(),true)) {}
 const juce::String getName() const override {return "ODE TO GSX";}
 void prepareToPlay(double sr,int) override {engine.prepare(sr);}
 void releaseResources() override {}
 bool isBusesLayoutSupported(const BusesLayout& l) const override {return l.getMainInputChannelSet().isDisabled() && (l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo() || l.getMainOutputChannelSet()==juce::AudioChannelSet::mono());}
 void processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer& midi) override {
  juce::ScopedNoDenormals guard; midi.clear();
  for(int i=0;i<b.getNumSamples();++i) {float x=engine.process(controls);for(int ch=0;ch<b.getNumChannels();++ch)b.setSample(ch,i,x);}
 }
 bool hasEditor() const override {return true;}
 juce::AudioProcessorEditor* createEditor() override;
 bool acceptsMidi() const override {return false;}
 bool producesMidi() const override {return false;}
 double getTailLengthSeconds() const override {return 20;}
 int getNumPrograms() override {return 1;}
 int getCurrentProgram() override {return 0;}
 void setCurrentProgram(int) override {}
 const juce::String getProgramName(int) override {return "Default";}
 void changeProgramName(int,const juce::String&) override {}
 void getStateInformation(juce::MemoryBlock& dest) override {
  const juce::ScopedLock lock(getCallbackLock());
  juce::XmlElement xml("ODE_TO_GSX");xml.setAttribute("version",1);
  for(int i=0;i<gsx::Count;++i) {xml.setAttribute("value"+juce::String(i),double(controls.value[i].load()));xml.setAttribute("step"+juce::String(i),double(controls.step[i].load()));}
  xml.setAttribute("increment",double(controls.increment.load()));xml.setAttribute("volume",double(controls.volume.load()));xml.setAttribute("running",controls.running.load());copyXmlToBinary(xml,dest);
 }
 void setStateInformation(const void* data,int size) override {
  auto xml=getXmlFromBinary(data,size);if(!xml || !xml->hasTagName("ODE_TO_GSX")) return;
  const juce::ScopedLock lock(getCallbackLock());
  auto finite=[](double v,double lo,double hi,double fallback){return float(std::isfinite(v)?std::clamp(v,lo,hi):fallback);};
  for(int i=0;i<gsx::Count;++i) {controls.value[i]=gsx::valid(float(xml->getDoubleAttribute("value"+juce::String(i),gsx::defaults[i])),i);controls.step[i]=finite(xml->getDoubleAttribute("step"+juce::String(i),0),-1000,1000,0);}
  controls.increment=finite(xml->getDoubleAttribute("increment",1),-100,100,1);controls.volume=finite(xml->getDoubleAttribute("volume",.2),0,1,.2);controls.running=xml->getBoolAttribute("running",false);
 }
};
