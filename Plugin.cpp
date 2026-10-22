#include "Plugin.h"
class Main : public juce::AudioProcessorEditor, private juce::Timer {
 Processor& processor; gsx::Controls& controls;
 std::array<juce::Slider,gsx::Count> values,steps;
 std::array<juce::Label,gsx::Count> labels;
 juce::Label title,description,headings,status,incLabel,volumeLabel;
 juce::Slider increment,volume;
 juce::TextButton start{"Start"},reset{"Reset"},randomize{"Randomize"};
 juce::ComboBox randomMode;
 juce::Random random;
public:
 Main(Processor& p) : AudioProcessorEditor(p), processor(p), controls(p.controls) {
  const char* names[]={"FREQ (Hz)","FRQ.RNG (Hz)","DURATION (ms)","DUR.RNG (ms)","DELAY (ms)","RAMP (ms)","SQUARE VOICES","SAW VOICES","TOTAL VOICES"};
  auto label=[this](juce::Label& l,juce::String s){l.setText(s,juce::dontSendNotification);l.setColour(juce::Label::textColourId,juce::Colour(0xffe8ae26));addAndMakeVisible(l);};
  label(title,"ODE TO GSX"); title.setFont(juce::FontOptions(27.f));
  label(description,"sine + square + saw  /  64 voices  /  v0.2.1");
  label(headings,"PARAMETER                                      VALUE                           STEP x INC");
  for(int i=0;i<gsx::Count;++i) {
   label(labels[i],names[i]);
   auto setup=[this](juce::Slider& s,double lo,double hi,double interval){s.setSliderStyle(juce::Slider::IncDecButtons);s.setTextBoxStyle(juce::Slider::TextBoxLeft,false,110,28);s.setRange(lo,hi,interval);addAndMakeVisible(s);};
   setup(values[i],gsx::minima[i],gsx::maxima[i],.001);
   setup(steps[i],-1000,1000,.1);
   values[i].setValue(controls.value[i].load(),juce::dontSendNotification);
   steps[i].setValue(controls.step[i].load(),juce::dontSendNotification);
   values[i].onValueChange=[this,i]{controls.value[i]=float(values[i].getValue());};
   steps[i].onValueChange=[this,i]{controls.step[i]=float(steps[i].getValue());};
  }
  label(incLabel,"INC");label(volumeLabel,"OUTPUT");
  increment.setSliderStyle(juce::Slider::IncDecButtons);increment.setTextBoxStyle(juce::Slider::TextBoxLeft,false,90,28);increment.setRange(-100,100,.1);increment.setValue(controls.increment.load());increment.onValueChange=[this]{controls.increment=float(increment.getValue());};addAndMakeVisible(increment);
  volume.setRange(0,1,.001);volume.setValue(controls.volume.load());volume.setTextBoxStyle(juce::Slider::TextBoxRight,false,60,28);volume.onValueChange=[this]{controls.volume=float(volume.getValue());};addAndMakeVisible(volume);
  addAndMakeVisible(start);start.onClick=[this]{bool run=!controls.running.load();controls.running=run;start.setButtonText(run?"Stop":"Start");};
  addAndMakeVisible(reset); reset.onClick=[this]{
   const juce::ScopedLock lock(processor.getCallbackLock());
   for(int i=0;i<gsx::Count;++i) { setStep(i,0); setValue(i,gsx::defaults[i]); }
   controls.increment=1; increment.setValue(1,juce::dontSendNotification);
   controls.volume=.2f; volume.setValue(.2,juce::dontSendNotification);
  };
  addAndMakeVisible(randomMode); randomMode.addItem("ALL",1);randomMode.addItem("STEP",2);randomMode.addItem("VALUES",3);randomMode.setSelectedId(1);
  randomMode.setTooltip("ALL: values and steps. STEP: right-hand steps only. VALUES: left-hand values and INC. Output level is preserved.");
  addAndMakeVisible(randomize);randomize.onClick=[this]{randomizeControls();};
  label(status,"Stopped. Press Start to hear the synth.");
  setSize(710,700);startTimerHz(20);
 }
 ~Main() override {stopTimer();}
 void setValue(int i,float x) { controls.value[i]=gsx::valid(x,i); values[i].setValue(controls.value[i].load(),juce::dontSendNotification); }
 void setStep(int i,float x) { controls.step[i]=x; steps[i].setValue(x,juce::dontSendNotification); }
 float between(float lo,float hi) { return lo+random.nextFloat()*(hi-lo); }
 void randomizeControls() {
  // UI batch changes are serialized with the audio callback, never from it.
  const juce::ScopedLock lock(processor.getCallbackLock());
  int mode=randomMode.getSelectedId();
  if(mode==1||mode==3) {
   float f=std::round(std::exp(between(std::log(40.f),std::log(2000.f))));
   float d=std::round(between(8,250));
   setValue(gsx::Frequency,f);setValue(gsx::FrequencyRange,std::round(between(0,f*1.5f)));
   setValue(gsx::Duration,d);setValue(gsx::DurationRange,std::round(between(0,d)));
   setValue(gsx::Delay,std::round(between(0,300)));setValue(gsx::Ramp,std::round(between(50,1000)));
   int n=1+random.nextInt(64), sq=random.nextInt(n+1), saw=random.nextInt(n-sq+1);
   setValue(gsx::Voices,float(n));setValue(gsx::SquareVoices,float(sq));setValue(gsx::SawVoices,float(saw));
   float inc=float(1+random.nextInt(4));controls.increment=inc;increment.setValue(inc,juce::dontSendNotification);
  }
  if(mode==1||mode==2) {
   const float magnitude[]={5,5,2,2,2,5,1,1,1};
   bool any=false;
   for(int i=0;i<gsx::Count;++i) {float x=random.nextInt(4)==0?0:float(random.nextInt(11)-5)*magnitude[i]/5; x=std::round(x*10)/10;setStep(i,x);any|=x!=0;}
   if(!any) setStep(gsx::Frequency,1);
  }
 }
 void paint(juce::Graphics& g) override {g.fillAll(juce::Colour(0xff282018));}
 void resized() override {
  title.setBounds(24,15,660,40);description.setBounds(24,57,660,25);headings.setBounds(24,95,660,25);
  for(int i=0;i<gsx::Count;++i){int y=130+i*42;labels[i].setBounds(24,y,240,30);values[i].setBounds(275,y,180,30);steps[i].setBounds(490,y,180,30);}
  incLabel.setBounds(24,517,50,30);increment.setBounds(75,517,150,30);volumeLabel.setBounds(275,517,80,30);volume.setBounds(355,517,315,30);
  start.setBounds(24,565,200,38);reset.setBounds(464,565,206,38);randomize.setBounds(24,615,200,34);randomMode.setBounds(244,615,200,34);status.setBounds(24,660,660,28);
 }
 static bool editingValue(const juce::Slider& slider) {
  for(int i=0;i<slider.getNumChildComponents();++i)
   if(auto* label=dynamic_cast<juce::Label*>(slider.getChildComponent(i)))
    if(label->isBeingEdited()) return true;
  return false;
 }
 void timerCallback() override {
  for(int i=0;i<gsx::Count;++i) if(!values[i].isMouseButtonDown()&&!editingValue(values[i])) values[i].setValue(controls.value[i].load(),juce::dontSendNotification);
  for(int i=0;i<gsx::Count;++i) if(!steps[i].isMouseButtonDown()&&!editingValue(steps[i])) steps[i].setValue(controls.step[i].load(),juce::dontSendNotification);
  if(!increment.isMouseButtonDown()&&!editingValue(increment)) increment.setValue(controls.increment.load(),juce::dontSendNotification);
  if(!volume.isMouseButtonDown()&&!editingValue(volume)) volume.setValue(controls.volume.load(),juce::dontSendNotification);
  start.setButtonText(controls.running.load()?"Stop":"Start");
  status.setText(controls.running.load()?"Playing — free-running grains":"Stopped — press Start. Standalone audio settings are in Options.",juce::dontSendNotification);
 }
};

juce::AudioProcessorEditor* Processor::createEditor() { return new Main(*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new Processor; }
