#include "Engine.h"
#include <iostream>
#include <limits>
#include <cstdlib>
void check(bool x,const char* msg){if(!x){std::cerr<<msg<<"\n";std::exit(1);}}
int main(){
 for(int i=0;i<gsx::Count;++i) {
  if(i==gsx::Ramp) continue;
  check(gsx::rampValue(gsx::minima[i]+1,-1,1,i)==gsx::maxima[i],"Exact lower-bound wrap");
  check(gsx::rampValue(gsx::minima[i]+1,-2,1,i)==gsx::maxima[i]-1,"Wrap preserves overshoot");
  check(gsx::rampValue(gsx::minima[i],0,1,i)==gsx::minima[i],"Zero step stays at minimum");
  check(gsx::rampValue(gsx::minima[i],-1,0,i)==gsx::minima[i],"Zero INC stays at minimum");
  check(gsx::rampValue(gsx::minima[i]+1,1,-1,i)==gsx::maxima[i],"Negative INC wraps");
 }
 check(gsx::rampValue(1,-130,1,gsx::Voices)==63,"Multiple wraps");
 {gsx::Engine wrapped;gsx::Controls c;wrapped.prepare(48000);c.running=true;c.value[gsx::Voices]=1;c.step[gsx::Voices]=-1;c.value[gsx::Ramp]=2;c.step[gsx::Ramp]=-1;wrapped.process(c);check(c.value[gsx::Voices]==64,"Engine voice wrap");check(c.value[gsx::Ramp]==1,"RAMP stops at 1 ms");}

 // Exercise every parameter through repeated fractional downward ramps.
 for(int param=0;param<gsx::Count;++param) {
  gsx::Engine engine;gsx::Controls c;engine.prepare(48000);c.running=true;
  c.value[gsx::Ramp]=1;c.value[param]=gsx::minima[param]+.02f;c.step[param]=-.1f;c.increment=.2f;
  engine.process(c);
  // Floating-point residuals may require one more tick; force the next due time
  // by processing enough samples for the longest allowed ramp period.
  for(int i=0;i<480001;++i) engine.process(c);
  if(param==gsx::Ramp) check(c.value[param]==1,"Repeated negative RAMP stays at 1 ms");
  else check(c.value[param]>gsx::minima[param],"Fractional ramp must not stick at lower bound");
 }
 check(gsx::Engine::envelope(0)==0 && gsx::Engine::envelope(.25)==1 && gsx::Engine::envelope(.75)==1 && gsx::Engine::envelope(1)==0,"Envelope shape");
 for(double sr:{44100.,48000.,96000.}) {
  gsx::Engine e;gsx::Controls c;e.prepare(sr);c.running=true;c.value[gsx::Voices]=64;c.value[gsx::Duration]=1;c.value[gsx::DurationRange]=20000;c.value[gsx::FrequencyRange]=40000;c.value[gsx::Delay]=0;c.value[gsx::SquareVoices]=20;c.value[gsx::SawVoices]=20;
  double energy=0;for(int i=0;i<int(sr);++i){float x=e.process(c);check(std::isfinite(x)&&std::abs(x)<=1,"Finite bounded output");energy+=x*x;}check(energy>0,"Audible output");
  c.running=false;for(int i=0;i<int(sr*.03);++i)e.process(c);check(e.process(c)==0,"Stop fades to silence");
  c.value[gsx::Voices]=0;c.running=true;for(int i=0;i<1000;++i)check(e.process(c)==0,"Zero voices silence");
 }
 gsx::Engine e;gsx::Controls c;e.prepare(48000);c.running=true;c.value[gsx::Ramp]=10;c.step[gsx::Frequency]=2;c.increment=3;
 for(int i=0;i<480;++i)e.process(c);check(c.value[gsx::Frequency]==206,"First ramp");e.process(c);check(c.value[gsx::Frequency]==212,"Second ramp");
 c.value[gsx::Duration]=std::numeric_limits<float>::quiet_NaN();for(int i=0;i<48000;++i)check(std::isfinite(e.process(c)),"Invalid input guard");
 std::cout<<"PASS: envelope, 64 voices, extreme ranges, 3 sample rates, stop, zero voices, ramps, NaN guard\n";
}
