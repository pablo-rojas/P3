/// @file

#include <iostream>
#include <fstream>
#include <string.h>
#include <errno.h>
#include <math.h>

#include "wavfile_mono.h"
#include "pitch_analyzer.h"

#include "docopt.h"

#define FRAME_LEN   0.050 /* 50 ms. */
#define FRAME_SHIFT 0.015 /* 15 ms. */
#define FILTER_LEN 0.001 /* 1 ms. */

using namespace std;
using namespace upc;

static const char USAGE[] = R"(
get_pitch - Pitch Detector 

Usage:
    get_pitch [--power <pot>] [--rmaxnorm <rmax>] [--r1norm <r1>] [--zcr <rmax>] <input-wav> <output-txt>
    get_pitch (-h | --help)
    get_pitch --version

Options:
    -h, --help  Show this screen
    --version   Show the version of the project
    --power <pot>      Power threshold for the detector [default: -44]
    --rmaxnorm <rmax>  Normalized rmax threshold for the detector [default: 0.36]
    --r1norm <r1>      Normalized r1 threshold for the detector [default: 0.69]
    --zcr <z>       Normalized rmax threshold for the detector [default: 0.0]

Arguments:
    input-wav   Wave file with the audio signal
    output-txt  Output file: ASCII file with the result of the detection:
                    - One line per frame with the estimated f0
                    - If considered unvoiced, f0 must be set to f0 = 0
)";

int main(int argc, const char *argv[]) {
	/// \TODO 
	///  Modify the program syntax and the call to **docopt()** in order to
	///  add options and arguments to the program.
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE,
        {argv + 1, argv + argc},	// array of arguments, without the program name
        true,    // show help if requested
        "2.0");  // version string

	std::string input_wav = args["<input-wav>"].asString();
	std::string output_txt = args["<output-txt>"].asString();

  // Read input sound file
  unsigned int rate;
  vector<float> x;
  if (readwav_mono(input_wav, rate, x) != 0) {
    cerr << "Error reading input file " << input_wav << " (" << strerror(errno) << ")\n";
    return -2;
  }

  int n_len = rate * FRAME_LEN;
  int n_shift = rate * FRAME_SHIFT;
  unsigned int n_filter = rate * FILTER_LEN;

  // Define analyzer

    float pot = std::stof(args["--power"].asString());
    float rmax = std::stof(args["--rmaxnorm"].asString());
    float r1 = std::stof(args["--r1norm"].asString());
    float zcr = std::stof(args["--zcr"].asString());
    PitchAnalyzer analyzer(n_len, rate, PitchAnalyzer::RECT, 50, 500, zcr, pot, rmax, r1);

  /// \TODO
  /// Preprocess the input signal in order to ease pitch estimation. For instance,
  /// central-clipping or low pass filtering may be used.


  vector<float> h_i(n_filter);
  h_i.assign(n_filter,0);
  unsigned int k_max = (unsigned int) 1000*n_filter/rate;

  for (unsigned int n = 0; n < n_filter; n++){
    for (unsigned int k = 0; k < k_max; k++){
      h_i[n] = h_i[n] + cos(2*M_1_PI*k*n/n_filter)/n_filter;
    }
  }

  vector<float> y(x.size());
  y.assign(x.size(),0);
  for (unsigned int n = 0; n < x.size(); n++){
    for (unsigned int m = n; (n - m < n_filter) && (m < x.size()); m++){
      y[n] = y[n] + x[m]*h_i[n-m];
    }
    x[n] = y[n];
  }


  // Iterate for each frame and save values in f0 vector
  vector<float>::iterator iX;
  vector<float> f0;
  for (iX = x.begin(); iX + n_len < x.end(); iX = iX + n_shift) {
    float f = analyzer(iX, iX + n_len);
    f0.push_back(f);
  }

  /// \TODO
  /// Postprocess the estimation in order to supress errors. For instance, a median filter
  /// or time-warping may be used.

  vector<float> fo(f0.size());
  unsigned int n_max;
  unsigned int n_min;
  float mean = 0;
  unsigned int num = 0;

  for (unsigned int n = 1; n < f0.size()-1; n++){
    if ((f0[n] != 0) && ((f0[n-1] != 0) || (f0[n+1] != 0) )) {
      n_max = n-1;
      n_min = n-1;
      if (f0[n] > f0[n_max]) n_max = n;
      else if (f0[n] < f0[n_min]) n_min = n;

      if (f0[n+1] > f0[n_max]) n_max = n+1;
      else if (f0[n+1] < f0[n_min]) n_min = n+1;

      for (unsigned int i = n-1; i <= n+1; i++){
        if ((i != n_max) && (i != n_min)) {
          fo[n] = f0[i];
        }
      }
    } else {
      fo[n] = f0[n];
    }
    if (fo[n] != 0){
      mean = mean + fo[n];
      num++;
    }
  } 
  mean = mean/num;

  for (unsigned int n = 1; n < f0.size()-1; n++){
    if ((fo[n] >= 500.0) || (abs(fo[n] - mean)/mean > 0.44)){
      f0[n] = 0;
    } else {
      f0[n] = fo[n];
    }
  } 


  // Write f0 contour into the output file
  ofstream os(output_txt);
  if (!os.good()) {
    cerr << "Error reading output file " << output_txt << " (" << strerror(errno) << ")\n";
    return -3;
  }

  os << 0 << '\n'; //pitch at t=0
  for (iX = f0.begin(); iX != f0.end(); ++iX) 
    os << *iX << '\n';
  os << 0 << '\n';//pitch at t=Dur
  os << 0 << '\n';//pitch at t=Dur

  return 0;
}
