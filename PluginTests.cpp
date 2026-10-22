#include "Plugin.h"
#include <iostream>
#include <cstdlib>
void check(bool b,const char* m){if(!b){std::cerr<<m<<"\n";std::exit(1);}}
int main(){
 Processor a; a.prepareToPlay(48000,256);a.controls.running=true;a.controls.value[gsx::Frequency]=440;a.controls.step[gsx::Delay]=-2;
 juce::MemoryBlock state;a.getStateInformation(state);Processor b;b.setStateInformation(state.getData(),int(state.getSize()));
 check(b.controls.value[gsx::Frequency]==440 && b.controls.step[gsx::Delay]==-2 && b.controls.running,"State round trip");
 b.prepareToPlay(48000,256);juce::AudioBuffer<float> buffer(2,256);juce::MidiBuffer midi;double energy=0;
 for(int block=0;block<100;++block){b.processBlock(buffer,midi);for(int i=0;i<256;++i){float x=buffer.getSample(0,i);check(std::isfinite(x)&&std::abs(x)<=1,"Plugin output bounds");check(x==buffer.getSample(1,i),"Stereo outputs");energy+=x*x;}}
 check(energy>0,"Plugin generates audio");b.setStateInformation("bad",3);check(b.controls.value[gsx::Frequency]==440,"Invalid state ignored");
 std::cout<<"Plugin state and render tests passed\n";
}
