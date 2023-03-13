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

double compute_intersection_length(Turn A, Turn B) {
  double max_start = std::max(A.start, B.start);
  double min_end = std::min(A.end, B.end);
  return std::max(0.0, min_end - max_start);
}

std::vector<std::vector<double>> build_cost_matrix(TurnList &ref,
                                                   TurnList &hyp) {
  int M = ref.forward_index.size();
  int N = hyp.forward_index.size();
  std::vector<std::vector<double>> cost_matrix(M, std::vector<double>(N));

  int i, j;
  for (auto &ref_turn : ref.turns) {
    for (auto &hyp_turn : hyp.turns) {
      i = ref.forward_index.find(ref_turn.spk)->second;
      j = hyp.forward_index.find(hyp_turn.spk)->second;
      cost_matrix[i][j] -= compute_intersection_length(ref_turn, hyp_turn);
    }
  }
  return cost_matrix;
}

void map_labels(TurnList &ref, TurnList &hyp, std::vector<int> assignment) {
  int k = 0;
  std::map<std::string, std::string> ref_map, hyp_map;
  std::set<std::string> ref_spk_remaining = ref.speaker_set;
  std::set<std::string> hyp_spk_remaining = hyp.speaker_set;
  std::string ref_spk, hyp_spk;
  for (int i = 0; i < assignment.size(); ++i) {
    if (assignment[i] != -1) {
      ref_spk = ref.reverse_index.find(i)->second;
      hyp_spk = hyp.reverse_index.find(assignment[i])->second;
      ref_map.insert(
          std::pair<std::string, std::string>(ref_spk, std::to_string(k)));
      hyp_map.insert(
          std::pair<std::string, std::string>(hyp_spk, std::to_string(k)));
      k += 1;
      ref_spk_remaining.erase(ref_spk);
      hyp_spk_remaining.erase(hyp_spk);
    }
  }
  for (auto &spk : ref_spk_remaining) {
    ref_map.insert(
        std::pair<std::string, std::string>(spk, std::to_string(k++)));
  }
  for (auto &spk : hyp_spk_remaining) {
    hyp_map.insert(
        std::pair<std::string, std::string>(spk, std::to_string(k++)));
  }
  ref.map_labels(ref_map);
  hyp.map_labels(hyp_map);
  // free up space
  ref_map.clear();
  hyp_map.clear();
}

void compute_der_mapped(TurnList &ref, TurnList &hyp, Metrics &metrics,
                        std::string region_type) {
  // Create a list of tokens combining reference and hypothesis
  std::vector<Token> tokens(2 * (ref.size() + hyp.size()));
  int i = -1;
  for (auto &turn : ref.turns) {
    tokens[++i] = Token(START, REF, turn.spk, turn.start);
    tokens[++i] = Token(END, REF, turn.spk, turn.end);
  }
  for (auto &turn : hyp.turns) {
    tokens[++i] = Token(START, HYP, turn.spk, turn.start);
    tokens[++i] = Token(END, HYP, turn.spk, turn.end);
  }
  // Sort the tokens. They will be sorted first by timestamp and then
  // by type (i.e. "end" tokens before "start"), since we overloaded
  // the Token "<" (less than) operator.
  std::sort(tokens.begin(), tokens.end());
  // Create list of homogeneous speaker regions from tokens
  std::vector<Region> regions;
  double region_start = tokens[0].timestamp;
  std::vector<std::string> ref_spk, hyp_spk;
  if (tokens[0].system == REF) {
    ref_spk.push_back(tokens[0].spk);
  } else {
    hyp_spk.push_back(tokens[0].spk);
  }
  for (int i = 1; i < tokens.size(); ++i) {
    if (tokens[i].timestamp - region_start > DBL_EPSILON) {
      regions.push_back(
          Region(region_start, tokens[i].timestamp, ref_spk, hyp_spk));
    }
    if (tokens[i].type == START) {
      if (tokens[i].system == REF) {
        ref_spk.push_back(tokens[i].spk);
      } else {
        hyp_spk.push_back(tokens[i].spk);
      }
    } else {
      if (tokens[i].system == REF) {
        ref_spk.erase(std::find(ref_spk.begin(), ref_spk.end(), tokens[i].spk));
      } else {
        hyp_spk.erase(std::find(hyp_spk.begin(), hyp_spk.end(), tokens[i].spk));
      }
    }
    region_start = tokens[i].timestamp;
  }
  // compute DER metrics
  double miss = 0, falarm = 0, conf = 0, total_dur = 0, scored_dur = 0, dur;
  int N_ref, N_hyp, N_correct;
  for (auto &region : regions) {
    dur = region.duration();
    N_ref = region.ref_spk.size();
    N_hyp = region.hyp_spk.size();
    if (!(region_type == "all") && !(region_type == "single" && N_ref == 1) &&
        !(region_type == "nonoverlap" && N_ref <= 1) &&
        !(region_type == "overlap" && N_ref > 1))
      continue;
    N_correct = region.num_correct();
    miss += dur * (std::max(0, N_ref - N_hyp));
    falarm += dur * (std::max(0, N_hyp - N_ref));
    conf += dur * (std::min(N_ref, N_hyp) - N_correct);
    total_dur += dur * N_ref;
    scored_dur += dur;
  }
  // free up memory
  std::vector<Token>().swap(tokens);
  std::vector<Region>().swap(regions);
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

Metrics compute_der(TurnList &ref, TurnList &hyp, std::string regions) {
  ref.merge_same_speaker_turns();
  hyp.merge_same_speaker_turns();
  ref.build_speaker_index();
  hyp.build_speaker_index();
  std::vector<std::vector<double>> cost_matrix = build_cost_matrix(ref, hyp);
  HungarianAlgorithm hungarian_solver;
  std::vector<int> assignment;
  double cost = hungarian_solver.Solve(cost_matrix, assignment);
  map_labels(ref, hyp, assignment);
  Metrics metrics;
  compute_der_mapped(ref, hyp, metrics, regions);
  return metrics;
}

}  // end namespace spyder

#endif
