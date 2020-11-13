/// @file

#ifndef PITCH_ANALYZER_H
#define PITCH_ANALYZER_H

#include <vector>
#include <algorithm>

namespace upc {
  const float MIN_F0 = 40.0F;    ///< Minimum value of pitch in Hertzs
  const float MAX_F0 = 800.0F; ///< Maximum value of pitch in Hertzs
  const float THRESHOLD = 0.01; ///< Threshold for the YIN PDA
  const float ZCR_THRESHOLD = 0.00;
  const float P_THRESHOLD = -44;
  const float RMAX_THRESHOLD = 0.36;
  const float R1_THRESHOLD = 0.69;

  ///
  /// PitchAnalyzer: class that computes the pitch (in Hz) from a signal frame.
  /// No pre-processing or post-processing has been included
  ///
  class PitchAnalyzer {
  public:
	/// Wndow type
    enum Window {
		RECT, 						///< Rectangular window
		HAMMING						///< Hamming window
	};

    void set_window(Window type); ///< pre-compute window

  private:
    std::vector<float> window; ///< precomputed window
    unsigned int frameLen, ///< length of frame (in samples). Has to be set in the constructor call
      samplingFreq, ///< sampling rate (in samples per second). Has to be set in the constructor call
      npitch_min, ///< minimum value of pitch period, in samples
      npitch_max, ///< maximum value of pitch period, in samples
      W, ///< window size for acf and amdf, in samples
      frameL; 
      float th, ZCR_th, P_th, rmax_th, r1_th; ///< Threshold for step 4
 
	///
	/// Computes correlation from lag=0 to r.size()
	///
    void autocorrelation(const std::vector<float> &x, std::vector<float> &r) const;

	///
	/// Returns the pitch (in Hz) of input frame x
	///
    float compute_pitch(std::vector<float> & x) const;
	
	///
	/// Returns true is the frame is unvoiced
	///
    bool unvoiced(float pot, float r1norm, float rmaxnorm, float ZCR) const;

//           float threshold = THRESHOLD, ///< Threshold for Step 4
  public:
    PitchAnalyzer(	unsigned int fLen,			///< Frame length in samples
					unsigned int sFreq,			///< Sampling rate in Hertzs
					Window w=PitchAnalyzer::RECT,	///< Window type
					float min_F0 = MIN_F0,		///< Pitch range should be restricted to be above this value
					float max_F0 = MAX_F0,	///< Pitch range should be restricted to be below this value
          float zcr = ZCR_THRESHOLD,
          float p = P_THRESHOLD,
          float rmax = RMAX_THRESHOLD,
          float r1 = R1_THRESHOLD
				 )
	{
      ZCR_th = zcr*sFreq;
      P_th = p;
      rmax_th = rmax;
      r1_th = R1_THRESHOLD;
      th = THRESHOLD;
      frameLen = fLen;
      samplingFreq = sFreq;
      W = (unsigned int) (0.025*sFreq);
      frameL = (unsigned int) (0.03*sFreq);
      set_f0_range(min_F0, max_F0);
      set_window(w);
    }

	///
    /// Operator (): computes the pitch for the given vector x
	///
    float operator()(const std::vector<float> & _x) const {
      if (_x.size() != frameLen)
        return -1.0F;

      std::vector<float> x(_x); //local copy of input frame
      return compute_pitch(x);
    }

	///
    /// Operator (): computes the pitch for the given "C" vector (float *).
    /// N is the size of the vector pointer by pt.
	///
    float operator()(const float * pt, unsigned int N) const {
      if (N != frameLen)
        return -1.0F;

      std::vector<float> x(N); //local copy of input frame, size N
      std::copy(pt, pt+N, x.begin()); ///copy input values into local vector x
      return compute_pitch(x);
    }

	///
    /// Operator (): computes the pitch for the given vector, expressed by the begin and end iterators
	///
    float operator()(std::vector<float>::const_iterator begin, std::vector<float>::const_iterator end) const {

      if (end-begin != frameLen)
        return -1.0F;

      std::vector<float> x(end-begin); //local copy of input frame, size N
      std::copy(begin, end, x.begin()); //copy input values into local vector x
      return compute_pitch(x);
    }
    
	///
    /// Sets pitch range: takes min_F0 and max_F0 in Hz, sets npitch_min and npitch_max in samples
	///
    void set_f0_range(float min_F0, float max_F0);
  };
}
#endif
