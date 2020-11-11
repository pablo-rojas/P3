/// @file

#include <iostream>
#include <math.h>
#include "pitch_analyzer.h"

using namespace std;

/// Name space of UPC
namespace upc
{
  void PitchAnalyzer::autocorrelation(const vector<float> &x, vector<float> &r) const
  {
    for (unsigned int l = 0; l < r.size(); ++l)
    {
      for (unsigned int n = 0; n < x.size() - l; ++n)
      {
        r[l] += x[n] * x[n + l];
      }
    }

    if (r[0] == 0.0F) //to avoid log() and divide zero
      r[0] = 1e-10;
  }

  void PitchAnalyzer::set_window(Window win_type)
  {
    if (frameLen == 0)
      return;

    window.resize(frameLen);
    window.assign(frameLen, 0);

    switch (win_type)
    {
    case HAMMING:
      for (unsigned int i = 0; i < frameL; ++i)
        window[i] = (0.53836-0.46164*cos(2*i*M_1_PI/(frameL-1)));
      break;

    case RECT:
      for (unsigned int i = 0; i < frameL; ++i)
        window[i] = 1;
      break;

    default:
      window.assign(frameL, 1);
    }
  }

  void PitchAnalyzer::set_f0_range(float min_F0, float max_F0)
  {
    npitch_min = (unsigned int)samplingFreq / max_F0;
    if (npitch_min < 2)
      npitch_min = 2; // samplingFreq/2

    npitch_max = 1 + (unsigned int)samplingFreq / min_F0;

    //frameLen should include at least 2*T0
    if (npitch_max > frameLen / 2)
      npitch_max = frameLen / 2;
  }

  bool PitchAnalyzer::unvoiced(float pot, float r1norm, float rmaxnorm, float ZCR) const
  {
    //if (ZCR > ZCR_th) return true;
    if ((r1norm > r1_th) && (rmaxnorm > rmax_th)) return false;
    else return true;
  }

  float PitchAnalyzer::compute_pitch(vector<float> &x) const
  {

    // Step 1: Correlation Method
    vector<float> r(npitch_max);

    for (unsigned int l = 0; l < r.size(); ++l)
    {
      for (unsigned int n = 0; n < W - l; ++n)
      {
        r[l] += x[n] * x[n + l];
      }
    }

    // Step 2: Diference Function
    vector<float> d(npitch_max);
    float rt = 0;

    for(unsigned int l = 0; l < d.size(); l++){
      rt = 0;
      for (unsigned int n = l; n < l + W; ++n)
      {
        rt += x[n] * x[n];
      }
      d[l] = (r[0] + rt - 2 * r[l])/(W - l);
    }

    // Step 3: Cumulative Mean Normalized Difference Function
    float sum = 0;
    d[0] = 1;

    for (unsigned int l = 1; l < d.size(); l++){
      sum += d[l];
      d[l] = d[l]*l/sum;
    }

    // Step 4: Absolute Threshold
    unsigned int lag = npitch_min;
    bool check = true;
    float Dmin = 2;
    for (unsigned int i = npitch_min; ((i < d.size()) && check); ++i)
    {
      if (d[i] < Dmin) {
        Dmin = d[i];
        lag = i;
      }
      if (d[i] < 0.11){
        lag = i;
        check = false;
      }
    }
    
    float ZCR = 0;

    for (unsigned int n = 1; n < frameL; n++)
    {
      if (x[n] * x[n - 1] < 0)
        ZCR++;
    }
    ZCR = samplingFreq * ZCR / (2 * (frameL - 1));

    //Window input frame
    for (unsigned int i = 0; i < x.size(); ++i)
      x[i] *= window[i];
    r.assign(r.size(), 0);
    autocorrelation(x, r);
    float pot = 10 * log10(r[0]);




    //You can print these (and other) features, look at them using wavesurfer
    //Based on that, implement a rule for unvoiced
    //change to #if 1 and compile
#if 0
    if (r[0] > 0.0F)
      cout << pot << '\t' << r[1]/r[0] << '\t' << r[lag]/r[0] << endl;
#endif

    if (unvoiced(pot, r[1] / r[0], r[lag] / r[0], ZCR))
      return 0;
    else
      return (float)samplingFreq / (float)lag;
  }
} // namespace upc
