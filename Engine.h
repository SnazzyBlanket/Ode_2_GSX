#pragma once
#include "Synthesis/oscillator.h"
#include <array>
#include <atomic>
#include <algorithm>
#include <cmath>
#include <cstdint>
namespace gsx {
enum Param { Frequency, FrequencyRange, Duration, DurationRange, Delay, Ramp, SquareVoices, SawVoices, Voices, Count };
inline constexpr std::array<float, Count> defaults {200,0,100,0,200,250,0,0,1};
inline constexpr std::array<float, Count> minima {0,0,1,0,0,1,0,0,0};
inline constexpr std::array<float, Count> maxima {20000,40000,10000,20000,10000,10000,64,64,64};
inline float valid(float x, int i) { return std::isfinite(x) ? std::clamp(x,minima[i],maxima[i]) : defaults[i]; }
// Downward ramps wrap on reaching the minimum; retain overshoot for large steps.
// Explicit edits/reset may still select zero. Upward ramps retain their upper cap.
inline float rampValue(float current, float step, float increment, int i) {
 const double delta=double(step)*increment;
 const double next=double(valid(current,i))+delta;
 if(!std::isfinite(next)) return valid(current,i);
 if(i!=Ramp && delta<0 && next<=minima[i]) {
  const double span=double(maxima[i])-minima[i];
  return float(maxima[i]-std::fmod(double(minima[i])-next,span));
 }
 return float(std::clamp(next,double(minima[i]),double(maxima[i])));
}
struct Controls {
 std::array<std::atomic<float>, Count> value{}, step{};
 std::atomic<float> increment{1}, volume{0.2f};
 std::atomic<bool> running{false};
 Controls() { for(int i=0;i<Count;++i) { value[i]=defaults[i]; step[i]=0; } }
};
class Engine {
 struct Voice { daisysp::Oscillator osc; double age=0, length=1, wait=0; float gain=0; bool active=false; };
 std::array<Voice,64> voice{};
 double rate=48000, rampWait=0;
 bool wasRunning=false;
 float level=0;
 uint32_t randomState=0x47535801;
 float random() { randomState^=randomState<<13; randomState^=randomState>>17; randomState^=randomState<<5; return float(randomState>>8)/16777216.f; }
 void resetVoices() { for(auto& v:voice) { v.osc.Init(float(rate)); v.osc.SetFreq(0); v.osc.SetAmp(1); v.age=0; v.wait=rate*.01; v.active=false; } rampWait=0; randomState=0x47535801; }
public:
 void prepare(double sr) { rate=std::isfinite(sr)&&sr>=8000?sr:48000; wasRunning=false; level=0; resetVoices(); }
 static float envelope(double t) { return float(std::max(0.0,std::min({1.0,t*4,(1-t)*4}))); }
 float process(Controls& c) {
  bool run=c.running.load(std::memory_order_relaxed);
  if(run&&!wasRunning) { resetVoices(); for(auto& v:voice) v.wait+=valid(c.value[Delay].load(),Delay)*rate/1000; }
  wasRunning=run;
  if(run && rampWait<=0) {
   float inc=c.increment.load(); if(!std::isfinite(inc)) inc=0;
   for(int i=0;i<Count;++i) { float old=c.value[i].load(); float s=c.step[i].load();
    if(std::isfinite(s)&&s!=0) c.value[i].compare_exchange_strong(old,rampValue(old,s,inc,i));
   }
   rampWait=valid(c.value[Ramp].load(),Ramp)*rate/1000;
  }
  if(run) --rampWait;
  const int n=int(valid(c.value[Voices].load(),Voices));
  const int sq=int(valid(c.value[SquareVoices].load(),SquareVoices));
  const int saw=int(valid(c.value[SawVoices].load(),SawVoices));
  float sum=0;
  for(int i=0;i<64;++i) {
   auto& v=voice[i];
   if(run && i<n && !v.active) {
    if(v.wait<=0) {
     float hz=valid(c.value[Frequency].load(),Frequency)+(random()-.5f)*valid(c.value[FrequencyRange].load(),FrequencyRange);
     v.osc.SetFreq(std::clamp(std::abs(hz),0.f,float(rate*.45)));
     float ms=valid(c.value[Duration].load(),Duration)+(random()-.5f)*valid(c.value[DurationRange].load(),DurationRange);
     v.length=std::max(1.0,double(std::clamp(ms,1.f,20000.f))*rate/1000);
     v.age=0; v.gain=1.f/std::max(1,n); v.active=true;
    } else --v.wait;
   }
   v.osc.SetWaveform(i<sq?daisysp::Oscillator::WAVE_POLYBLEP_SQUARE:i<sq+saw?daisysp::Oscillator::WAVE_POLYBLEP_SAW:daisysp::Oscillator::WAVE_SIN);
   float sample=v.osc.Process();
   if(v.active) {
    sum+=sample*envelope(v.age/v.length)*v.gain;
    if(++v.age>=v.length) {v.active=false; v.wait=valid(c.value[Delay].load(),Delay)*rate/1000;}
   }
  }
  float target=run?std::clamp(c.volume.load(),0.f,1.f):0.f;
  if(!std::isfinite(target)) target=0;
  level+=std::clamp(target-level,-float(1/(rate*.02)),float(1/(rate*.02)));
  return std::clamp(sum*level,-1.f,1.f);
 }
};
}
