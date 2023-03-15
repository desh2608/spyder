// spyder/utils.cc

// Copyright 2023  Johns Hopkins University (Author: Desh Raj)

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

#ifndef SPYDER_UTILS_CC
#define SPYDER_UTILS_CC

#include "utils.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "float.h"

namespace spyder {

double compute_intersection_length(Turn A, Turn B) {
  double max_start = std::max(A.start, B.start);
  double min_end = std::min(A.end, B.end);
  return std::max(0.0, min_end - max_start);
}

std::vector<std::vector<double>> build_cost_matrix(TurnList &ref, TurnList &hyp) {
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

void map_labels(TurnList &ref, TurnList &hyp, std::vector<int> assignment,
                std::map<std::string, std::string> &ref_map,
                std::map<std::string, std::string> &hyp_map) {
  int k = 0;
  std::set<std::string> ref_spk_remaining = ref.speaker_set;
  std::set<std::string> hyp_spk_remaining = hyp.speaker_set;
  std::string ref_spk, hyp_spk;
  for (int i = 0; i < assignment.size(); ++i) {
    if (assignment[i] != -1) {
      ref_spk = ref.reverse_index.find(i)->second;
      hyp_spk = hyp.reverse_index.find(assignment[i])->second;
      ref_map.insert(std::pair<std::string, std::string>(ref_spk, std::to_string(k)));
      hyp_map.insert(std::pair<std::string, std::string>(hyp_spk, std::to_string(k)));
      k += 1;
      ref_spk_remaining.erase(ref_spk);
      hyp_spk_remaining.erase(hyp_spk);
    }
  }
  for (auto &spk : ref_spk_remaining) {
    ref_map.insert(std::pair<std::string, std::string>(spk, std::to_string(k++)));
  }
  for (auto &spk : hyp_spk_remaining) {
    hyp_map.insert(std::pair<std::string, std::string>(spk, std::to_string(k++)));
  }
}

std::vector<Region> create_regions_from_tokens(std::vector<Token> &tokens) {
  // Sort the tokens. They will be sorted first by timestamp and then
  // by type (i.e. "end" tokens before "start"), since we overloaded
  // the Token "<" (less than) operator.
  std::sort(tokens.begin(), tokens.end());

  // Create list of evaluation regions
  std::vector<Region> regions;
  double region_start = tokens[0].timestamp;
  std::unordered_set<std::string> ref_spk, hyp_spk;
  int evaluate = 0;

  for (int i = 0; i < tokens.size(); ++i) {
    // If the evaluate flag is set and the region is not empty, add it to the
    // list of regions
    if (evaluate == 1 && tokens[i].timestamp - region_start > DBL_EPSILON) {
      std::vector<std::string> ref_spk_list(ref_spk.begin(), ref_spk.end());
      std::vector<std::string> hyp_spk_list(hyp_spk.begin(), hyp_spk.end());
      regions.push_back(
          Region(region_start, tokens[i].timestamp, ref_spk_list, hyp_spk_list));
    }

    // Update the list of ref and hyp speakers in the current region
    if (tokens[i].system == REF) {
      if (tokens[i].type == START) {
        ref_spk.insert(tokens[i].spk);
      } else {
        ref_spk.erase(tokens[i].spk);
      }
    } else if (tokens[i].system == HYP) {
      if (tokens[i].type == START) {
        hyp_spk.insert(tokens[i].spk);
      } else {
        hyp_spk.erase(tokens[i].spk);
      }
    } else {
      // If it is a UEM token, update the evaluate flag
      evaluate = int(tokens[i].type == START);
    }

    // Update the region start time
    region_start = tokens[i].timestamp;
  }
  return regions;
}

}  // end namespace spyder

#endif
