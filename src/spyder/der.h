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
#include "utils.h"

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
  Metrics(double duration, double miss, double falarm, double conf)
      : duration(duration),
        miss(miss),
        falarm(falarm),
        conf(conf),
        der(miss + falarm + conf) {}
  ~Metrics() {}
};

// Compute the evaluation regions based on the reference, hypothesis, and the UEM
// segments. \param ref: a list of reference turns \param hyp: a list of hypothesis
// turns \param uem: a list of UEM segments \param collar: the collar size in seconds
// \return a list of evaluation regions
std::vector<Region> get_eval_regions(TurnList& ref, TurnList& hyp, TurnList& uem);

// Compute the evaluation regions based on the reference, hypothesis, and the UEM
// segments. \param ref: a list of reference turns \param hyp: a list of hypothesis
// turns \param uem: a list of UEM segments \param collar: the collar size in seconds
// \return a list of scoring regions; this is different from the evaluation regions in 2
// ways:
// 1. The speaker labels in scoring regions are mapped to a common label space.
// 2. It uses collars, and so it is a subset of the evaluation regions.
std::vector<Region> get_score_regions(TurnList& ref, TurnList& hyp, TurnList& uem,
                                      float collar = 0.0);

// Compute diarization error rate with mapped turn lists.
// \param ref: a list of reference turns
// \param hyp: a list of hypothesis turns
// \param score_regions: a list of evaluation regions
// \param metrics: the DER metrics
// \param regions: the regions to compute DER for (e.g. "single", "overlap", etc.)
void compute_der_mapped(std::vector<Region>& score_regions, Metrics& metrics,
                        std::string regions);

// Compute diarization error rate. First the lists are mapped to a common
// label space using the Hungarian algorithm.
// \param ref: a list of reference turns
// \param hyp: a list of hypothesis turns
// \param uem: a list of UEM segments
// \param regions: the regions to compute DER for (e.g. "single", "overlap", etc.)
// \param collar: the collar size in seconds
Metrics compute_der(TurnList& ref, TurnList& hyp, TurnList& uem,
                    std::string regions = "all", float collar = 0.0);

}  // end namespace spyder

#endif
