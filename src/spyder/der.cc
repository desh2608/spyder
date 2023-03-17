// spyder/der.cc

// Copyright 2021  Johns Hopkins University (Author: Desh Raj)

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef SPYDER_DER_CC
#define SPYDER_DER_CC

#include "der.h"

#include <algorithm>
#include <map>
#include <set>
#include <unordered_set>
#include <vector>

#include "float.h"
#include "hungarian.h"

namespace spyder {

void compute_der_mapped(std::vector<Region> &score_regions, Metrics &metrics,
                        std::string region_type) {
  double miss = 0, falarm = 0, conf = 0, total_dur = 0, scored_dur = 0, dur;
  int N_ref, N_hyp, N_correct;
  for (auto &region : score_regions) {
    dur = region.duration();
    N_ref = region.ref_spk.size();
    N_hyp = region.hyp_spk.size();
    if (!(region_type == ALL) && !(region_type == SINGLE && N_ref == 1) &&
        !(region_type == NONOVERLAP && N_ref <= 1) && !(region_type == OVERLAP && N_ref > 1))
      continue;
    N_correct = region.num_correct();
    miss += dur * (std::max(0, N_ref - N_hyp));
    falarm += dur * (std::max(0, N_hyp - N_ref));
    conf += dur * (std::min(N_ref, N_hyp) - N_correct);
    total_dur += dur * N_ref;
    scored_dur += dur;
  }
  // free up memory
  std::vector<Region>().swap(score_regions);

  metrics.duration = total_dur;
  if (total_dur == 0) {
    metrics.miss = 0;
    metrics.falarm = 0;
    metrics.conf = 0;
    metrics.der = 0;
  } else {
    metrics.miss = miss / total_dur;
    metrics.falarm = falarm / total_dur;
    metrics.conf = conf / total_dur;
    metrics.der = (miss + falarm + conf) / total_dur;
  }
  return;
}

Metrics compute_der(TurnList &ref, TurnList &hyp, TurnList &uem, std::string regions,
                    float collar) {
  // Merge overlapping segments from the same speaker.
  ref.merge_same_speaker_turns();
  hyp.merge_same_speaker_turns();
  uem.merge_same_speaker_turns();

  // Obtain the evaluation regions based on the UEM
  std::vector<Region> eval_regions = get_eval_regions(ref, hyp, uem);

  // Map the reference and hypothesis speakers to the same labels.
  ref.build_speaker_index();
  hyp.build_speaker_index();
  std::vector<std::vector<double>> cost_matrix = build_cost_matrix(ref, hyp, eval_regions);
  HungarianAlgorithm hungarian_solver;
  std::vector<int> assignment;
  double cost = hungarian_solver.Solve(cost_matrix, assignment);

  std::map<std::string, std::string> ref_map, hyp_map;
  map_labels(ref, hyp, assignment, ref_map, hyp_map);

  // Obtain scoring regions based on collar
  if (collar != 0.0) {
    add_collar_to_uem(uem, ref, collar);
  }
  eval_regions = get_eval_regions(ref, hyp, uem);

  // Finally, we compute the DER metrics.
  Metrics metrics;
  metrics.ref_map = ref_map;
  metrics.hyp_map = hyp_map;
  compute_der_mapped(eval_regions, metrics, regions);
  return metrics;
}

}  // end namespace spyder

#endif
