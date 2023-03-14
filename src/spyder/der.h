// spyder/der.h

// Copyright 2021  Johns Hopkins University (Author: Desh Raj)

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef SPYDER_DER_H
#define SPYDER_DER_H

#include <vector>

#include "containers.h"

namespace spyder {

// The DER metrics: missed speech, false alarm, speaker confusion (error),
// and diarization error rate (DER).
class Metrics {
 public:
  double duration;
  double miss;
  double falarm;
  double conf;
  double der;
  Metrics() {}
  Metrics(double duration, double miss, double falarm, double conf) : duration(duration), miss(miss), falarm(falarm), conf(conf), der(miss + falarm + conf) {}
  ~Metrics() {}
};

// Compute intersection length of two turn tuples.
// \param A: a Turn tuple
// \param B: a Turn tuple
double compute_intersection_length(Turn A, Turn B);

// Build cost matrix given reference and hypothesis lists.
// \param ref: a list of reference turns
// \param hyp: a list of hypothesis turns
std::vector<std::vector<double>> build_cost_matrix(TurnList& ref, TurnList& hyp);

// Map reference and hypothesis labels to common space based on assignment
// vector.
// \param ref, reference list of turns
// \param hyp, hypothesis list of turns
// \param assignment, vector of assignments from ref to hyp
void map_labels(TurnList& ref, TurnList& hyp, std::vector<int> assignment);

// Compute diarization error rate with mapped turn lists.
// \param ref: a list of reference turns
// \param hyp: a list of hypothesis turns
void compute_der_mapped(TurnList& ref, TurnList& hyp, Metrics& metrics);

// Compute diarization error rate. First the lists are mapped to a common
// label space using the Hungarian algorithm.// \param ref: a list of reference turns
// \param hyp: a list of hypothesis turns
Metrics compute_der(TurnList& ref, TurnList& hyp, TurnList& uem, std::string regions = "all", float collar = 0.0);

}  // end namespace spyder

#endif
